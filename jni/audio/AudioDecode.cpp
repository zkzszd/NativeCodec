#include "AudioDecode.hpp"
#include <thread>
#include "../base/MyDef.hpp"

#define DECODE_TYPE "audio/mp4a-latm"
#define PERIOD_TIME 46 //ms


namespace WeiYu
{
  AudioDecode::AudioDecode()
    //:_audioChannelCount(ChannelCount),_audioSampleRate(SampleRate)
  {
    _pMediaCodec = AMediaCodec_createDecoderByType(DECODE_TYPE);
    _pMediaFormat = AMediaFormat_new();
  }

  AudioDecode::~AudioDecode()
  {
    if(_pMediaCodec)
    {
      AMediaCodec_delete(_pMediaCodec);
      _pMediaCodec = NULL;
    }
    if(_pMediaFormat)
    {
      AMediaFormat_delete(_pMediaFormat);
      _pMediaFormat = NULL;
    }
    if(_audioStream)
      {
        android_CloseAudioDevice(_audioStream);
        _audioStream = NULL;
      }
    if(_speaker)
      {
//        //析构函数已调用
//        _speaker->stop();
//        _speaker->close();
        delete _speaker;
        _speaker = NULL;
      }
  }

    void AudioDecode::handle(int what, LoopMsgObj *obj)
    {
      MsgRTMPPPack *msgPack = (MsgRTMPPPack*)obj;
      RTMPPacket *packet = msgPack->rtmpPack;
      char *data = packet->m_body;
      if(isFirst)
      {
        LOGV("init audio first");
        isFirst = false;
        //ACC音频同步包(1bytes+3bytes AccAudioData) (4bytes 包含了AACDecoderSpecific和AudioSpecificConfig)
        AACDecoderSpecific adsAACDecoderSpecific = { 0 };
        AudioSpecificConfig ascAudioSpecificConfig = { 0 };
        adsAACDecoderSpecific.nAudioFortmatType = (data[0] & 0xf0) >> 4;  //音频编码类型（0：Liner PCM platform endian，1：PCM，2：mp3，4：Nellymoser 16-kHz mono,5:Nellymoser 8-kHz mono,6:Nellymoser,7:G.711 A-law logarithmic PCM,8:G.711 mu-law logarithmic PCM,9:reserved,10: AAC,14:MP3 8-Khz,15:Device-specific sound）
        adsAACDecoderSpecific.nAudioSampleType = (data[0] & 0x0c) >> 2;   //音频采样率(0:5.5kHz,1:11KHz,2:22 kHz,3:44 kHz)
        adsAACDecoderSpecific.nAudioSizeType = (data[0] & 0x02) >> 1; //音频采样精度(0:8bits,1:16bits)
        adsAACDecoderSpecific.nAudioStereo = data[0] & 0x01;//是否立体声(0:sndMono,1:sndStereo)
        if(adsAACDecoderSpecific.nAudioFortmatType == 10)
        {
            //The following values are defined:
            //0 = AAC sequence header
            //1 = AAC raw
            adsAACDecoderSpecific.nAccPacketType = data[1];
            unsigned short audioSpecificConfig = 0;
            audioSpecificConfig = (data[2] & 0xff) << 8;
            audioSpecificConfig += 0x00ff & data[3];
            ascAudioSpecificConfig.nAudioObjectType = (audioSpecificConfig & 0xF800) >> 11;
            ascAudioSpecificConfig.nSampleFrequencyIndex = (audioSpecificConfig & 0x0780) >> 7;
            ascAudioSpecificConfig.nChannels = (audioSpecificConfig & 0x78) >> 3;
            ascAudioSpecificConfig.nFrameLengthFlag = (audioSpecificConfig & 0x04) >> 2;
            ascAudioSpecificConfig.nDependOnCoreCoder = (audioSpecificConfig & 0x02) >> 1;
            ascAudioSpecificConfig.nExtensionFlag = audioSpecificConfig & 0x01;
            LOGV("nAudioObjectType %d",ascAudioSpecificConfig.nAudioObjectType);
            LOGV("nSampleFrequencyIndex %d",ascAudioSpecificConfig.nSampleFrequencyIndex);
            LOGV("nChannels %d",ascAudioSpecificConfig.nChannels);
            LOGV("nFrameLengthFlag %d",ascAudioSpecificConfig.nFrameLengthFlag);
            LOGV("nDependOnCoreCoder %d",ascAudioSpecificConfig.nDependOnCoreCoder);
            LOGV("nExtensionFlag %d",ascAudioSpecificConfig.nExtensionFlag);
        }
        else if(adsAACDecoderSpecific.nAudioFortmatType == 11)
        {
            //speex类型数据时，后面的4位数据不起作用，固定的是16KHZ，单声道，16bit/sample
            adsAACDecoderSpecific.nAudioStereo = 0;
            adsAACDecoderSpecific.nAudioSizeType = 1;
            adsAACDecoderSpecific.nAudioSampleType = 4;
        }
        if(adsAACDecoderSpecific.nAudioStereo)
            _audioChannelCount = 2;
        else
            _audioChannelCount = 1;
        if(adsAACDecoderSpecific.nAudioSampleType == 3)
            _audioSampleRate = 48000;

        LOGV("count %d  nAudioSampleType=%d",_audioChannelCount,adsAACDecoderSpecific.nAudioSampleType);

        _speaker = new NXSLSpeaker();
        NXSDKAudioFormat audioFormat;
        audioFormat.channels = 2;
        audioFormat.sample_bits = 16;
        audioFormat.sample_rate = _audioSampleRate;
        _speaker->open(audioFormat,2048,8);
        _speaker->start();

//        //open audio drive
//        int _audioFrameSize = 4096;// 8192;//_audioSampleRate*PERIOD_TIME/1000;
//        _audioStream = android_OpenAudioDevice();
//        if (_audioStream == NULL) {
//                LOGV("failed to open audio device ! \n");
//        }
//        if(android_AudioInitOut(_audioStream,_audioSampleRate,_audioChannelCount,_audioFrameSize))
//        {
//          LOGV("failed to android_AudioInitIn");
//          android_CloseAudioDevice(_audioStream);
//          _audioStream = NULL;
//          return;
//        }
//        std::thread *_thread = new std::thread(std::bind(&bqPlayerCallback,_audioStream->bqPlayerBufferQueue,_audioStream));
//        _thread->detach();
//        delete _thread;
        //bqPlayerCallback(_audioStream->bqPlayerBufferQueue,_audioStream);

        AMediaFormat_setString(_pMediaFormat, AMEDIAFORMAT_KEY_MIME, DECODE_TYPE);
        AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, _audioChannelCount); //
        AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, _audioSampleRate); // ﻿采样率
        //AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_BIT_RATE, _audioSampleRate*16); //
//        AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_AAC_PROFILE, 2);
//        AMediaFormat_setInt32(_pMediaFormat, AMEDIAFORMAT_KEY_IS_ADTS, 0);
        AMediaFormat_setBuffer(_pMediaFormat, "csd-0",data+2, 2);
        int code = AMediaCodec_configure(_pMediaCodec,_pMediaFormat,NULL,NULL, AMEDIACODEC_CONFIGURE_FLAG_DECODE);
        if( code != AMEDIA_OK)
        {
                LOGV("zkzszd failed AMediaCodec_configure: %d ", code);
        }
        AMediaCodec_start(_pMediaCodec);
        delete msgPack;
      }
      else
      {
          if(packet->m_nBodySize > 2)
          {
              if(data[1] == 1)
              {
                  ssize_t bufidx;
                  bufidx = AMediaCodec_dequeueInputBuffer(_pMediaCodec, 2000);                 //后面是超时时间
                  if (bufidx >= 0)
                  {
                      size_t bufsize;
                      uint8_t *buf = AMediaCodec_getInputBuffer(_pMediaCodec, bufidx, &bufsize);
                      memcpy(buf,packet->m_body+2,packet->m_nBodySize-2);
                      AMediaCodec_queueInputBuffer(_pMediaCodec, bufidx, 0, packet->m_nBodySize-2,packet->m_nTimeStamp,0);
                          //_readThread_stop ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
                  }
                  ssize_t status;
                  AMediaCodecBufferInfo info;
                  status = AMediaCodec_dequeueOutputBuffer(_pMediaCodec, &info, 2000);
                  while(status >= 0)
                  {
                        size_t bufsize;
                        uint8_t *buf = AMediaCodec_getOutputBuffer(_pMediaCodec, status, &bufsize);

                        NXPtr<NXBuffer> abuffer = new NXBuffer(NXBufferAudio,info.size);//new NXBuffer(info.size) ;
                        uint8_t* audioBuffer = abuffer->getPlanePtr(0);
                        memcpy(audioBuffer, buf, info.size);
                        _speaker->speak(abuffer);


//                        //android_AudioOutNoWait不能保证buf全部写入
//                        short *pcm = (short*)buf;
//                        int length = info.size/2;
//                        int put_len = 0;
//                        while(put_len < info.size/2)
//                        {
//                            put_len += android_AudioOutNoWait(_audioStream,pcm,length);
//                            length = info.size/2 - put_len;
//                            pcm += put_len;
//                        }


                        //android_AudioOutNoWait(_audioStream,(short*)buf,info.size/2);

                        AMediaCodec_releaseOutputBuffer(_pMediaCodec, status, true);//info.size != 0);
                        status = AMediaCodec_dequeueOutputBuffer(_pMediaCodec, &info, 0);
                  }
              }
          }
          delete msgPack;
      }

    }
}
