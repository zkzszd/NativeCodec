#include "YUVUtil.hpp"

namespace WeiYu
{
  void YUVUtil::YUV420spRotate90(char *dst, const char *src, int srcWidth, int srcHeight)
  {
      static int nWidth = 0, nHeight = 0;
      static int wh = 0;
      static int uvHeight = 0;
      if(srcWidth != nWidth || srcHeight != nHeight)
      {
          nWidth = srcWidth;
          nHeight = srcHeight;
          wh = srcWidth * srcHeight;
          uvHeight = srcHeight >> 1;//uvHeight = height / 2
      }

      //旋转Y
      int k = 0;
      for(int i = 0; i < srcWidth; i++) {
          int nPos = 0;
          for(int j = 0; j < srcHeight; j++) {
              dst[k] = src[nPos + i];
              k++;
              nPos += srcWidth;
          }
      }

      for(int i = 0; i < srcWidth; i+=2){
          int nPos = wh;
          for(int j = 0; j < uvHeight; j++) {
              dst[k] = src[nPos + i];
              dst[k + 1] = src[nPos + i + 1];
              k += 2;
              nPos += srcWidth;
          }
      }
      return;
  }

  void YUVUtil::YUV420spRotateNegative90(char *dst, const char *src, int srcWidth, int height)
  {
      static int nWidth = 0, nHeight = 0;
      static int wh = 0;
      static int uvHeight = 0;
      if(srcWidth != nWidth || height != nHeight)
      {
          nWidth = srcWidth;
          nHeight = height;
          wh = srcWidth * height;
          uvHeight = height >> 1;//uvHeight = height / 2
      }

      //旋转Y
      int k = 0;
      for(int i = 0; i < srcWidth; i++){
          int nPos = srcWidth - 1;
          for(int j = 0; j < height; j++)
          {
              dst[k] = src[nPos - i];
              k++;
              nPos += srcWidth;
          }
      }

      for(int i = 0; i < srcWidth; i+=2){
          int nPos = wh + srcWidth - 1;
          for(int j = 0; j < uvHeight; j++) {
              dst[k] = src[nPos - i - 1];
              dst[k + 1] = src[nPos - i];
              k += 2;
              nPos += srcWidth;
          }
      }

      return;
  }

  bool YUVUtil::NV12ToYUV420p(YUVNV12 &nv12,YUV420p &yuv420p)
  {
    if(yuv420p.size != nv12.size)
      {
        return false;
      }
    int frame = nv12.width*nv12.height;
    //Y
    memcpy(yuv420p.Y,nv12.data,frame);
    char* src_uv = nv12.data+frame;
    for(int i = 0;i<frame/4;i++)
      {
        yuv420p.U[i] = src_uv[i*2];
        yuv420p.V[i] = src_uv[i*2+1];
      }
    return true;
  }

  bool YUVUtil::YUV420pToNV12(YUV420p &yuv420p,YUVNV12 &nv12)
  {
    if(yuv420p.size != nv12.size)
      {
        return false;
      }
    int frame = yuv420p.width*yuv420p.height;
    memcpy(nv12.data,yuv420p.data,frame);
    char* des_uv = nv12.data + frame;
    for(int i = 0;i<frame/4;i++)
    {
        des_uv[i*2] = yuv420p.U[i];
        des_uv[i*2+1] = yuv420p.V[i];
    }
    return true;
  }

  bool YUVUtil::NV21ToYUV420p(YUVNV21 &nv21,YUV420p &yuv420p)
  {
    if(nv21.size != yuv420p.size)
      {
        return false;
      }
    int frame = nv21.width*nv21.height;
    memcpy(yuv420p.Y,nv21.data,frame);
    char* src_vu =nv21.data+frame;
    for(int i = 0;i<frame/4;i++)
    {
        yuv420p.U[i] = src_vu[i*2+1];
        yuv420p.V[i] = src_vu[i*2];
    }
    return true;
  }

}
