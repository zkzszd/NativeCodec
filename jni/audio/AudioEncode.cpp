#include "AudioEncode.hpp"

#define ENCODE_TYPE "audio/mp4a-latm"
#define PERIOD_TIME 92 //ms

namespace WeiYu
{
  bool AudioEncode::openAudioDevice(int channelCount,int sampleRate)
  {
    _audioChannelCount = channelCount;
    _audioSampleRate = sampleRate;

    _audioFrameSize = sampleRate*PERIOD_TIME/1000;
    _audioBufferSize = _audioFrameSize*channelCount;

    _audioStream = android_OpenAudioDevice();
    if (_audioStream == NULL) {
            LOGV("failed to open audio device ! \n");
    }
    if(android_AudioInitIn(_audioStream,sampleRate,channelCount,_audioFrameSize))
    {
      LOGV("failed to android_AudioInitIn");
      android_CloseAudioDevice(_audioStream);
      _audioStream = NULL;
      return false;
    }

    AMediaFormat_setString(_pMediaFormat, AMEDIAFORMAT_KEY_MIME, ENCODE_TYPE);
    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, _audioChannelCount); //
    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, _audioSampleRate); // ﻿采样率
    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_BIT_RATE, _audioSampleRate*16); //
    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_AAC_PROFILE, 2);
    AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_IS_ADTS, 0);
//    uint8_t es[2] = {0x12, 0x12};
//    AMediaFormat_setBuffer(_pMediaFormat, "csd-0", es, 2);

    //AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_BIT_RATE, _audioBufferSize);
    //AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_FLAC_COMPRESSION_LEVEL, 2);   //2对应 AACObjectLC

    int code = AMediaCodec_configure(_pMediaCodec,_pMediaFormat,NULL,NULL, AMEDIACODEC_CONFIGURE_FLAG_ENCODE);//编码不能设置窗口，设置会崩溃
    if( code != AMEDIA_OK)
    {
            LOGV("zkzszd failed AMediaCodec_configure: %d ", code);
    }
    AMediaCodec_start(_pMediaCodec);

    return true;
  }

  void AudioEncode::thread_fun()
  {
    int64_t nanoTime = systemnanotime();
    int read_len = 0;
    while(!isEncodeStop)
    {
        int bufidx = AMediaCodec_dequeueInputBuffer(_pMediaCodec, 2000);
        //LOGV("dequeueInputBuffer at %d",bufidx);
        if(bufidx >= 0)
        {
            size_t bufsize;
            uint8_t *buf = AMediaCodec_getInputBuffer(_pMediaCodec, bufidx, &bufsize);

            char *buffer = (char*)malloc(sizeof(char)*bufsize);
            read_len = android_AudioIn(_audioStream,(short*)buffer,bufsize/2);
            memcpy(buf,buffer,read_len);
            free(buffer);
            AMediaCodec_queueInputBuffer(_pMediaCodec, bufidx, 0, read_len, (systemnanotime() - nanoTime) / 1000,0);
        }

        AMediaCodecBufferInfo info;
        ssize_t status;
        status = AMediaCodec_dequeueOutputBuffer(_pMediaCodec, &info, 0);
        while(status >= 0)
        {
            size_t bufsize;
            uint8_t *buf = AMediaCodec_getOutputBuffer(_pMediaCodec, status, &bufsize);

            if(_callableObject)
            {
                NaluStruct *naluPack = new NaluStruct(buf,info.size);
                naluPack->type = -1;
                _callableObject(naluPack);
            }

            //LOGV("write len %d %x%x",info.size,buf[0],buf[1]);
            fwrite(buf,1,info.size,fp);

            AMediaCodec_releaseOutputBuffer(_pMediaCodec, status, true);//info.size != 0);
            status = AMediaCodec_dequeueOutputBuffer(_pMediaCodec, &info, 0);
        }
    }
  }

  void AudioEncode::startRun()
  {
    if(!_thread)
    {
      _thread = new std::thread(std::bind(&AudioEncode::thread_fun,this));
    }
    _thread->detach();
  }

  AudioEncode::~AudioEncode()
  {
    isEncodeStop = true;

    if(_thread)
    {
      delete _thread;
      _thread = NULL;
    }

    android_CloseAudioDevice(_audioStream);

    AMediaFormat_delete(_pMediaFormat);
    AMediaCodec_delete(_pMediaCodec);

    if(fp)
      {
        fclose(fp);
      }
  }

  AudioEncode::AudioEncode()
  {
    _pMediaCodec = AMediaCodec_createEncoderByType(ENCODE_TYPE);
    _pMediaFormat = AMediaFormat_new();
    fp = fopen("/storage/emulated/0/rec.aac","wb+");
  }
}
