#ifndef _ENCODE_LOOP_H_
#define _ENCODE_LOOP_H_
#include "../looper/FixedLoop.hpp"
#include "../NativeCodec/NativeCodec.h"
#include "../librtmp/RTMPPusher.hpp"
#include "../base/MyDef.hpp"
#include "../GLES/NativeYUVSurface.h"
#include <stdio.h>
#include <stdlib.h>
#include "Blur.hpp"
#include <functional>
#include <android/log.h>
#define TAG "EncodeLoop"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
using namespace std;
namespace WeiYu
{
#define DEF_WIDTH 480
#define DEF_HEIGHT 640
#define DEF_FRAME_RATE 25
#define DEF_I_FRAME_INTERVAL 1  //1秒
#define DEF_Channel_Count 2
#define DEF_Sample_Rate 15
#define DEF_FORMAT OMX_COLOR_FormatYUV420SemiPlanar

  enum MES
  {
    MES_Camera_data = 1
  };

  class EncodeLoop : public FixedLoop
  {
  public:
    EncodeLoop(int videoWidth,int videoHeight,int videoFramerate,int bitRate,int I_FRAME_INTERVAL,int videoFormat);
    ~EncodeLoop();
  public:
    //JavaVM 全局
    //glSurface surfaceView 控件
    int initYUVSurface(JavaVM* javaVM,jobject glSurface);
    //Callback函数中使用完NaluStruct要delete掉
    void addCallback(function<void(NaluStruct*)> callableObject)
    {
      _callableObject = callableObject;
    }
  private:
    virtual void handle(int what, LoopMsgObj *data);
    AMediaCodec* _pMediaCodec;
    AMediaFormat* _pMediaFormat;  //考虑可能后期可以做成动态帧率

    NativeYUVSurface* _nativeYUVSurface = NULL;         //gles直接渲染yuv

    function<void(NaluStruct*)> _callableObject = NULL;
  private://美颜
    Blur *blur = NULL;
  private:

    int _videoWidth;
    int _videoHeight;
    int _videoFramerate;
    int _videoBitRate;
    int _I_FRAME_INTERVAL;      //关键帧间隔，单位秒
    int _inputFormat;           //编码数据格式
  private:
  };
}

#endif
