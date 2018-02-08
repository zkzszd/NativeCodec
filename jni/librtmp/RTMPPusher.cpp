#include "RTMPPusher.hpp"

namespace WeiYu
{
  char * put_byte(char *output, uint8_t nVal)
  {
          output[0] = nVal;
          return output + 1;
  }
  char * put_be16(char *output, uint16_t nVal)
  {
          output[1] = nVal & 0xff;
          output[0] = nVal >> 8;
          return output + 2;
  }
  char * put_be24(char *output, uint32_t nVal)
  {
          output[2] = nVal & 0xff;
          output[1] = nVal >> 8;
          output[0] = nVal >> 16;
          return output + 3;
  }
  char * put_be32(char *output, uint32_t nVal)
  {
          output[3] = nVal & 0xff;
          output[2] = nVal >> 8;
          output[1] = nVal >> 16;
          output[0] = nVal >> 24;
          return output + 4;
  }
  char *  put_be64(char *output, uint64_t nVal)
  {
          output = put_be32(output, nVal >> 32);
          output = put_be32(output, nVal);
          return output;
  }
  char * put_amf_string(char *c, const char *str)
  {
          uint16_t len = strlen(str);
          c = put_be16(c, len);
          memcpy(c, str, len);
          return c + len;
  }
  char * put_amf_double(char *c, double d)
  {
          *c++ = AMF_NUMBER;  /* type: Number */
          {
                  unsigned char *ci, *co;
                  ci = (unsigned char *)&d;
                  co = (unsigned char *)c;
                  co[0] = ci[7];
                  co[1] = ci[6];
                  co[2] = ci[5];
                  co[3] = ci[4];
                  co[4] = ci[3];
                  co[5] = ci[2];
                  co[6] = ci[1];
                  co[7] = ci[0];
          }
          return c + 8;
  }

  void RTMPPusher::handle(int what, LoopMsgObj *data)
  {
    //要加是否断开连接逻辑
    if(!isConnect())
    {
        LOGV("开始断线重连");
        if(!connect())
        {
            LOGV("重连失败");
            delete data;
            return;
        }
    }

    switch(what)
    {
    case RTMPPUSHER_MES_H264_DATA:
      {
        NaluStruct* nalu = (NaluStruct*)data;
        if(SendH264Packet((char*)nalu->data,nalu->size,(nalu->type == 0x05) ? true : false,RTMP_GetTime()-_time))
          {
            //LOGV("send pack ok");
          }
        else
          {
            LOGV("at handle send h264 pack fail");
          }
        delete nalu;                       //注意要用new 会调用析构函数，释放内部空间
        break;
      }
    case RTMPPUSHER_MES_AAC_DATA:
      {
        NaluStruct* nalu = (NaluStruct*)data;
        if(SendPacket(RTMP_PACKET_TYPE_AUDIO, (unsigned char*)nalu->data,nalu->size, RTMP_GetTime()-_time))
        {

        }
        else
        {
          LOGV("at handle send audio pack fail");
        }
        delete nalu;                       //注意要用new 会调用析构函数，释放内部空间
        break;
      }
    default:
      break;
    }

  }

  bool RTMPPusher::SendMetadata(LPRTMPMetadata lpMetaData)
  {
          if (lpMetaData == NULL)
          {
                  return false;
          }
          char body[1024] = { 0 };

          char * p = (char *)body;
          p = put_byte(p, AMF_STRING);
          p = put_amf_string(p, "@setDataFrame");

          p = put_byte(p, AMF_STRING);
          p = put_amf_string(p, "onMetaData");

          p = put_byte(p, AMF_OBJECT);
          p = put_amf_string(p, "copyright");
          p = put_byte(p, AMF_STRING);
          p = put_amf_string(p, "firehood");

          p = put_amf_string(p, "width");
          p = put_amf_double(p, lpMetaData->nWidth);

          p = put_amf_string(p, "height");
          p = put_amf_double(p, lpMetaData->nHeight);

          p = put_amf_string(p, "framerate");
          p = put_amf_double(p, lpMetaData->nFrameRate);

          p = put_amf_string(p, "videocodecid");
          p = put_amf_double(p, FLV_CODECID_H264);

          p = put_amf_string(p, "");
          p = put_byte(p, AMF_OBJECT_END);

          if(SendPacket(RTMP_PACKET_TYPE_INFO, (unsigned char*)body, p - body, 0) != 1)
          {
              LOGV("send packet type info failed");
          }

          int i = 0;
          body[i++] = 0x17; // 1:keyframe  7:AVC
          body[i++] = 0x00; // AVC sequence header

          body[i++] = 0x00;
          body[i++] = 0x00;
          body[i++] = 0x00; // fill in 0;   0

                                          // AVCDecoderConfigurationRecord.
          body[i++] = 0x01;               // configurationVersion
          body[i++] = lpMetaData->Sps[1]; // AVCProfileIndication
          body[i++] = lpMetaData->Sps[2]; // profile_compatibility
          body[i++] = lpMetaData->Sps[3]; // AVCLevelIndication
          body[i++] = 0xff;               // lengthSizeMinusOne

                                            // sps nums
          body[i++] = 0xE1;                 //&0x1f
                                            // sps data length
          body[i++] = lpMetaData->nSpsLen >> 8;
          body[i++] = lpMetaData->nSpsLen & 0xff;
          // sps data
          memcpy(&body[i], lpMetaData->Sps, lpMetaData->nSpsLen);
          i = i + lpMetaData->nSpsLen;

          // pps nums
          body[i++] = 0x01; //&0x1f
                                            // pps data length
          body[i++] = lpMetaData->nPpsLen >> 8;
          body[i++] = lpMetaData->nPpsLen & 0xff;
          // sps data
          memcpy(&body[i], lpMetaData->Pps, lpMetaData->nPpsLen);
          i = i + lpMetaData->nPpsLen;

          _time = RTMP_GetTime();

          return SendPacket(RTMP_PACKET_TYPE_VIDEO, (unsigned char*)body, i, 0);
  }

  bool RTMPPusher::SendAudioHeader(char* data,int length)
  {
    if(data == NULL)
      {
        return false;
      }
    RTMPPacket packet;
    RTMPPacket_Reset(&packet);
    RTMPPacket_Alloc(&packet, 4);

    packet.m_body[0] = data[0];
    packet.m_body[1] = data[1];
    packet.m_body[2] = data[2];
    packet.m_body[3] = data[3];//0x10修改为0x90,2016-1-19

    packet.m_headerType  = RTMP_PACKET_SIZE_MEDIUM;
    packet.m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet.m_hasAbsTimestamp = 0;
    packet.m_nChannel   = 2;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = m_pRtmp->m_stream_id;
    packet.m_nBodySize  = 4;

    //调用发送接口
    int nRet = RTMP_SendPacket(m_pRtmp, &packet, 0);
    if (nRet != 1)
    {
        LOGV("RTMP_SendPacket fail %d\n",nRet);
    }
    //int nRet = RTMP_SendPacket(rtmp, &packet, TRUE);
    RTMPPacket_Free(&packet);//释放内存
    return (nRet = 0?true:false);
  }

  bool RTMPPusher::SendH264Packet(char *data,int size, bool bIsKeyFrame, unsigned int nTimeStamp)
  {
          if (data == NULL && size<11)
          {
                  return false;
          }

          unsigned char *body = new unsigned char[size + 9];

          int i = 0;
          if (bIsKeyFrame)
          {
                  body[i++] = 0x17;// 1:Iframe  7:AVC
          }
          else
          {
                  body[i++] = 0x27;// 2:Pframe  7:AVC
          }
          body[i++] = 0x01;// AVC NALU
          body[i++] = 0x00;
          body[i++] = 0x00;
          body[i++] = 0x00;

          // NALU size
          body[i++] = size >> 24;
          body[i++] = size >> 16;
          body[i++] = size >> 8;
          body[i++] = size & 0xff;;

          // NALU data
          memcpy(&body[i], data, size);

          bool bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO, body, i + size, nTimeStamp);
          delete[] body;
          return bRet;
  }

  int RTMPPusher::SendPacket(unsigned int nPacketType, unsigned char *data, unsigned int size, unsigned int nTimestamp)
  {
          if (m_pRtmp == NULL)
          {
                  return FALSE;
          }

          RTMPPacket packet;
          RTMPPacket_Reset(&packet);
          RTMPPacket_Alloc(&packet, size);

          packet.m_packetType = nPacketType;
          packet.m_nChannel = 0x04;
          packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
          packet.m_nTimeStamp = nTimestamp;
          packet.m_nInfoField2 = m_pRtmp->m_stream_id;
          packet.m_nBodySize = size;
          memcpy(packet.m_body, data, size);

          int nRet = RTMP_SendPacket(m_pRtmp, &packet, 0);
          if (nRet != 1)
          {
              //LOGV("RTMP_SendPacket fail %d\n",nRet);
          }

          RTMPPacket_Free(&packet);

          return nRet;
  }
}
