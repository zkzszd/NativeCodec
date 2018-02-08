#include <functional>
#include <iostream>
#include "../video/EncodeLoop.hpp"
#include "../audio/AudioEncode.hpp"
#include "../librtmp/RTMPPusher.hpp"
#include "../util/MediaCodecUtil.h"
#include "../librtmp/SpsUtil.h"
#include "../base/MyDef.hpp"
#include <android/log.h>
#include <android/looper.h>
#include "../util/YUVUtil.hpp"
#include <jni.h>
#define TAG "pusherWork"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

using namespace std;
using namespace WeiYu;

typedef struct _pushWork
{
  EncodeLoop *encodeLoop = NULL;
  AudioEncode *audioEncode = NULL;
  RTMPPusher *rtmpPusher = NULL;
  RTMPMetadata metaData = {0};

  char _audioHeader[2];

  void audioCallback(NaluStruct* naluData)
  {
    if(naluData->size == 2)
      {
        char header[4];
//        第一个字节AF，A就是10代表的意思是AAC。
//        第一个字节中的后四位F代表如下 ：
//          前2个bit的含义 抽样频率，这里是二进制11，代表44kHZ
//        0 = 5.5-kHz
//        1 = 11-kHz
//        2 = 22-kHz
//        3 = 44-kHz 二进制11
//        对于AAC总是3
//          第3个bit，代表 音频用16位的
//          第4个bit代表声道
//        0 = sndMono，单声道
//        1 = sndStereo，立体声
//        对于AAC总是1
        _audioHeader[0] = header[0] = 0xAF;
//        第2个字节  AACPacketType，这个字段来表示AACAUDIODATA的类型：0 = AAC sequence header（音频同步包），1 = AAC raw（音频raw数据）。第一个音频包用0，后面的都用1
        header[1] = 0x00;
        _audioHeader[1] = 0x01;
        LOGV("audio header %x %x",naluData->data[0],naluData->data[1]);
        header[2] = naluData->data[0];
        header[3] = naluData->data[1];
        if(rtmpPusher->SendAudioHeader(header,4))
          {
            LOGV("send audio head ok");
          }
        else
          {
            LOGV("send audio head fail");
          }
        delete naluData;
      }
    else
      {
        unsigned char* oldData = naluData->data;
        naluData->data = (unsigned char*)malloc(naluData->size+2);
        naluData->data[0] =  _audioHeader[0];
        naluData->data[1] =  _audioHeader[1];
        memcpy(naluData->data+2,oldData,naluData->size);
        naluData->size += 2;
        free(oldData);
        rtmpPusher->post(RTMPPUSHER_MES_AAC_DATA,naluData);
      }
  }

  //callback
  void videoCallback(NaluStruct* naluData)
  {
      switch(naluData->type)
      {
      case 7:     //sps 继续读取下个nalu一般为pps一起打包   （安卓编码出来的sps和pps在一起）
        {
          int spsLen = readOneNaluFromBuf((char*)naluData->data,naluData->size);
          int ppsLen = naluData->size-spsLen;
          memcpy(metaData.Sps,naluData->data,spsLen);
          metaData.nSpsLen = spsLen;

          int width = 0, height = 0,frate = 0;
          Util_decode_sps(metaData.Sps, metaData.nSpsLen, width, height,frate);
          LOGV("decode sps len = %d spslen=%d %d,%d ,%d",naluData->size,spsLen,width,height,frate);

          memcpy(metaData.Pps,naluData->data+spsLen,ppsLen);
          metaData.nPpsLen = ppsLen;

          if(!rtmpPusher->SendMetadata(&metaData))
            {
              LOGV("SendMetadata failed");
            }
          else
            {
              LOGV("send metadate ok");
            }
          delete naluData;
          LOGV("sps end");
          break;
        }
  //              case 0x68:      //目前发现mediacodec编码出来的pps和sps在一起，没有单独的pps
  //                {
  //                  LOGV("process pps");
  //                  memcpy(metaData.Pps,buf,info.size);
  //                  metaData.nPpsLen = info.size;
  //                  fwrite(buf,info.size,1,(d->fd));
  //                  stream.SendMetadata(&metaData);
  //                  LOGV("pps end");
  //                }
  //                break;
      default:
        {
          rtmpPusher->post(RTMPPUSHER_MES_H264_DATA,naluData);
          break;
        }
      }
  }
  ~_pushWork()
  {
    if(encodeLoop)
      {
      delete encodeLoop;
      encodeLoop = NULL;
      }
    if(rtmpPusher)
      {
        delete rtmpPusher;
        rtmpPusher = NULL;
      }

    if(audioEncode)
      {
        delete audioEncode;
        audioEncode = NULL;
      }

  }
}PushWork;

PushWork *pushWork = NULL;
static JavaVM* g_jvm = NULL;
extern "C" {
  JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void *reserved)
  {
      JNIEnv* env = NULL;
      jint result = -1;

      if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
          return -1;

      g_jvm = vm;

      return JNI_VERSION_1_4;
  }

  void Java_com_example_nativecodec_NativePusher_putNaluData(JNIEnv* env,jclass clazz,
      jbyteArray jNalu_data,jint length)
  {
    if(pushWork && pushWork->rtmpPusher)
    {
        jbyte* pdata = env->GetByteArrayElements(jNalu_data, 0);
        NaluStruct* naluData = new NaluStruct((const unsigned char*)pdata,length);
//        naluData->size = length;
//        naluData->data = (unsigned char* )malloc(length);
//        memcpy(naluData->data,pdata,length);
        pushWork->videoCallback(naluData);
        env->ReleaseByteArrayElements(jNalu_data, pdata, 0);
    }
  }


  void Java_com_example_nativecodec_NativePusher_putCameraData(JNIEnv* env,jclass clazz,
          jbyteArray jyuv_data,jint length,jint width,jint height)
  {
    if(pushWork && pushWork->encodeLoop)
    {
      jbyte* pdata = env->GetByteArrayElements(jyuv_data, 0);

      //此处是旋转后的数据，所以直接初始化宽高颠倒
      YUVStruct *yuvData = new YUVStruct(length,height,width);
      YUVUtil::YUV420spRotateNegative90(yuvData->data,(char*)pdata,width,height);

      YUV420p *yuv420p = new YUV420p(length,yuvData->width,yuvData->height);
      YUVUtil::NV21ToYUV420p(*(YUVNV21*)yuvData,*yuv420p);
      delete yuvData;


      pushWork->encodeLoop->post(MES_Camera_data,yuv420p);

      //pushWork->encodeLoop->putCameraData((char*)pdata,length);
      env->ReleaseByteArrayElements(jyuv_data, pdata, 0);
    }
    else
      {
        LOGV("putCameraData but not init encode error!");
      }

  }

  void Java_com_example_nativecodec_NativePusher_initAudioEncode(JNIEnv* env,jclass clazz,
      jint channelCount,jint sampleRate)
  {
    if(pushWork == NULL)
    {
      pushWork = new PushWork();
    }
    if(pushWork->audioEncode == NULL)
      {
        pushWork->audioEncode = new AudioEncode();
        if(!pushWork->audioEncode->openAudioDevice(channelCount,sampleRate))
          {
            LOGV("open openAudioDevice failed");
          }
        pushWork->audioEncode->addCallback(std::bind(&PushWork::audioCallback,pushWork,std::placeholders::_1));
        pushWork->audioEncode->startRun();
      }
    pushWork->metaData.nAudioChannels = channelCount;
    pushWork->metaData.nAudioSampleSize = sampleRate;

  }

  //安卓摄像头默认横屏，需要逆时针旋转90度，旋转完后宽高颠倒
  //此处初始化宽高电脑，接着把putdata数据宽高颠倒
  void Java_com_example_nativecodec_NativePusher_initVideoEncode(JNIEnv* env,jclass clazz,
          jint videoWidth,jint videoHeight,jint videoFramerate,jint bitRate,jint I_FRAME_INTERVAL,jint videoFormat,jobject surface)
  {
    if(pushWork == NULL)
      {
        pushWork = new PushWork();
      }
    if(pushWork->encodeLoop == NULL)
      {
        pushWork->encodeLoop = new EncodeLoop(videoHeight,videoWidth,videoFramerate,bitRate,I_FRAME_INTERVAL,videoFormat);
        if(surface != NULL)
        {
            pushWork->encodeLoop->initYUVSurface(g_jvm,surface);
        }
      }
    //别忘记addcallback
    //bind绑定的函数参数必须指定（或者占位符制定）
    pushWork->encodeLoop->addCallback(std::bind(&PushWork::videoCallback,pushWork,std::placeholders::_1));
    pushWork->metaData.nWidth = videoHeight;
    pushWork->metaData.nHeight = videoWidth;
    pushWork->metaData.nFrameRate = videoFramerate;
    pushWork->metaData.bHasAudio = false;
  }

  jboolean Java_com_example_nativecodec_NativePusher_initPusherWorkPusher(JNIEnv* env,jclass clazz,
        jstring url)
  {
    jboolean ret = JNI_TRUE;
    if(pushWork == NULL)
      {
        pushWork = new PushWork();
      }
    pushWork->rtmpPusher = new RTMPPusher();
    LOGV("begin connect");
    const char *utf8_url = env->GetStringUTFChars(url, NULL);
    if(!pushWork->rtmpPusher->connect(utf8_url))
      {
        ret = JNI_FALSE;
        LOGV("pusherWork pusher connect %s failed!",utf8_url);
      }
    else
      {
        LOGV("connect %s ok",utf8_url);
      }
    env->ReleaseStringUTFChars(url, utf8_url);
    return ret;
  }

  void Java_com_example_nativecodec_NativePusher_PusherWorkDestory(JNIEnv* env,jclass clazz)
  {
    if(pushWork)
      delete pushWork;
  }


}
