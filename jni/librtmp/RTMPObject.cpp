#include "RTMPObject.hpp"
#define DEF_TIMEOUT     30      /* seconds */
#define DEF_BUFTIME     (10 * 60 * 60 * 1000)   /* 10 hours default */

namespace WeiYu		//δ��
{

	bool RTMPObject::_InitRtmp()
	{
	  bool ret_code = true;
#ifdef WIN32
              WORD version;
              WSADATA wsaData;
              version = MAKEWORD(1, 1);
              ret_code = (WSAStartup(version, &wsaData) == 0) ? true : false;
#endif
              LOG("at rtmp object create");
              m_pRtmp = RTMP_Alloc();
              RTMP_Init(m_pRtmp);
              return ret_code;
	}

	RTMPObject::RTMPObject():
            _rtmp_obj_type(RTMPOBJ_TYPE_UNKNOW),
            _enableVideo(true),
            _enableAudio(true)
	{
	  _InitRtmp();
	}

	RTMPObject::RTMPObject(RTMPOBJ_TYPE _rtmp_obj_type):
	    _rtmp_obj_type(_rtmp_obj_type),
	    _enableVideo(true),
	    _enableAudio(true)
	{
	  _InitRtmp();
	}

	RTMPObject::RTMPObject(RTMPOBJ_TYPE _rtmp_obj_type,std::string& str_url):
            _rtmp_obj_type(_rtmp_obj_type),
            m_strUrl(str_url),
            _enableVideo(true),
            _enableAudio(true)
	{
	  _InitRtmp();
	}

	RTMPObject::RTMPObject(std::string& str_url,bool isRecAudio,bool isRecVideo):
            _rtmp_obj_type(RTMPOBJ_TYPE_PLAY),
            m_strUrl(str_url),
            _enableVideo(isRecAudio),
            _enableAudio(isRecVideo)
	{
	    _InitRtmp();
	}

	RTMPObject::~RTMPObject()
	{
              if (isConnect())
              {
                      Disconnect();
              }
              RTMP_Free(m_pRtmp);
              m_pRtmp = NULL;
#ifdef WIN32
		WSACleanup();
#endif // WIN32

	}

	void RTMPObject::setConnectUrl(std::string& str_url)
	{
	  m_strUrl = str_url;
	}

	bool RTMPObject::setReceiveAudio(bool isRecAudio)
	{
	  if(isRecAudio == _enableAudio)
	    return true;
	  if(isConnect())
	  {
	      LOG("zkzszd RTMP_SendReceiveAudio");
	      if(RTMP_SendReceiveAudio(m_pRtmp,isRecAudio))
	      {
	          isRecAudio = _enableAudio;
	          return true;
	      }
	  }
	  else
	    isRecAudio = _enableAudio;
	  return false;
	}

	bool RTMPObject::setReceiveVideo(bool isRecVideo)
	{
	    if(isRecVideo == _enableVideo)
	      return true;
	    if(isConnect())
	    {
	        if(RTMP_SendReceiveVideo(m_pRtmp, isRecVideo))
                {
	            isRecVideo = _enableVideo;
	            return true;
                }
	    }
	    else
	      isRecVideo = _enableVideo;

	    return false;
	}

	bool RTMPObject::isConnect()
	{
	  return RTMP_IsConnected(m_pRtmp);
	}

	void RTMPObject::Disconnect()
	{
	      RTMP_Close(m_pRtmp);
	}

	bool RTMPObject::connect()
        {
	  //断线重连必须执行次操作，才能重现连上（比较疑惑）
	  RTMP_Free(m_pRtmp);
          m_pRtmp = RTMP_Alloc();
          RTMP_Init(m_pRtmp);

	  LOG("base begin connect");
          //set connection timeout,default 30s
          m_pRtmp->Link.timeout = 10;
          if (RTMP_SetupURL(m_pRtmp, (char*)m_strUrl.c_str())<0)
          {
              LOG("RTMP_SetupURL failed!");
                  return FALSE;
          }
          m_pRtmp->Link.lFlags |= RTMP_LF_LIVE;           // ��ֱ������? then we can't seek/resume
          RTMP_SetBufferMS(m_pRtmp, 3600*1000);//1hour
          if(_rtmp_obj_type == RTMPOBJ_TYPE_PUSH)
            RTMP_EnableWrite(m_pRtmp);          /*设置可写,即发布流,这个函数必须在连接前使用,否则无效*/
          if (!RTMP_Connect(m_pRtmp, NULL))
          {
              LOG("RTMP_Connect failed!");
                  return FALSE;
          }
          if (!RTMP_ConnectStream(m_pRtmp, 0))
          {
              LOG("RTMP_ConnectStream failed");
                  return FALSE;
          }
          //判断是否打开音视频,默认打开
          if(_rtmp_obj_type == RTMPOBJ_TYPE_PUSH)
          {
              if(!_enableVideo)
              {
                  RTMP_SendReceiveVideo(m_pRtmp, _enableVideo);
              }
              if(!_enableAudio)
              {
                  RTMP_SendReceiveAudio(m_pRtmp, _enableAudio);
              }
          }

          return true;
        }

	bool RTMPObject::connect(std::string url)
	{
	  m_strUrl = url;
	  return connect();
	}
}
