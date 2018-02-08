#include "DecodeLoop.hpp"
#include "../util/MediaCodecUtil.h"
#include "../base/MyDef.hpp"
#include<string.h>
static unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
namespace WeiYu
{
    DecodeLoop::DecodeLoop(ANativeWindow *window):_window(window),looper()
    {
            const char* mine = "video/avc";
            _mediaCodec = AMediaCodec_createDecoderByType(mine);
            _videoFormat = AMediaFormat_new();
#if LOG_FILE
            _fp = fopen("/storage/emulated/0/zkzszd.h264","wb");
#endif
    }

    DecodeLoop::~DecodeLoop()
    {
    	if (_window) {
            ANativeWindow_release(_window);
            _window = NULL;
        }
        if(_mediaCodec)
          {
            AMediaCodec_delete(_mediaCodec);
            _mediaCodec = NULL;
          }
        if(_videoFormat)
          {
            AMediaFormat_delete(_videoFormat);
            _videoFormat = NULL;
          }
#if LOG_FILE
        if(_fp)
          {
            fclose(_fp);
          }
#endif
    }

    void DecodeLoop::parseRTMPVideoPacket(RTMPPacket &packet,Nalu &nalu)
    {
      char* data = packet.m_body;
//      result = pc.m_body[0];
//      bool bIsKeyFrame = false;
//      if (result == 0x17)//I frame
//      {
//              bIsKeyFrame = true;
//      }
//      else if (result == 0x27)
//      {
//              bIsKeyFrame = false;
//      }
//      static unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
      if (bVideoFirst)
      {
          LOGV("init video first");
          nalu.data[0] = 0x00;
          nalu.data[1] = 0x00;
          nalu.data[2] = 0x00;
          nalu.data[3] = 0x01;
          nalu.size = 4;
          //Access to SPS
          int spsnum = data[10] & 0x1f;
          int number_sps = 11;
          int count_sps = 1;
          while (count_sps <= spsnum) {
              size_t spslen = (data[number_sps] & 0x000000FF) << 8 | (data[number_sps + 1] & 0x000000FF);
              number_sps += 2;
              //LOGV("sps size %d %d  %d %d",nalu.size,number_sps,spslen,packet.m_nBodySize);
              memcpy((void*)(nalu.data+nalu.size),data + number_sps,spslen);
              nalu.size += spslen;
//              memcpy((void*)(nalu.data+nalu.size),start_code,4);
//              LOGV("ssp size %d %d  %d %d",nalu.size,number_sps,spslen,packet.m_nBodySize);
//              nalu.size += 4;
              //fwrite(data + number_sps, 1, spslen, fp);
              //fwrite(start_code, 1, 4, fp);
              number_sps += spslen;
              count_sps++;
          }
          //如果sps成功解析到视频信息，覆盖info解析到的视频信息
          int spsWidth = 0,spsHeight = 0,spsFrameRate = 0;
          if(Util_decode_sps(nalu.data, nalu.size, spsWidth, spsHeight,spsFrameRate))
            {
              LOGV("h264 decode sps %d %d %d",spsWidth,spsHeight,spsFrameRate);
              if(spsWidth > 0)
                nVideoWidth = spsWidth;
              if(spsHeight > 0)
                nVideoHeight = spsHeight;
              if(spsFrameRate > 0)
                nVideoFrameRate = spsFrameRate;
            }

          AMediaFormat_setBuffer(_videoFormat, "csd-0", nalu.data, nalu.size);
#if LOG_FILE
          fwrite(nalu.data, 1, nalu.size, _fp);
#endif

          nalu.size = 0;

          //Get PPS
          int ppsnum = data[number_sps] & 0x1f;
          int number_pps = number_sps + 1;
          int count_pps = 1;
          while (count_pps <= ppsnum) {
              size_t ppslen = (data[number_pps] & 0x000000FF) << 8 | data[number_pps + 1] & 0x000000FF;
              number_pps += 2;
              memcpy((void*)(nalu.data+nalu.size),start_code,4);
              //LOGV("pps11 size %d %d  %d %d",nalu.size,number_pps,ppslen,packet.m_nBodySize);
              nalu.size += 4;
              //LOGV("pps size %d %d  %d %d",nalu.size,number_pps,ppslen,packet.m_nBodySize);
              memcpy((void*)(nalu.data+nalu.size),data + number_pps,ppslen);
              nalu.size += ppslen;

              number_pps += ppslen;
              count_pps++;
          }
          AMediaFormat_setString(_videoFormat, AMEDIAFORMAT_KEY_MIME, "video/avc");
          AMediaFormat_setInt32(_videoFormat, AMEDIAFORMAT_KEY_WIDTH, 640); // 视频宽度
          AMediaFormat_setInt32(_videoFormat, AMEDIAFORMAT_KEY_HEIGHT, 480); // ﻿视频高度
          AMediaFormat_setInt32(_videoFormat, AMEDIAFORMAT_KEY_FRAME_RATE, nVideoFrameRate);//帧速率
          //AMediaFormat_setInt32(_videoFormat, AMEDIAFORMAT_KEY_BIT_RATE, 125000);
          AMediaFormat_setBuffer(_videoFormat, "csd-1", nalu.data, nalu.size);
#if LOG_FILE
          fwrite(nalu.data, 1, nalu.size, _fp);
#endif
          nalu.size = 0;
          int code = AMediaCodec_configure(_mediaCodec, _videoFormat,_window,NULL, 0);
          if( code != AMEDIA_OK)
          {
              LOGV("zkzszd failed AMediaCodec_configure: %d ", code);
          }
          AMediaCodec_start(_mediaCodec);
          bVideoFirst = false;
      }
      else
      {
          //AVCNALU
          int len = 0;
          int num = 5;
          while (num<packet.m_nBodySize)
          {
              len = (data[num] & 0x000000FF) << 24 | (data[num + 1] & 0x000000FF) << 16 | (data[num + 2] & 0x000000FF) << 8 | data[num + 3] & 0x000000FF;

              num += 4;
              memcpy((void*)(nalu.data+nalu.size),start_code,4);
              nalu.size += 4;
              //会出现len 》packet.m_nBodySize
              if(len > packet.m_nBodySize)
                {
                  LOGV("------------len  > body size");
                  int temp_len = packet.m_nBodySize - num;
                  memcpy((void*)(nalu.data+nalu.size),data + num,temp_len);
                  nalu.size += temp_len;
                  break;
                }
              memcpy((void*)(nalu.data+nalu.size),data + num,len);
              nalu.size += len;
              num = num + len;
          }
#if LOG_FILE
          fwrite(nalu.data, 1, nalu.size, _fp);
#endif
      }
    }

      void DecodeLoop::handle(int what, LoopMsgObj *obj)
      {
          MsgRTMPPPack *packet = (MsgRTMPPPack*)obj;
          Nalu nalu;
          parseRTMPVideoPacket(*packet->rtmpPack,nalu);

          ssize_t bufidx;
          bufidx = AMediaCodec_dequeueInputBuffer(_mediaCodec, 2000);                 //后面是超时时间
          if (bufidx >= 0)
          {
                  size_t bufsize;
                  uint8_t *buf = AMediaCodec_getInputBuffer(_mediaCodec, bufidx, &bufsize);
                  memcpy(buf,nalu.data,nalu.size);
                  AMediaCodec_queueInputBuffer(_mediaCodec, bufidx, 0, nalu.size ,packet->rtmpPack->m_nTimeStamp,0);
                      //_readThread_stop ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
          }
          delete packet;

          ssize_t status;
          AMediaCodecBufferInfo info;
          status = AMediaCodec_dequeueOutputBuffer(_mediaCodec, &info, 2000);
          while(status >= 0)
          {
                  size_t bufsize;
                  uint8_t *buf = AMediaCodec_getOutputBuffer(_mediaCodec, status, &bufsize);
                  AMediaCodec_releaseOutputBuffer(_mediaCodec, status, true);//info.size != 0);
                  status = AMediaCodec_dequeueOutputBuffer(_mediaCodec, &info, 0);
                  //usleep(1000000/nVideoFrameRate);             //usleep单位微秒
          }
      }
}
