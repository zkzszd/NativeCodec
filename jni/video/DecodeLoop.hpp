#ifndef _DECODE_LOOP_H_
#define _DECODE_LOOP_H_
#include "../looper/looper.h"
#include <stdlib.h>
#include <stdio.h>
#include "../librtmp/rtmp.h"
#include "../librtmp/rtmp_sys.h"
#include <android/native_window_jni.h>
#include "../NativeCodec/NativeCodec.h"
#include <android/native_window_jni.h>
#include <android/log.h>
#include "../librtmp/SpsUtil.h"
#define TAG "DecodeLoop"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

namespace WeiYu
{

typedef struct _Nalu
{
        int type;
        int size = 0;
        unsigned char data[102400];
}Nalu;

  class DecodeLoop : public looper
  {
  public:
    ~DecodeLoop();
    DecodeLoop(ANativeWindow *window);
    virtual void handle(int what, LoopMsgObj *data);
  private:
    void parseRTMPVideoPacket(RTMPPacket &packet,Nalu &nalu);
  private:
    DecodeLoop();
    AMediaCodec *_mediaCodec = NULL;
    AMediaFormat *_videoFormat = NULL;
    ANativeWindow* _window = NULL;
  private:
    bool bVideoFirst = true;    //用来标记是否第一次连接（主要是为了解析sps）
    int nVideoCodecId = 0;
    int nVideoWidth = 0;
    int nVideoHeight = 0;
    int nVideoFrameRate = 0;
    int nFileSize = 0;
#define  LOG_FILE 0
#if LOG_FILE
    FILE *_fp;
#endif
  };
}

#endif
