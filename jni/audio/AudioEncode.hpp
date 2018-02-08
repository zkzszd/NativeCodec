#ifndef _AUDIO_ENCODE_HPP_
#define _AUDIO_ENCODE_HPP_

#include <stdio.h>
#include "opensl_io.h"
#include <thread>
#include <functional>
#include "../base/MyDef.hpp"
#include "../NativeCodec/NativeCodec.h"
#include "../util/MediaCodecUtil.h"
#include <android/log.h>
using namespace std;

#define TAG "AudioEncode"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

namespace WeiYu
{
  class AudioEncode
  {
  public:
    bool openAudioDevice(int channelCount,int sampleRate);
    void startRun();
    void pause() {isEncodeStop = true;}
    //Callback函数中使用完NaluStruct要delete掉
    void addCallback(function<void(NaluStruct*)> callableObject)
    {
      _callableObject = callableObject;
    }
  public:
    AudioEncode();
    virtual ~AudioEncode();
  private:
    void thread_fun();
    bool isEncodeStop = false;
    std::thread *_thread = NULL;
    function<void(NaluStruct*)> _callableObject = NULL;
    FILE *fp = NULL;
  private:
    OPENSL_STREAM* _audioStream = NULL;

    AMediaCodec* _pMediaCodec = NULL;
    AMediaFormat* _pMediaFormat = NULL;  //考虑可能后期可以做成动态帧率

    int _audioChannelCount;     //声道数 通常所说的立体声就是技术层面的双声道
    int _audioSampleRate;       //
    int _audioBitRate;
    int _audioFrameSize;
    int _audioBufferSize;
  };
}

#endif //_AUDIO_ENCODE_HPP_
