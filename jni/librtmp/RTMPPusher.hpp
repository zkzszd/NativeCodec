#ifndef _RTMP_PUSHER_HPP_
#define _RTMP_PUSHER_HPP_

#include "rtmp.h"
#include "RTMPObject.hpp"
#include "../looper/NaluLoop.hpp"
#include "../util/MediaCodecUtil.h"
#include "../base/MyDef.hpp"
#include <string>
#include <jni.h>
#define TAG "RTMPPusher"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
namespace WeiYu
{
  enum RTMPPusherMES
  {
    RTMPPUSHER_MES_H264_DATA = 1,
    RTMPPUSHER_MES_AAC_DATA = 2
  };

  typedef struct _RTMPMetadata
  {
          // video, must be h264 type
          unsigned int    nWidth;
          unsigned int    nHeight;
          unsigned int    nFrameRate;     // fps
          unsigned int    nVideoDataRate; // bps
          unsigned int    nSpsLen;
          unsigned char   Sps[1024];
          unsigned int    nPpsLen;
          unsigned char   Pps[1024];

          // audio, must be aac type
          bool            bHasAudio;
          unsigned int    nAudioSampleRate;     //audiosamplerate
          unsigned int    nAudioSampleSize;     //audiosamplesize
          unsigned int    nAudioChannels;
          char            pAudioSpecCfg;
          unsigned int    nAudioSpecCfgLen;

  } RTMPMetadata, *LPRTMPMetadata;

  //后期需要优化动态丢帧
  class RTMPPusher : public NaluLoop ,public RTMPObject
  {
    typedef RTMPObject Super;
  public:
    RTMPPusher():RTMPObject(RTMPOBJ_TYPE_PUSH),NaluLoop(30){LOGV("RTMPPusher create!");}
    // ����MetaData
    bool SendMetadata(LPRTMPMetadata lpMetaData);
    bool SendAudioHeader(char* data,int length);
  private:
    virtual void handle(int what, LoopMsgObj *data);
    bool SendH264Packet(char *data,int size, bool bIsKeyFrame, unsigned int nTimeStamp);
    //bool SendAACPacket(char *data,int size, unsigned int nTimeStamp);
    int SendPacket(unsigned int nPacketType, unsigned char *data, unsigned int size, unsigned int nTimestamp);

    unsigned int _time = 0; //在SendMetadata获取时间比较准确

    enum
    {
        FLV_CODECID_H264 = 7,
    };
  };
}

#endif
