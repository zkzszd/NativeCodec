#include "Blur.hpp"
#include <omp.h>
namespace WeiYu
{
  Blur::Blur(int widht,int height)
  {
    mIntegralMatrix = new uint32_t[widht * height];
    mIntegralMatrixSqr = new uint32_t[widht * height];
    columnSum = new uint32_t[widht];
    columnSumSqr = new uint32_t[widht];
    radius = widht > height ? widht * 0.02 : height * 0.02;
  }
  Blur::~Blur()
  {
    delete []mIntegralMatrix;
    delete []mIntegralMatrixSqr;
    delete []columnSum;
    delete []columnSumSqr;
  }

  void Blur::process(YUVStruct &yuvFrame,float sigema)
  {
    initIntegral(yuvFrame);
    smooth(yuvFrame,sigema);
  }

  void Blur::initIntegral(YUVStruct &yuvFrame)
  {
    columnSum[0] = yuvFrame.data[0];
    columnSumSqr[0] = yuvFrame.data[0] * yuvFrame.data[0];
    mIntegralMatrix[0] = columnSum[0];
    mIntegralMatrixSqr[0] = columnSumSqr[0];
    for(int i = 1;i < yuvFrame.width;i++)
    {
      columnSum[i] = yuvFrame.data[i];
      columnSumSqr[i] = yuvFrame.data[i] * yuvFrame.data[i];

      mIntegralMatrix[i] = columnSum[i];
      mIntegralMatrix[i] += mIntegralMatrix[i-1];
      mIntegralMatrixSqr[i] = columnSumSqr[i];
      mIntegralMatrixSqr[i] += mIntegralMatrixSqr[i-1];
    }
    for (int i = 1;i < yuvFrame.height; i++)
    {
      int offset = i * yuvFrame.width;

      columnSum[0] += yuvFrame.data[offset];
      columnSumSqr[0] += yuvFrame.data[offset] * yuvFrame.data[offset];

      mIntegralMatrix[offset] = columnSum[0];
      mIntegralMatrixSqr[offset] = columnSumSqr[0];
       // other columns
      for(int j = 1; j < yuvFrame.width; j++)
      {
          columnSum[j] += yuvFrame.data[offset+j];
          columnSumSqr[j] += yuvFrame.data[offset+j] * yuvFrame.data[offset+j];

          mIntegralMatrix[offset+j] = mIntegralMatrix[offset+j-1] + columnSum[j];
          mIntegralMatrixSqr[offset+j] = mIntegralMatrixSqr[offset+j-1] + columnSumSqr[j];
      }
    }
  }//initIntegral

  void Blur::smooth(YUVStruct &yuvFrame,float sigema)
  {
//#pragma omp parallel for
    for(int i = 1; i < yuvFrame.height; i++)
    {
         LOGV("id= %d",omp_get_thread_num());
           for(int j = 1; j < yuvFrame.width; j++)
             {
               int offset = i * yuvFrame.width + j;
//               if(mSkinMatrix[offset] == 255)
//               {
                   int iMax = i + radius >= yuvFrame.height-1 ? yuvFrame.height-1 : i + radius;
                   int jMax = j + radius >= yuvFrame.width-1 ? yuvFrame.width-1 :j + radius;
                   int iMin = i - radius <= 1 ? 1 : i - radius;
                   int jMin = j - radius <= 1 ? 1 : j - radius;

                   int squar = (iMax - iMin + 1)*(jMax - jMin + 1);
                   int i4 = iMax*yuvFrame.width+jMax;
                   int i3 = (iMin-1)*yuvFrame.width+(jMin-1);
                   int i2 = iMax*yuvFrame.width+(jMin-1);
                   int i1 = (iMin-1)*yuvFrame.width+jMax;

                   float m = (mIntegralMatrix[i4]
                           + mIntegralMatrix[i3]
                           - mIntegralMatrix[i2]
                           - mIntegralMatrix[i1]) / squar;

                   float v = (mIntegralMatrixSqr[i4]
                           + mIntegralMatrixSqr[i3]
                           - mIntegralMatrixSqr[i2]
                           - mIntegralMatrixSqr[i1]) / squar - m*m;
                   float k = v / (v + sigema);
                   yuvFrame.data[offset] = m - k * m + k * yuvFrame.data[offset];
//               }
           }
       }
  }


}
