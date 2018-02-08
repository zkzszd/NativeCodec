//
//  MyTypedef.h
//  
//
//  Created by zkzszd on 2017/4/25.
//
//

#ifndef MyTypedef_h
#define MyTypedef_h
#include <stdlib.h>
#include <string.h>
#include "../looper/NaluLoop.hpp"
#include "../librtmp/rtmp.h"

namespace WeiYu {
    typedef unsigned char       uint8;
    typedef char                int8;
    typedef unsigned short      uint16;
    typedef short               int16;
    typedef unsigned int        uint32;
    typedef int                 int32;
    typedef unsigned long long  uint64;
    typedef long long           int64;

#define ABS(a) ((a)<(0)?(-a):(a))
#define MAX(a, b) ((a)>(b)?(a):(b))
#define MIN(a, b) ((a)<(b)?(a):(b))
	
#define CODE_FAIL -1
#define CODE_SUCESS 0
#define CODE_FILE_OPEN_FILE 1

      enum RET_CODE
      {
              RET_ERR_UNKNOWN = -2,
              RET_FAIL = -1,							//ʧ��
              RET_OK	= 0,							//�ɹ�
              RET_ERR_OPEN_FILE,						//���ļ�ʧ�ܣ������ļ�������
              RET_ERR_NOT_SUPPORT,					//��֧�ֵĲ���
              RET_ERR_OUTOFMEMORY,					//	�ڴ����
              RET_ERR_STACKOVERFLOW,					//	��ջ���
              RET_ERR_NULLREFERENCE,					//	������
              RET_ERR_ARGUMENTOUTOFRANGE,				//	��������������Χ��
              RET_ERR_PARAMISMATCH,					//	������ƥ�� ��������
      };

      class YUVStruct :public LoopMsgObj
      {
      public:
        int32 size = 0;
        int32 width = 0;
        int32 height = 0;
        char *data = NULL;
        YUVStruct(int32 size,int32 width,int height);
        YUVStruct(char*data,int32 size,int32 width,int height);
        virtual ~YUVStruct();
      };
      //NV12: YYYYYYYY UVUV
      typedef YUVStruct YUVNV12;
      //NV21: YYYYYYYY VUVU
      typedef YUVStruct YUVNV21;

      class YUV420p :public YUVStruct
      {
      public:
        char* Y;
        char* U;
        char* V;

        YUV420p(int32 size,int32 width,int height);
        YUV420p(char*data,int32 size,int32 width,int height);
        virtual ~YUV420p();
      };

      class NaluStruct :public LoopMsgObj
      {
      public:
        NaluStruct(int size);
        NaluStruct(const unsigned char*buf,int bufLen);
        virtual ~NaluStruct();
        int type;
        int size;
        unsigned char *data = NULL;
      };

      struct MsgRTMPPPack : LoopMsgObj
      {
        RTMPPacket *rtmpPack = NULL;
        MsgRTMPPPack(RTMPPacket& pack)
        {
          rtmpPack = (RTMPPacket *)malloc(sizeof(RTMPPacket));
          memcpy(rtmpPack,&pack,sizeof(RTMPPacket));
        }
        virtual ~MsgRTMPPPack()
        {
          if(rtmpPack)
          {
              RTMPPacket_Free(rtmpPack);
              rtmpPack = NULL;
          }
        }
      };
}

#endif /* MyTypedef_h */
