#pragma once
#include "rtmp.h"   
#include "rtmp_sys.h"   
#include "amf.h"   
#include <stdio.h>  
#include <string>
#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#endif // _WIN32
#include <android/log.h>
#define TAG "RTMPObject"
#define LOG(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)


namespace WeiYu	//δ��
{
    enum RTMPOBJ_TYPE
    {
      RTMPOBJ_TYPE_UNKNOW,
      RTMPOBJ_TYPE_PLAY,
      RTMPOBJ_TYPE_PUSH
    };

      class RTMPObject
      {
      public:
          // ���ӵ�RTMP Server   //由于crtmpserver是每个一段时间(默认8s)发送数据包,需大于发送间隔才行
          virtual bool connect(std::string str_url);
          //必须确保已经设置过url
          bool connect();
          void setConnectUrl(std::string& str_url);
          // �Ͽ�����
          void Disconnect();

          bool isConnect();
          //是否接受音频
          bool setReceiveAudio(bool isRecAudio);
          bool setReceiveVideo(bool isRecVideo);
      public:
          RTMPObject();
          RTMPObject(RTMPOBJ_TYPE _rtmp_obj_type);
          RTMPObject(RTMPOBJ_TYPE _rtmp_obj_type,std::string& str_url);
          //次构造函数默认构造player
          RTMPObject(std::string& str_url,bool isRecAudio,bool isRecVideo);
          virtual ~RTMPObject();
      private:
              bool _InitRtmp();
              RTMPOBJ_TYPE _rtmp_obj_type;
      protected:
              RTMP* m_pRtmp;
              std::string m_strUrl;
              bool _enableVideo;      //是否打开视频
              bool _enableAudio;      //是否打开音频
      };
}
