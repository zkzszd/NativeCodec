/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* This is a JNI example where we use native methods to play video
 * using the native AMedia* APIs.
 * See the corresponding Java source file located at:
 *
 *   src/com/example/nativecodec/NativeMedia.java
 *
 * In this example we use assert() for "impossible" error conditions,
 * and explicit handling and recovery for more likely error conditions.
 */

#include <assert.h>
#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <iostream>
#include <deque>


using namespace std;

#include "../base/NXLock.h"
#include "../looper/looper.h"
#include "../NativeCodec/NativeCodec.h"
#include "../librtmp/rtmpclient.h"
#include "../librtmp/CRTMPStream.hpp"
#include "../video/DecodeLoop.hpp"
#include "../librtmp/RTMPPlayer.hpp"
#include "../util/MediaCodecUtil.h"
using namespace WeiYu;

typedef struct _MYNaluUnit
{
        int type;
        int size;
        unsigned char data[102400];
}MYNaluUnit;

#define WIDTH 640
#define HEIGHT 480
#define Framerate  25
#define FPS 30.0f


//#include "media/NdkMediaCodec.h"
//#include "media/NdkMediaFormat.h"




// for __android_log_print(ANDROID_LOG_INFO, "YourApp", "formatted message");
#include <android/log.h>
#define TAG "NativeCodec"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

// for native window JNI
#include <android/native_window_jni.h>

class workerdata : public LoopMsgObj
{
public:
    FILE* fd;
    ANativeWindow* window;
    AMediaCodec *codec;
    int64_t renderstart;
    bool sawInputEOS;
    bool sawOutputEOS;			//end of stream(zkzszd)
    bool isPlaying;
    bool renderonce;			//render once渲染一次
    NXLock lock;
    deque<char*> deque_data;
public:
    workerdata()
    {
      fd = NULL;
      window = NULL;
      codec = NULL;
      renderstart = 0;
      sawInputEOS= false;
      sawOutputEOS = false;
      isPlaying = false;
      renderonce = false;
    }
};

workerdata data;

enum {
    kMsgCodecBuffer,
    kMsgPause,
    kMsgResume,
    kMsgPauseAck,
    kMsgDecodeDone,
    kMsgSeek,
};


MYNaluUnit NalData;

class mylooper: public looper {
    virtual void handle(int what, void* obj);
};

static mylooper *mlooper = NULL;

long computePresentationTime(long frameIndex) {
     return 132 + frameIndex * 1000000 / Framerate;
 }

int readOneNaluFromFile(FILE *fp,MYNaluUnit *nalu)
{
	nalu->size = 0;
	char m_pFileBuf[1024];
	int m_nFileBufSize;
	bool is_ok = false;

	while((m_nFileBufSize = fread(m_pFileBuf,1,1024,fp)) == 1024)
	{
		//倒数后八个字符
		for(int pos = 4;pos < 1016;)
		{
			while (pos<m_nFileBufSize)
			{
				if (m_pFileBuf[pos++] == 0x00 &&
					m_pFileBuf[pos++] == 0x00 &&
					m_pFileBuf[pos++] == 0x00 &&
					m_pFileBuf[pos++] == 0x01
					)
				{
					is_ok = true;
					break;
				}
			}
			if(is_ok)
			{
				pos -= 4;
				nalu->type = nalu->data[4] & 0x1f;
				memcpy(nalu->data+nalu->size,m_pFileBuf,pos);
				nalu->size += pos;
				//LOGV("---%d%d%d%d  %d",m_pFileBuf[pos],m_pFileBuf[pos+1],m_pFileBuf[pos+2],m_pFileBuf[pos+3],pos-1024);
				//文件指针回移动
				fseek(fp,pos-1024,SEEK_CUR);
//				LOGV("--111---%d%d%d%d",fgetc(fp),fgetc(fp),fgetc(fp),fgetc(fp));
//				fseek(fp,-4,SEEK_CUR);
				return 0;
			}
			else
			{

				memcpy(nalu->data+nalu->size,m_pFileBuf,1016);
				nalu->size += 1016;
				//回移动文件指针
				fseek(fp,-8,SEEK_CUR); // 文件SEEK_SET当前定位
			}
		}
	}
	nalu->size += m_nFileBufSize;
	nalu->type = nalu->data[4] & 0x1f;
	memcpy(nalu->data,m_pFileBuf,m_nFileBufSize);

	return 1;
}


void doCodecWork(workerdata *d) {
	int read_file_state = readOneNaluFromFile(d->fd,&NalData) ;
	ssize_t bufidx;
	bufidx = AMediaCodec_dequeueInputBuffer(d->codec, 2000);				//后面是超时时间
	if (bufidx >= 0)
	{
		LOGV("put input buf");
		size_t bufsize;
		uint8_t *buf = AMediaCodec_getInputBuffer(d->codec, bufidx, &bufsize);
		memcpy(buf,NalData.data,NalData.size);
		AMediaCodec_queueInputBuffer(d->codec, bufidx, 0, NalData.size,systemnanotime(),
				read_file_state == 1 ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
	}

	ssize_t status;
	AMediaCodecBufferInfo info;
	status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 2000);
	LOGV("status %d %d %d %d %d %d %d %d",NalData.size,read_file_state,status,NalData.data[0],NalData.data[1],NalData.data[2],NalData.data[3],NalData.data[4]);
	while(status >= 0)
	{
		size_t bufsize;
		uint8_t *buf = AMediaCodec_getOutputBuffer(d->codec, status, &bufsize);
		LOGV("out put len %d ",info.size);
		AMediaCodec_releaseOutputBuffer(d->codec, status, true);//info.size != 0);
		status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 0);
		usleep(40000);             //usleep单位微秒
	}
	 if (read_file_state == 0) {
		 mlooper->post(kMsgCodecBuffer, d);
	 }
}

int pingIP(char* str_ip)
{
  int ret_code = 0;
  char cmd[128] = {0};
  sprintf(cmd,"ping -c 1 %s",str_ip);
  int i = -1;
  int count=0;
  while(1)
  {
   i=system(cmd);
   printf("/ni=%d",i);
   count++;
   if(i==0)
   {
    i=system("settop install;settop odtest.out");
    break;
   }
   if(count>3)
   {
    LOGV(" error cannot ping %s",str_ip);
    system("cd /root;ls;settop install;settop odtest.out");
    ret_code = -1;
    break;
   }
  }
  LOGV("ping ok");
  return ret_code;
}

void mylooper::handle(int what, void* obj) {
    switch (what) {
        case kMsgCodecBuffer:
            doCodecWork((workerdata*)obj);
            break;

        case kMsgDecodeDone:
        {
            workerdata *d = (workerdata*)obj;
            AMediaCodec_stop(d->codec);
            AMediaCodec_delete(d->codec);
            d->sawInputEOS = true;
            d->sawOutputEOS = true;
        }
        break;

        case kMsgSeek:
        {
            workerdata *d = (workerdata*)obj;
            AMediaCodec_flush(d->codec);
            d->renderstart = -1;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;
            if (!d->isPlaying) {
                d->renderonce = true;
                post(kMsgCodecBuffer, d);
            }
            LOGV("seeked");
        }
        break;

        case kMsgPause:
        {
            workerdata *d = (workerdata*)obj;
            if (d->isPlaying) {
                // flush all outstanding codecbuffer messages with a no-op message
                d->isPlaying = false;
                post(kMsgPauseAck, NULL, true);
            }
        }
        break;

        case kMsgResume:
        {
            workerdata *d = (workerdata*)obj;
            if (!d->isPlaying) {
                d->renderstart = -1;
                d->isPlaying = true;
                post(kMsgCodecBuffer, d);
            }
        }
        break;
    }
}


extern "C" {

//  RTMPPlayer *player;
//  void Java_com_example_nativecodec_NativeCodec_quit(JNIEnv* env,
//      jclass clazz)
//  {
//    delete player;
//  }
//
//  jboolean Java_com_example_nativecodec_NativeCodec_createStreamingMediaPlayer(JNIEnv* env,
//          jclass clazz, jstring filename,jobject surface)
//  {
//    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
//    player = new RTMPPlayer(window);
//    //rtmp://live.hkstv.hk.lxdns.com/live/hks
//    //rtmp://192.168.152.177:1935/live/stream
//
//    //亚太卫视 rtmp://58.61.150.198/live/Livestream
//    //亚太第一卫视 rtmp://v1.one-tv.com/live/mpegts.stream
//    //希望电视台 rtmp://203.69.85.37/livepkgr/hopestream1?adbe-live-event=hopetv
//    //GOOD TV 1  rtmp://l.jdjys.net/livefy/livestream
//    //GOOD TV 2  rtmp://l.jdjys.net/livefy2/livestream
//    //台湾民视   rtmp://202.39.43.80:80/live_hich/BC000121_1 失败
//    //if(player->connect("rtmp://192.168.83.67:1935/live/live11.sdp"))
//    if(player->connect("rtmp://live.hkstv.hk.lxdns.com/live/hks"))    //香港卫视rtmp://send1.douyu.com/live
//    //if(player->connect("rtmp://pubsec.mudu.tv/watch/0t5ejs?auth_key=2082733261-0-0-8f2e55e5dcd07229b89bf88d29b08220"))
//    //if(player->connect("rtmp://send1.douyu.com/live/656505rBElAOPzEr?wsSecret=f9c648589c4852482b970befc8711de8&wsTime=5a55fd5d&wsSeek=off"))
//      {
//        LOGV("@@@ connect ok" );
//        player->start();
//        //player->readPacketThread();
//      }
//    else
//      {
//        LOGV("@@@ connect  failed" );
//      }
//    return JNI_TRUE;
//  }

//jboolean Java_com_example_nativecodec_NativeCodec_createStreamingMediaPlayer(JNIEnv* env,
//        jclass clazz, jstring filename,jobject surface)
//{
//	if (data.window) {
//		ANativeWindow_release(data.window);
//		data.window = NULL;
//	}
//
//	data.window = ANativeWindow_fromSurface(env, surface);
//	if(data.window)
//	LOGV("@@@ setsurface %p  %d  %d ",data.window, ANativeWindow_getWidth(data.window),ANativeWindow_getHeight(data.window));
//	else
//	{
//		LOGV("@@@ setsurface %p ",data.window );
//	}
//
//	const char *utf8 = env->GetStringUTFChars(filename, NULL);
//	char path[] = "/storage/emulated/0/receive.h264";
//	data.fd = fopen(path,"rb");
//	if(data.fd <= 0)
//	  {
//	    LOGV("@@@ open file failed %s ",utf8);
//	  }
//	env->ReleaseStringUTFChars(filename, utf8);
//	workerdata *d = &data;
//
//	AMediaCodec *codec = NULL;
//	const char* mine = "video/avc";
//	codec = AMediaCodec_createDecoderByType(mine);
//
//	AMediaFormat *videoFormat = AMediaFormat_new();
//	AMediaFormat_setString(videoFormat, AMEDIAFORMAT_KEY_MIME, "video/avc");
//	AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_WIDTH, 640); // 视频宽度
//	AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_HEIGHT, 480); // ﻿视频高度
//	//AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_FRAME_RATE, Framerate);//帧速率
//	//AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_BIT_RATE, 125000); //
//
//
//	//AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 5);	//I帧间隔
//	readOneNalu(d->fd,&NalData) ;
//	LOGV("sps len = %d %d%d%d%d",NalData.size,NalData.data[0],NalData.data[1],NalData.data[2],NalData.data[3]);
//	AMediaFormat_setBuffer(videoFormat, "csd-0", NalData.data, NalData.size); // sps
//	readOneNalu(d->fd,&NalData) ;
//	LOGV("pps len = %d %d%d%d%d",NalData.size,NalData.data[0],NalData.data[1],NalData.data[2],NalData.data[3]);
//	AMediaFormat_setBuffer(videoFormat, "csd-1", NalData.data, NalData.size); // pps
//	//AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_COLOR_FORMAT, 21);//OMX_COLOR_FormatYUV420Planar对应YV12		#21 COLOR_FormatYUV420SemiPlanar (NV12)
//	int code = AMediaCodec_configure(codec, videoFormat,d->window,NULL, 0);// d->window, NULL, 0);
//	if( code != AMEDIA_OK)
//	{
//		LOGV("zkzszd failed AMediaCodec_configure: %d ", code);
//	}
//
//	d->codec = codec;
//	d->renderstart = -1;
//	d->sawInputEOS = false;
//	d->sawOutputEOS = false;
//	d->isPlaying = false;
//	d->renderonce = true;
//	AMediaCodec_start(codec);
//	AMediaFormat_delete(videoFormat);
//
//
//
//	mlooper = new mylooper();
//	mlooper->post(kMsgCodecBuffer, d);
//
//	return JNI_TRUE;
//}


jboolean Java_com_example_nativecodec_NativeCodec_createSendH264File(JNIEnv* env,
    jclass clazz, jstring filename)
{
  const char *utf8 = env->GetStringUTFChars(filename, NULL);
  char url[] = "rtmp://192.168.152.177:1935/live/live11.sdp";
  CRTMPStream stream;
  if (stream.Connect(url) == false)
  {
      LOGV("connect fail\n");
  }
  stream.SendH264File(utf8);
  stream.Close();


  env->ReleaseStringUTFChars(filename, utf8);
  return JNI_TRUE;
}


bool is_init_ok = false;
CRTMPStream stream;
jboolean Java_com_example_nativecodec_NativeCodec_createStreamingRecode(JNIEnv* env,
        jclass clazz, jstring filename)
{
    LOGV("@@@ create");
    char ip[] = "192.168.152.177";
    pingIP(ip);
    char url[] = "rtmp://pubsec.mudu.tv/watch/0t5ejs?auth_key=2082733261-0-0-8f2e55e5dcd07229b89bf88d29b08220";
    //char url[] = "rtmp://192.168.152.177:1935/live/live11.sdp";

    if (stream.Connect(url) == false)
    {
        LOGV("connect fail\n");
    }
    else
    {
      LOGV("connect rtmp ok");
    }

    // convert Java string to UTF-8
    const char *utf8 = env->GetStringUTFChars(filename, NULL);
//    LOGV("opening %s", utf8);
//    int fd = open(utf8, O_RDONLY);
//    env->ReleaseStringUTFChars(filename, utf8);
//    if (fd < 0) {
//        LOGV("failed: %d (%s)", fd, strerror(errno));
//        return JNI_FALSE;
//    }
    data.fd = fopen(utf8,"wb+");
    env->ReleaseStringUTFChars(filename, utf8);

    //data.fd = fd;

    workerdata *d = &data;

    //close(d->fd);


    AMediaCodec *codec = NULL;
    const char* mine = "video/avc";
    codec = AMediaCodec_createEncoderByType(mine);

    AMediaFormat *videoFormat = AMediaFormat_new();
    AMediaFormat_setString(videoFormat, AMEDIAFORMAT_KEY_MIME, "video/avc");
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_WIDTH, WIDTH); // 视频宽度
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_HEIGHT, HEIGHT); // ﻿视频高度
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_BIT_RATE, 125000); //       //不设置会出错
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_FRAME_RATE, Framerate);//帧速率 必须设置
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, 2); //
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, 15); // ﻿采样率

    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 1);	//I帧间隔时间 单位s
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_COLOR_FORMAT, 21);//OMX_COLOR_FormatYUV420Planar对应YV12		#21 COLOR_FormatYUV420SemiPlanar (NV12)
//	uint8_t sps[2] = {0x12, 0x12};
//	uint8_t pps[2] = {0x12, 0x12};
//	AMediaFormat_setBuffer(videoFormat, "csd-0", sps, 2); // sps
//	AMediaFormat_setBuffer(videoFormat, "csd-1", pps, 2); // pps
	//AMediaFormat_setBuffer(videoFormat, "csd-0", sps, spsSize); // sps
	//AMediaFormat_setBuffer(videoFormat, "csd-1", pps, ppsSize); // pps
	//http://www.jianshu.com/p/660652271254
	//如果设置输出窗口获取不到输出数据  只有解码器才能配置输出窗口（如果编码配置输出窗口，三星note2直接会崩溃）
	int code = AMediaCodec_configure(codec, videoFormat,NULL,NULL, AMEDIACODEC_CONFIGURE_FLAG_ENCODE);// d->window, NULL, 0);
	if( code != AMEDIA_OK)
	{
		LOGV("zkzszd failed AMediaCodec_configure: %d ", code);
	}

	d->codec = codec;
	d->renderstart = -1;
	d->sawInputEOS = false;
	d->sawOutputEOS = false;
	d->isPlaying = false;
	d->renderonce = true;
	AMediaCodec_start(codec);
	AMediaFormat_delete(videoFormat);



    mlooper = new mylooper();
    //mlooper->post(kMsgCodecBuffer, d);

    is_init_ok = true;
    return JNI_TRUE;
}

//数据流结束
void Java_com_example_nativecodec_NativeCodec_EOS(JNIEnv* env,
        jclass clazz)
{
	data.sawInputEOS = true;
}

RTMPMetadata metaData = {0};
void Java_com_example_nativecodec_NativeCodec_putCameraData(JNIEnv* env,
        jclass clazz, jint jwidth, jint jheight, jbyteArray jyuv_data)
{
	if( !is_init_ok )
		return;
	int width = 640;
	int heigth = 480;
	int total = 640*480*3/2;


	workerdata *d = &data;
	jbyte* pdata = env->GetByteArrayElements(jyuv_data, 0);
	//LOGV("zkzszd at put data %d %d %d",pdata[0],pdata[1],pdata[2]);
	int bufidx = AMediaCodec_dequeueInputBuffer(d->codec, 2000);
	//LOGV("dequeueInputBuffer at %d",bufidx);
	if(bufidx >= 0)
	{
		size_t bufsize;
		uint8_t *buf = AMediaCodec_getInputBuffer(d->codec, bufidx, &bufsize);
		//memcpy(buf,(char*)pdata,640*480);
		memcpy(buf,(char*)pdata,total);
		AMediaCodec_queueInputBuffer(d->codec, bufidx, 0, total, systemnanotime(),0);
	}

	AMediaCodecBufferInfo info;
	ssize_t status;

	status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 0);
	while(status >= 0)
	{
		//status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 0);
		size_t bufsize;
		uint8_t *buf = AMediaCodec_getOutputBuffer(d->codec, status, &bufsize);
		//LOGV("@@@ out put buffer lenght: %d bufix:%d-- %x%x%x%x", info.size,status,buf[0],buf[1],buf[2],buf[3]);

		switch(buf[4])
		{
		case 0x67:     //sps 继续读取下个nalu一般为pps一起打包   （安卓编码出来的sps和pps在一起）
		  {
		    LOGV("process sps");
		    metaData.bHasAudio = false;
		    metaData.nWidth = WIDTH;
		    metaData.nHeight = HEIGHT;
		    metaData.nFrameRate = Framerate;
		    int spsLen = readOneNaluFromBuf((char*)buf,info.size);
		    int ppsLen = info.size-spsLen;
		    LOGV("sps len %d pps %d buf %d",spsLen,ppsLen,info.size);
		    memcpy(metaData.Sps,buf,spsLen);
		    metaData.nSpsLen = spsLen;

		    int width = 0, height = 0,frate = 0;
		    Util_decode_sps(metaData.Sps, metaData.nSpsLen, width, height,frate);
		    LOGV("decode sps %d,%d ,%d",width,height,frate);

		    memcpy(metaData.Pps,buf+spsLen,ppsLen);
		    metaData.nPpsLen = ppsLen;
		    stream.SendMetadata(&metaData);

		    fwrite(buf,info.size,1,(d->fd));
		    LOGV("sps end");
		  }
		  break;
//		case 0x68:
//		  {
//		    LOGV("process pps");
//		    memcpy(metaData.Pps,buf,info.size);
//		    metaData.nPpsLen = info.size;
//		    fwrite(buf,info.size,1,(d->fd));
//		    stream.SendMetadata(&metaData);
//		    LOGV("pps end");
//		  }
//		  break;
		default:
		  {
		    //LOGV("@@@ send rtmp type %d",buf[4] & 0x1f);
                    fwrite(buf,info.size,1,(d->fd));
                    //stream.SendH264Packet(buf,info.size,buf[4] == 0x65?true:false,systemnanotime());
                    if(stream.sendOneNalu(buf,info.size,buf[4] & 0x1f))
                      {
                        LOGV("send nalu ok %x",buf[4]);
                      }
                    else
                      {
                        LOGV("send nalu failed");
                      }
		  }
		  break;
		}


		AMediaCodec_releaseOutputBuffer(d->codec, status, true);//info.size != 0);
		status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 0);
	}

	env->ReleaseByteArrayElements(jyuv_data, pdata, 0);
}

// set the playing state for the streaming media player
void Java_com_example_nativecodec_NativeCodec_setPlayingStreamingMediaPlayer(JNIEnv* env,
        jclass clazz, jboolean isPlaying)
{
    LOGV("@@@ playpause: %d", isPlaying);
    if (mlooper) {
        if (isPlaying) {
            mlooper->post(kMsgResume, &data);
        } else {
            mlooper->post(kMsgPause, &data);
        }
    }
}


// shut down the native media system
void Java_com_example_nativecodec_NativeCodec_shutdown(JNIEnv* env, jclass clazz)
{
    LOGV("@@@ shutdown");
    if (mlooper) {
        mlooper->post(kMsgDecodeDone, &data, true /* flush */);
        mlooper->quit();
        delete mlooper;
        mlooper = NULL;
    }
    if (data.window) {
        ANativeWindow_release(data.window);
        data.window = NULL;
    }
}


// set the surface
void Java_com_example_nativecodec_NativeCodec_setSurface(JNIEnv *env, jclass clazz, jobject surface)
{
    // obtain a native window from a Java surface
    if (data.window) {
        ANativeWindow_release(data.window);
        data.window = NULL;
    }
    data.window = ANativeWindow_fromSurface(env, surface);
    if(data.window)
    LOGV("@@@ setsurface %p  %d  %d ",data.window, ANativeWindow_getWidth(data.window),ANativeWindow_getHeight(data.window));
    else
    {
    	LOGV("@@@ setsurface %p ",data.window );
    }
}


// rewind the streaming media player
void Java_com_example_nativecodec_NativeCodec_rewindStreamingMediaPlayer(JNIEnv *env, jclass clazz)
{
    LOGV("@@@ rewind");
    mlooper->post(kMsgSeek, &data);
}

}
