#include "CRTMPStream.hpp"
#include "SpsUtil.h"
#ifdef WIN32     
#include <windows.h>   
#endif   
#include <stdio.h>
#ifdef WIN32   
#pragma comment(lib,"WS2_32.lib")   
#pragma comment(lib,"winmm.lib")   
#endif   

enum
{
	FLV_CODECID_H264 = 7,
};

int InitSockets()
{
#ifdef WIN32     
	WORD version;
	WSADATA wsaData;
	version = MAKEWORD(1, 1);
	return (WSAStartup(version, &wsaData) == 0);
#else     
	return TRUE;
#endif     
}

inline void CleanupSockets()
{
#ifdef WIN32     
	WSACleanup();
#endif     
}

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

CRTMPStream::CRTMPStream(void) :
	m_pRtmp(NULL),
	m_nFileBufSize(0),
	m_nCurPos(0)
{
	m_pFileBuf = new unsigned char[FILEBUFSIZE];
	memset(m_pFileBuf, 0, FILEBUFSIZE);
	InitSockets();
	m_pRtmp = RTMP_Alloc();
	RTMP_Init(m_pRtmp);
	time = RTMP_GetTime();
}

CRTMPStream::~CRTMPStream(void)
{
	Close();
	delete[] m_pFileBuf;
}

bool CRTMPStream::Connect(const char* url)
{
	if (RTMP_SetupURL(m_pRtmp, (char*)url)<0)
	{
		return FALSE;
	}
	RTMP_EnableWrite(m_pRtmp);
	if (RTMP_Connect(m_pRtmp, NULL)<0)
	{
		return FALSE;
	}
	if (RTMP_ConnectStream(m_pRtmp, 0)<0)
	{
		return FALSE;
	}
	return TRUE;
}

void CRTMPStream::Close()
{
	if (m_pRtmp)
	{
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		m_pRtmp = NULL;
	}
}

int CRTMPStream::SendPacket(unsigned int nPacketType, unsigned char *data, unsigned int size, unsigned int nTimestamp)
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
	if (nRet != 0)
	{
		printf("RTMP_SendPacket fail %d\n",nRet);
	}

	RTMPPacket_Free(&packet);

	return nRet;
}

bool CRTMPStream::sendHead(int width,int height, int iRateBit, int iFps)
{
	unsigned char szBodyBuffer[1024 * 32];
	RTMPPacket packet = { 0 };
	memset(&packet, 0, sizeof(RTMPPacket));
	packet.m_nChannel = 0x04;
	packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet.m_nTimeStamp = 0;
	packet.m_nInfoField2 = m_pRtmp->m_stream_id;
	packet.m_hasAbsTimestamp = 0;
	packet.m_body = (char *)szBodyBuffer;
	char * szTmp = (char *)szBodyBuffer;
	packet.m_packetType = RTMP_PACKET_TYPE_INFO;
	szTmp = put_byte(szTmp, AMF_STRING);
	szTmp = put_amf_string(szTmp, "@setDataFrame");
	szTmp = put_byte(szTmp, AMF_STRING);
	szTmp = put_amf_string(szTmp, "onMetaData");
	szTmp = put_byte(szTmp, AMF_OBJECT);
	szTmp = put_amf_string(szTmp, "author");
	szTmp = put_byte(szTmp, AMF_STRING);
	szTmp = put_amf_string(szTmp, "");
	szTmp = put_amf_string(szTmp, "copyright");
	szTmp = put_byte(szTmp, AMF_STRING);
	szTmp = put_amf_string(szTmp, "");
	szTmp = put_amf_string(szTmp, "description");
	szTmp = put_byte(szTmp, AMF_STRING);
	szTmp = put_amf_string(szTmp, "");
	szTmp = put_amf_string(szTmp, "keywords");
	szTmp = put_byte(szTmp, AMF_STRING);
	szTmp = put_amf_string(szTmp, "");
	szTmp = put_amf_string(szTmp, "rating");
	szTmp = put_byte(szTmp, AMF_STRING);
	szTmp = put_amf_string(szTmp, "");
	szTmp = put_amf_string(szTmp, "presetname");
	szTmp = put_byte(szTmp, AMF_STRING);
	szTmp = put_amf_string(szTmp, "Custom");
	szTmp = put_amf_string(szTmp, "width");
	szTmp = put_amf_double(szTmp, width);
	szTmp = put_amf_string(szTmp, "height");
	szTmp = put_amf_double(szTmp, height);
	szTmp = put_amf_string(szTmp, "framerate");
	szTmp = put_amf_double(szTmp,
		(double)iFps / 1);
	szTmp = put_amf_string(szTmp, "videocodecid");
	szTmp = put_byte(szTmp, AMF_STRING);
	szTmp = put_amf_string(szTmp, "avc1");
	szTmp = put_amf_string(szTmp, "videodatarate");
	szTmp = put_amf_double(szTmp, iRateBit);
	//szTmp = put_amf_string(szTmp, "avclevel");
	//szTmp = put_amf_double(szTmp, p264Param->i_level_idc);	//ָ�����õ�levelֵ����������븴�Ӷ��й�
	szTmp = put_amf_string(szTmp, "avcprofile");
	szTmp = put_amf_double(szTmp, 0x42);
	szTmp = put_amf_string(szTmp, "videokeyframe_frequency");
	szTmp = put_amf_double(szTmp, 3);
	szTmp = put_amf_string(szTmp, "");
	szTmp = put_byte(szTmp, AMF_OBJECT_END);
	packet.m_nBodySize = szTmp - (char *)szBodyBuffer;
	return RTMP_SendPacket(m_pRtmp, &packet, 1);
}

bool CRTMPStream::SendMetadata(LPRTMPMetadata lpMetaData)
{
	//int outChunkSize = 32;
	////�޸ķ��ͷְ��Ĵ�С  Ĭ��128�ֽ�
	//RTMPPacket pack;
	//RTMPPacket_Alloc(&pack, 4);
	//pack.m_packetType = RTMP_PACKET_TYPE_CHUNK_SIZE;
	//pack.m_nChannel = 0x02;
	//pack.m_headerType = RTMP_PACKET_SIZE_LARGE;
	//pack.m_nTimeStamp = 0;
	//pack.m_nInfoField2 = 0;
	//pack.m_nBodySize = 4;
	//pack.m_body[3] = outChunkSize & 0xff; //���ֽ���
	//pack.m_body[2] = outChunkSize >> 8;
	//pack.m_body[1] = outChunkSize >> 16;
	//pack.m_body[0] = outChunkSize >> 24;
	//m_pRtmp->m_inChunkSize = outChunkSize;
	//m_pRtmp->m_outChunkSize = outChunkSize;
	//RTMP_SendPacket(m_pRtmp, &pack, 1);
	//RTMPPacket_Free(&pack);


	if (lpMetaData == NULL)
	{
		return false;
	}
	char body[1024] = { 0 };;

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

	int index = p - body;

	SendPacket(RTMP_PACKET_TYPE_INFO, (unsigned char*)body, p - body, 0);

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

	return SendPacket(RTMP_PACKET_TYPE_VIDEO, (unsigned char*)body, i, 0);

}

bool CRTMPStream::SendH264Packet(unsigned char *data,int size, bool bIsKeyFrame, unsigned int nTimeStamp)
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

bool CRTMPStream::SendH264File(const char *pFileName)
{
	if (pFileName == NULL)
	{
		return FALSE;
	}
	FILE *fp = fopen(pFileName, "rb");
	if (!fp)
	{
		printf("ERROR:open file %s failed!", pFileName);
	}
	fseek(fp, 0, SEEK_SET);
	m_nFileBufSize = fread(m_pFileBuf, sizeof(unsigned char), FILEBUFSIZE, fp);
	if (m_nFileBufSize >= FILEBUFSIZE)
	{
		printf("warning : File size is larger than BUFSIZE\n");
	}
	fclose(fp);

	RTMPMetadata metaData;
	memset(&metaData, 0, sizeof(RTMPMetadata));

	NaluUnit naluUnit;
	// ��ȡSPS֡   
	ReadOneNaluFromBuf(naluUnit);
	metaData.nSpsLen = naluUnit.size;
	memcpy(metaData.Sps, naluUnit.data, naluUnit.size);

	// ��ȡPPS֡   
	ReadOneNaluFromBuf(naluUnit);
	metaData.nPpsLen = naluUnit.size;
	memcpy(metaData.Pps, naluUnit.data, naluUnit.size);

	// ����SPS,��ȡ��Ƶͼ�������Ϣ   
	int width = 0, height = 0,frate = 0;
	WeiYu::Util_decode_sps(metaData.Sps, metaData.nSpsLen, width, height,frate);
	metaData.nWidth = width;
	metaData.nHeight = height;
	metaData.nFrameRate = 25;

	//sendHead(width, height);



	// ����MetaData   
	LOGV("ret MetaData code %d ",SendMetadata(&metaData));

	unsigned int tick = 0;
	while (ReadOneNaluFromBuf(naluUnit))
	{
		bool bKeyframe = (naluUnit.type == 0x05) ? TRUE : FALSE;
		// ����H264����֡   
		LOGV("264pace code %d",SendH264Packet(naluUnit.data, naluUnit.size, bKeyframe, tick));
		msleep(40);
		tick += 40;
	}

	return TRUE;
}

bool CRTMPStream::sendOneNalu(unsigned char *data, int length, int type)
{
	bool bKeyframe = (type == 0x05) ? TRUE : FALSE;

	return SendH264Packet(data, length, bKeyframe, RTMP_GetTime()-time);
}


bool CRTMPStream::ReadOneNaluFromBuf(NaluUnit &nalu)
{
	int i = m_nCurPos;
	while (i<m_nFileBufSize)
	{
		if (m_pFileBuf[i++] == 0x00 &&
			m_pFileBuf[i++] == 0x00 &&
			m_pFileBuf[i++] == 0x00 &&
			m_pFileBuf[i++] == 0x01)
		{
			int pos = i;
			while (pos<m_nFileBufSize)
			{
				if (m_pFileBuf[pos++] == 0x00 &&
					m_pFileBuf[pos++] == 0x00 &&
					m_pFileBuf[pos++] == 0x00 &&
					m_pFileBuf[pos++] == 0x01
					)
				{
					break;
				}
			}
			//if (pos == nBufferSize)
			//{
			//	nalu.size = pos - i;
			//}
			//else
			//{
			//	nalu.size = (pos - 4) - i;
			//}
			nalu.size = (pos - 4) - i;
			nalu.type = m_pFileBuf[i] & 0x1f;
			nalu.data = &m_pFileBuf[i];

			m_nCurPos = pos - 4;
			return TRUE;
		}
	}
	return FALSE;
}
