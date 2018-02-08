#ifndef _YUV_UTIL_H_
#define _YUV_UTIL_H_
#include "../base/MyDef.hpp"
#include <string.h>
namespace WeiYu
{
  class YUVUtil
  {
  public:
    //顺时针
    static void YUV420spRotate90(char *dst, const char *src, int srcWidth, int srcHeight);
    //逆时针
    static void YUV420spRotateNegative90(char *dst, const char *src, int srcWidth, int height);

    static bool NV21ToYUV420p(YUVNV21 &nv21,YUV420p &yuv420p);

    static bool NV12ToYUV420p(YUVNV12 &nv12,YUV420p &yuv420p);

    static bool YUV420pToNV12(YUV420p &yuv420p,YUVNV12 &nv12);
  };
}

#endif
