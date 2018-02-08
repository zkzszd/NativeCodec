#ifndef _AUDIO_DECODE_H_
#define _AUDIO_DECODE_H_

#include <stdio.h>
#include "../librtmp/rtmp.h"
#include "../NativeCodec/NativeCodec.h"
#include "opensl_io.h"
#include "../looper/looper.h"
#include "../util/MediaCodecUtil.h"
#include "NXSLSpeaker.h"

#include <android/log.h>
#define TAG "AudioDecode"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

namespace WeiYu
{
  //ACC音频ADTS
  typedef struct tagAACDecoderSpecific
  {
      unsigned char nAudioFortmatType;    //音频编码类型（0：Liner PCM platform endian，1：PCM，2：mp3，4：Nellymoser 16-kHz mono,5:Nellymoser 8-kHz mono,6:Nellymoser,7:G.711 A-law logarithmic PCM,8:G.711 mu-law logarithmic PCM,9:reserved,10: AAC,14:MP3 8-Khz,15:Device-specific sound）
      unsigned char nAudioSampleType;     //音频采样率(0:5.5kHz,1:11KHz,2:22 kHz,3:44.1 kHz)对于AAC总是3
      unsigned char nAudioSizeType;       //音频采样精度(0:8bits,1:16bits)
      unsigned char nAudioStereo;         //是否立体声(0:sndMono,1:sndStereo)
      unsigned char nAccPacketType;
  }AACDecoderSpecific;

  typedef struct tagAudioSpecificConfig
  {
      unsigned char nAudioObjectType;
      unsigned char nSampleFrequencyIndex;
      unsigned char nChannels;
      unsigned char nFrameLengthFlag;
      unsigned char nDependOnCoreCoder;
      unsigned char nExtensionFlag;
  }AudioSpecificConfig;

  class AudioDecode : public looper
  {
  public:
    AudioDecode();
    virtual ~AudioDecode();
  public:

  private:
    virtual void handle(int what, LoopMsgObj *data);

  private:
    AMediaCodec* _pMediaCodec = NULL;
    AMediaFormat* _pMediaFormat = NULL;  //考虑可能后期可以做成动态帧率

    OPENSL_STREAM* _audioStream = NULL;
    NXSLSpeaker *_speaker = NULL;
    int _audioChannelCount = 0;     //声道数 通常所说的立体声就是技术层面的双声道
    int _audioSampleRate = 0;       //
    int _audioBitRate = 0;          //
    bool isFirst = true;
  };
}

#endif
