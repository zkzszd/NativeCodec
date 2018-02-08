#include "RTMPPlayer.hpp"
#include <string.h>

namespace WeiYu
{
  RTMPPlayer::RTMPPlayer(ANativeWindow *window):RTMPObject()
  {
    _decodeLoop = new DecodeLoop(window);
    _audioDecode = new AudioDecode();
  }

  RTMPPlayer::~RTMPPlayer()
  {
    _readThread_stop = true;
    if(_thread)
      {
        delete _thread;
        _thread = NULL;
      }
    if(_decodeLoop)
      {
        delete _decodeLoop;
        _decodeLoop = NULL;
      }
    if(_audioDecode)
      {
        delete _audioDecode;
        _audioDecode = NULL;
      }
  }

  void RTMPPlayer::ParseScriptTag(RTMPPacket &packet)
  {
      LOGV("begin parse info %d",packet.m_nBodySize);
      AMFObject obj;
      AVal val;
      AMFObjectProperty * property;
      AMFObject subObject;
      if (AMF_Decode(&obj, packet.m_body, packet.m_nBodySize, FALSE) < 0)
      {
              LOGV("%s, error decoding invoke packet", __FUNCTION__);
      }
      AMF_Dump(&obj);
      LOGV(" amf obj %d",obj.o_num);
      for (int n = 0; n < obj.o_num; n++)
      {
          property = AMF_GetProp(&obj, NULL, n);
          if (property != NULL)
          {
              if (property->p_type == AMF_OBJECT)
              {
                  AMFProp_GetObject(property, &subObject);
                  for (int m = 0; m < subObject.o_num; m++)
                  {
                      property = AMF_GetProp(&subObject, NULL, m);
                      LOGV("val = %s",property->p_name.av_val);
                      if (property != NULL)
                      {
                          if (property->p_type == AMF_OBJECT)
                          {

                          }
                          else if (property->p_type == AMF_BOOLEAN)
                          {
                              int bVal = AMFProp_GetBoolean(property);
                              if (strncasecmp("stereo", property->p_name.av_val, property->p_name.av_len) == 0)
                              {
                                      nAudioChannel =  bVal > 0 ? 2 : 1;
                              }
                          }
                          else if (property->p_type == AMF_NUMBER)
                          {
                              double dVal = AMFProp_GetNumber(property);
                              if (strncasecmp("width", property->p_name.av_val, property->p_name.av_len) == 0)
                              {
                                      nVideoWidth = (int)dVal;
                                      LOGV("parse widht %d",nVideoWidth);
                              }
                              else if (strcasecmp("height", property->p_name.av_val) == 0)
                              {
                                      nVideoHeight = (int)dVal;
                                      LOGV("parse Height %d",nVideoHeight);
                              }
                              else if (strcasecmp("framerate", property->p_name.av_val) == 0)
                              {
                                      nVideoFrameRate = (int)dVal;
                              }
                              else if (strcasecmp("videocodecid", property->p_name.av_val) == 0)
                              {
                                      nVideoCodecId = (int)dVal;
                              }
                              else if ((strcasecmp("audiosamplerate", property->p_name.av_val) == 0)||
                                  (strcasecmp("audiodatarate", property->p_name.av_val) == 0))
                              {
                                      nAudioSampleRate = (int)dVal;
                              }
                              else if (strcasecmp("audiosamplesize", property->p_name.av_val) == 0)
                              {
                                      nAudioSampleSize = (int)dVal;
                              }
                              else if (strcasecmp("audiocodecid", property->p_name.av_val) == 0)
                              {
                                      nAudioCodecId = (int)dVal;
                              }
                              else if (strcasecmp("filesize", property->p_name.av_val) == 0)
                              {
                                      nFileSize = (int)dVal;
                              }
                          }
                          else if (property->p_type == AMF_STRING)
                          {
                              AMFProp_GetString(property, &val);
                          }
                      }
                  }
              }
              else
              {
//                      AMFProp_GetString(property, &val);
//                      LOGV("val = %s",val.av_val);
              }
          }
      }
  }

  void RTMPPlayer::start()
  {
    if(!_thread)
      {
        _thread = new std::thread(std::bind(&RTMPPlayer::readPacketThread,this));
      }
    _thread->detach();
  }

      bool RTMPPlayer::readPacketThread()
      {
        //此处可以优化
        RTMPPacket packet = {0};
        while (!_readThread_stop)
        {
            //短线重连
            if(!isConnect())
            {
                LOGV("短线重连 re connect");
                if(!connect(m_strUrl))      //重连失败
                {
                    LOGV("短线重连 reConnect fail %s",m_strUrl.c_str());
                    msleep(10);
                    continue;
                }
            }

            RTMP_ReadPacket(m_pRtmp, &packet);
            if (RTMPPacket_IsReady(&packet))    //�Ƿ��ȡ���
            {
                    if (!packet.m_nBodySize)
                            continue;
                    if (packet.m_packetType == RTMP_PACKET_TYPE_VIDEO)
                    {
                        //本想用share_ptr,不过并没有特别发杂情况，loop处理完直接手动释放就ok
                        MsgRTMPPPack *videoPacket = new MsgRTMPPPack(packet);
                        _decodeLoop->post(0,videoPacket);
                    }
                    else if (packet.m_packetType == RTMP_PACKET_TYPE_AUDIO)
                    {
                        //LOGV("player get audio pack");
                        MsgRTMPPPack *audioPacket = new MsgRTMPPPack(packet);
                        _audioDecode->post(0,audioPacket);
                    }
                    else if (packet.m_packetType == RTMP_PACKET_TYPE_INFO)
                    {
                        LOGV("onReadVideoAndAudioInfo ");
                        ParseScriptTag(packet);
                        RTMPPacket_Free(&packet);

//                          RTMPPacket *infoPacket = (RTMPPacket *)malloc(sizeof(RTMPPacket));
//                          memcpy(infoPacket,&packet,sizeof(RTMPPacket));
//                          _decodeLoop->post(onReadVideoAndAudioInfo,(void*)infoPacket);
                    }
                    memset(&packet,0,sizeof(RTMPPacket));
                    RTMP_ClientPacket(m_pRtmp, &packet);
                    //RTMPPacket_Free(&packet);
            }
            else
            {
                    msleep(5);
            }
         }
      }
}
