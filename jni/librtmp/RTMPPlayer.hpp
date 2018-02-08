#ifndef _RTMP_PLAYER_HPP_ 
#define _RTMP_PLAYER_HPP_
#include "../librtmp/RTMPObject.hpp"
#include <string>
#include <stdlib.h>
#include "../video/DecodeLoop.hpp"
#include "../audio/AudioDecode.hpp"
#include <android/native_window_jni.h>
#include "../NativeCodec/NativeCodec.h"
#include <android/log.h>
#include <thread>
#include <functional>
#include "../base/MyDef.hpp"
#define TAG "RTMPPlayer"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

namespace WeiYu
{
      class RTMPPlayer : public RTMPObject
      {
      public:
        void start();
        void stop() {_readThread_stop = true;}
        bool readPacketThread();
      public:
//              virtual void onVideoBuf(char *NaluData,unsigned int dataLen);
//              virtual void onAudioBuf(char *AudioData, unsigned int dataLen);
//              virtual void onVideoAndAudioInfo(int videoWidth,int videoHeight,int videoFrameRate,
//                                                                              int AudioSmpleRate,int AudioChannel,bool AudioIsStereo);
      public:
        RTMPPlayer(ANativeWindow *window);
        ~RTMPPlayer();
      private:
        void ParseScriptTag(RTMPPacket &packet);
        RTMPPlayer();
        bool _readThread_stop = false;
        std::thread *_thread = NULL;
        DecodeLoop *_decodeLoop = NULL;                   //video decode
        AudioDecode *_audioDecode = NULL;
      private:
        //video and audio info
        int nVideoCodecId = 0;
        int nVideoWidth = 0;
        int nVideoHeight = 0;
        int nVideoFrameRate = 0;
        int nAudioCodecId = 0;
        int nAudioSampleRate = 0;
        int nAudioSampleSize = 0;
        int nAudioChannel = 2;
        int nFileSize = 0;
      };
}

#endif // ! 
