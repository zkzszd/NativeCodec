#pragma once
#include "../base/MyDef.hpp"

namespace WeiYu
{

  enum EdgeMode                                                                                         //      某些领域算法的边界处理方式
  {
          Edge_Mode_Repeat = 1,                                                                           //      重复边缘像素
          Edge_Mode_Smear                                                                                         //镜像边缘像素
  };
  struct RowHistogram                           //行统计直方图
  {
      uint32 width;
      int32 *data;
      ~RowHistogram()
      {
          if (data != nullptr)
          {
                  free(data);
          }
      }
  };

  struct ColHistogram
  {
      uint32 heigth;                          //列高
      int32 *data;
      ~ColHistogram()
      {
          if (data != nullptr)
          {
                  free(data);
          }
      }
  };
  /*ѡ����ģ�� */
  class SelectiveBlur
  {
  public:
    //处理YUV三通道
    static int32 selectBlurYUV420p(YUV420p &src420p,YUV420p &des420p,int32 Radius,int32 Threshold, EdgeMode Edge);
    //只处理YUV的Y通道
    static int32 selectBlurYUV420p_Y(YUV420p &src420p,int32 Radius,int32 Threshold, EdgeMode Edge);
  private:
    static RET_CODE ValidCoordinateCol(ColHistogram& colHistogram, int32 heigth, int32 top, int32 bottom, EdgeMode edgeMode);
    static RET_CODE ValidCoordinateRow(RowHistogram& rowHistogram,int32 width, int32 left, int32 right, EdgeMode edgeMode);
    SelectiveBlur();
  };
}

