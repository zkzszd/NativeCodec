#ifndef _MEDIACODEC_UTIL_H_
#define _MEDIACODEC_UTIL_H_

#include <assert.h>
#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

namespace WeiYu
{

int64_t systemnanotime();
//从buf中获取一个nalu结束位置，如果没找到，返回length-1，如果buf开始不是00000001返回0
int readOneNaluFromBuf(char* data,int length);
}
#endif
