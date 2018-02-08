#ifndef _BLUR_H_
#define _BLUR_H_

#include "../base/MyDef.hpp"
#include <android/log.h>
#define TAG "Blur"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

namespace WeiYu
{
  //算法来源：http://blog.csdn.net/oshunz/article/details/50372968
  class Blur
  {
  public:
    Blur(int widht,int height);
    ~Blur();
  public:
    void process(YUVStruct &yuvFrame,float sigema);
  private:
    void initIntegral(YUVStruct &yuvFrame);
    void smooth(YUVStruct &yuvFrame,float sigema);
  private:
    uint32_t *mIntegralMatrix;
    uint32_t *mIntegralMatrixSqr;
    uint32_t *columnSum;
    uint32_t *columnSumSqr;
    uint32_t radius;
  };

}


#endif
