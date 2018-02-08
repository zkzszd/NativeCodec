#include "rtmpclient.h"

#define  LOG_TAG    "RTMP_CLIENT"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

RTMP *pRtmp;

int rtmpclient_open(char *url) {
    pRtmp = RTMP_Alloc();
    RTMP_Init(pRtmp);
    pRtmp->Link.timeout = 1;
    pRtmp->Link.lFlags |= RTMP_LF_LIVE;
    if (!RTMP_SetupURL(pRtmp, url)) {
        RTMP_Free(pRtmp);
        return -1;
    }
    RTMP_EnableWrite(pRtmp);
    if (!RTMP_Connect(pRtmp, NULL)) {
        RTMP_Free(pRtmp);
        return -1;
    }
    if (!RTMP_ConnectStream(pRtmp, 0)) {
        RTMP_Close(pRtmp);
        RTMP_Free(pRtmp);
        return -1;
    }
    return 1;
}

int rtmpclient_close() {
    if (pRtmp) {
        RTMP_Close(pRtmp);
        RTMP_Free(pRtmp);
        pRtmp = NULL;
        return 1;
    }
    return -1;
}

int rtmpclient_isConnected() {
    if (pRtmp && RTMP_IsConnected(pRtmp)) {
        return 1;
    }
    return -1;
}

int rtmpclient_writeFlvFrame(uint8_t *flv_data, int flv_size) {
    if (rtmpclient_isConnected()) {
        return RTMP_Write(pRtmp, flv_data, flv_size);
    }
    return -2;
}

int rtmpclient_isTimeout() {
    return -1;
}

