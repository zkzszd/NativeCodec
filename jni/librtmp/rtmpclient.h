
#ifndef _RTMP_CLIENT_H_
#define _RTMP_CLIENT_H_
#include "rtmp.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"{
#endif

int rtmpclient_open(char *url);

int rtmpclient_close();

int rtmpclient_isConnected();

int rtmpclient_writeFlvFrame(uint8_t *flv_data, int flv_size);

int rtmpclient_isTimeout();

#ifdef __cplusplus
}
#endif
#endif
