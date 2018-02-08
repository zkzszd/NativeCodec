#pragma once   
#include "rtmp.h"   
#include "rtmp_sys.h"   
#include "amf.h"   
#include <stdio.h>   
#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#endif // _WIN32

#include <android/log.h>
#define TAG "CRTMPStream"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

#define FILEBUFSIZE (1024 * 1024 * 10)       //  10M   

// NALU��Ԫ   
typedef struct _NaluUnit
{
	int type;
	int size;
	unsigned char *data;
}NaluUnit;

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
	unsigned int    nAudioSampleRate;
	unsigned int    nAudioSampleSize;
	unsigned int    nAudioChannels;
	char            pAudioSpecCfg;
	unsigned int    nAudioSpecCfgLen;

} RTMPMetadata, *LPRTMPMetadata;

//����H264��Ƶ��RTMP Server��ʹ��libRtmp��
class CRTMPStream
{
public:
	CRTMPStream(void);
	~CRTMPStream(void);
public:
	// ���ӵ�RTMP Server   
	bool Connect(const char* url);
	// �Ͽ�����   
	void Close();

	bool sendHead(int width, int height, int iRateBit = 96, int iFps = 25);

	// ����MetaData   
	bool SendMetadata(LPRTMPMetadata lpMetaData);
	// ����H264����֡   
	bool SendH264Packet(unsigned char *data,int size, bool bIsKeyFrame, unsigned int nTimeStamp);
	// ����H264�ļ�   
	bool SendH264File(const char *pFileName);
	//����һ��nalu
	bool sendOneNalu(unsigned char *data, int length, int type);
private:
	// �ͻ����ж�ȡһ��NALU��   
	bool ReadOneNaluFromBuf(NaluUnit &nalu);
	// ��������   
	int SendPacket(unsigned int nPacketType, unsigned char *data, unsigned int size, unsigned int nTimestamp);
private:
	unsigned int time;
	RTMP* m_pRtmp;
	unsigned char* m_pFileBuf;
	unsigned int  m_nFileBufSize;
	unsigned int  m_nCurPos;
};
