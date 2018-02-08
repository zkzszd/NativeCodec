#ifndef _SPS_UTIL_H_
#define _SPS_UTIL_H_
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
namespace WeiYu
{

typedef  unsigned int UINT;
typedef  unsigned char BYTE;
typedef  unsigned long DWORD;



/**
 * 解码SPS,获取视频图像宽、高和帧率信息
 *
 * @param buf SPS数据内容
 * @param nLen SPS数据的长度
 * @param width 图像宽度
 * @param height 图像高度

 * @成功则返回true , 失败则返回false
 */
bool Util_decode_sps(BYTE * buf,unsigned int nLen,int &width,int &height,int &fps);

}

#endif
