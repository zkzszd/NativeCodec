#include "EncodeLoop.hpp"
#include "../util/MediaCodecUtil.h"
#include <string.h>
#include "../util/YUVUtil.hpp"
#include "SelectiveBlur.hpp"

namespace WeiYu
{
  EncodeLoop::EncodeLoop(int videoWidth,int videoHeight,int videoFramerate,int bitRate,int I_FRAME_INTERVAL,int videoFormat):
      FixedLoop(videoFramerate),
      _videoWidth(videoWidth),
      _videoHeight(videoHeight),
      _videoFramerate(videoFramerate),
      _I_FRAME_INTERVAL(I_FRAME_INTERVAL),
      _inputFormat(videoFormat),
      _videoBitRate(bitRate)
  {
    const char* mine = "video/avc";
    _pMediaCodec = AMediaCodec_createEncoderByType(mine);
    _pMediaFormat = AMediaFormat_new();

    AMediaFormat_setString(_pMediaFormat, AMEDIAFORMAT_KEY_MIME, mine);

//    AMediaFormat_setString(videoFormat, AMEDIAFORMAT_KEY_MIME, "video/avc");
//        AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_WIDTH, WIDTH); // 视频宽度
//        AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_HEIGHT, HEIGHT); // ﻿视频高度
//        AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_BIT_RATE, 125000); //       //不设置会出错
//        AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_FRAME_RATE, Framerate);//帧速率 必须设置
//        AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, 2); //
//        AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, 15); // ﻿采样率
//
//        AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 1);   //I帧间隔时间 单位s
//        AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_COLOR_FORMAT, 21);

    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_WIDTH, _videoWidth); // 视频宽度
    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_HEIGHT, _videoHeight); // ﻿视频高度
    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_BIT_RATE, _videoBitRate); //       //不设置会出错
    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_FRAME_RATE, _videoFramerate);//帧速率 必须设置
//    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, _audioChannelCount); //
//    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, _audioSampleRate); // ﻿采样率
    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, _I_FRAME_INTERVAL);       //I帧间隔时间 单位s
    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_COLOR_FORMAT, _inputFormat);

    int code = AMediaCodec_configure(_pMediaCodec,_pMediaFormat,NULL,NULL, AMEDIACODEC_CONFIGURE_FLAG_ENCODE);//编码不能设置窗口，设置会崩溃
    if( code != AMEDIA_OK)
    {
            LOGV("zkzszd failed AMediaCodec_configure: %d ", code);
    }
    AMediaCodec_start(_pMediaCodec);

    blur = new Blur(_videoWidth,_videoHeight);
  }

  EncodeLoop::~EncodeLoop()
  {
    if(_nativeYUVSurface)
      {
        delete _nativeYUVSurface;
      }
    if(_pMediaFormat)
      {
        AMediaFormat_delete(_pMediaFormat);
      }
    if(_pMediaCodec)
      {
        AMediaCodec_delete(_pMediaCodec);
      }
    if(blur)
      {
        delete blur;
      }
  }

  int EncodeLoop::initYUVSurface(JavaVM* javaVM,jobject glSurface)
  {
    _nativeYUVSurface = new NativeYUVSurface(javaVM, glSurface);
    if (_nativeYUVSurface->Init() != 0)
    {
        LOGV( "初期化失败_nativeYUVSurface->Init()");
        return -1;
    }
    return 0;
  }

  void EncodeLoop::handle(int what, LoopMsgObj *data)
  {
    switch(what)
    {
    case MES_Camera_data:
      {
        YUV420p *yuv420 = (YUV420p *)data;
//        //显示
//        if(_nativeYUVSurface)
//        {
//            _nativeYUVSurface->PutYUV420pDataAndRender((uint8_t*)yuv420->data,yuv420->width*yuv420->height*1.5,yuv420->width,yuv420->height);
//        }
        LOGV("begin %lld",systemnanotime());
//        //选择模糊
        SelectiveBlur::selectBlurYUV420p_Y(*yuv420,7,16, Edge_Mode_Smear);
//        blur->process((YUVStruct&)(*yuv420),40);
        LOGV("end %lld",systemnanotime());
        //显示
        if(_nativeYUVSurface)
        {
            _nativeYUVSurface->PutYUV420pDataAndRender((uint8_t*)yuv420->data,yuv420->width*yuv420->height*1.5,yuv420->width,yuv420->height);
        }

        YUVStruct* yuvData = new YUVStruct(yuv420->size,yuv420->width,yuv420->height);
        YUVUtil::YUV420pToNV12(*yuv420,*(YUVNV12*)yuvData);
        delete yuv420;

//        //显示
//        if(_nativeYUVSurface)
//        {
//            _nativeYUVSurface->PutNV12DataAndRender((uint8_t*)yuvData->data,yuvData->width*yuvData->height*1.5,yuvData->width,yuvData->height);
//        }

        //LOGV("zkzszd at put data %d %d %d",pdata[0],pdata[1],pdata[2]);
        int bufidx = AMediaCodec_dequeueInputBuffer(_pMediaCodec, 2000);
        //LOGV("dequeueInputBuffer at %d",bufidx);
        if(bufidx >= 0)
        {
                size_t bufsize;
                uint8_t *buf = AMediaCodec_getInputBuffer(_pMediaCodec, bufidx, &bufsize);
                //memcpy(buf,(char*)pdata,640*480);
                memcpy(buf,yuvData->data,yuvData->size);
                AMediaCodec_queueInputBuffer(_pMediaCodec, bufidx, 0, yuvData->size, systemnanotime(),0);
        }

        delete yuvData;

        AMediaCodecBufferInfo info;
        ssize_t status;

        status = AMediaCodec_dequeueOutputBuffer(_pMediaCodec, &info, 0);
        while(status >= 0)
        {
            //status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 0);
            size_t bufsize;
            uint8_t *buf = AMediaCodec_getOutputBuffer(_pMediaCodec, status, &bufsize);
            //LOGV("@@@ out put buffer lenght: %d bufix:%d-- %x%x%x%x", info.size,status,buf[0],buf[1],buf[2],buf[3]);
            if(_callableObject)
              {
                NaluStruct *nalu = new NaluStruct(buf,info.size);
//                LOGV("zkzszd type %d",nalu->type);
//                nalu->data = (unsigned char*)malloc(sizeof(unsigned char)*info.size);
//                //nalu->data = new unsigned char[info.size];
//                memcpy(nalu->data,buf,info.size);
//                nalu->size = info.size;
//                nalu->type = nalu->data[4] & 0x1f;
                _callableObject(nalu);
              }

            AMediaCodec_releaseOutputBuffer(_pMediaCodec, status, true);//info.size != 0);
            status = AMediaCodec_dequeueOutputBuffer(_pMediaCodec, &info, 0);
        }
      }break;
    default:
      break;
    }

  }
}
