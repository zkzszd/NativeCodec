#include "SelectiveBlur.hpp"
#include <iostream>
#include <arm_neon.h>
#include <omp.h>

using namespace std;

namespace WeiYu
{

  /// <summary>
  /// 无符号短整形直方图数据相加，Y = X + Y， 整理时间2014.12.28;
  /// </summary>
  /// <param name="X">加数。</param>
  /// <param name="Y">被加数，结果保存于该数中。</param>
  /// <remarks>使用了SSE优化。</remarks>
  void HistgramAddShort(unsigned short *X, unsigned short *Y)
  {
          *(uint16x8_t*)(Y + 0) = vaddq_u16(*(uint16x8_t*)&Y[0], *(uint16x8_t*)&X[0]);         //      不要想着用自己写的汇编超过他的速度了，已经试过了
          *(uint16x8_t*)(Y + 8) = vaddq_u16(*(uint16x8_t*)&Y[8], *(uint16x8_t*)&X[8]);
          *(uint16x8_t*)(Y + 16) = vaddq_u16(*(uint16x8_t*)&Y[16], *(uint16x8_t*)&X[16]);
          *(uint16x8_t*)(Y + 24) = vaddq_u16(*(uint16x8_t*)&Y[24], *(uint16x8_t*)&X[24]);
          *(uint16x8_t*)(Y + 32) = vaddq_u16(*(uint16x8_t*)&Y[32], *(uint16x8_t*)&X[32]);
          *(uint16x8_t*)(Y + 40) = vaddq_u16(*(uint16x8_t*)&Y[40], *(uint16x8_t*)&X[40]);
          *(uint16x8_t*)(Y + 48) = vaddq_u16(*(uint16x8_t*)&Y[48], *(uint16x8_t*)&X[48]);
          *(uint16x8_t*)(Y + 56) = vaddq_u16(*(uint16x8_t*)&Y[56], *(uint16x8_t*)&X[56]);
          *(uint16x8_t*)(Y + 64) = vaddq_u16(*(uint16x8_t*)&Y[64], *(uint16x8_t*)&X[64]);
          *(uint16x8_t*)(Y + 72) = vaddq_u16(*(uint16x8_t*)&Y[72], *(uint16x8_t*)&X[72]);
          *(uint16x8_t*)(Y + 80) = vaddq_u16(*(uint16x8_t*)&Y[80], *(uint16x8_t*)&X[80]);
          *(uint16x8_t*)(Y + 88) = vaddq_u16(*(uint16x8_t*)&Y[88], *(uint16x8_t*)&X[88]);
          *(uint16x8_t*)(Y + 96) = vaddq_u16(*(uint16x8_t*)&Y[96], *(uint16x8_t*)&X[96]);
          *(uint16x8_t*)(Y + 104) = vaddq_u16(*(uint16x8_t*)&Y[104], *(uint16x8_t*)&X[104]);
          *(uint16x8_t*)(Y + 112) = vaddq_u16(*(uint16x8_t*)&Y[112], *(uint16x8_t*)&X[112]);
          *(uint16x8_t*)(Y + 120) = vaddq_u16(*(uint16x8_t*)&Y[120], *(uint16x8_t*)&X[120]);
          *(uint16x8_t*)(Y + 128) = vaddq_u16(*(uint16x8_t*)&Y[128], *(uint16x8_t*)&X[128]);
          *(uint16x8_t*)(Y + 136) = vaddq_u16(*(uint16x8_t*)&Y[136], *(uint16x8_t*)&X[136]);
          *(uint16x8_t*)(Y + 144) = vaddq_u16(*(uint16x8_t*)&Y[144], *(uint16x8_t*)&X[144]);
          *(uint16x8_t*)(Y + 152) = vaddq_u16(*(uint16x8_t*)&Y[152], *(uint16x8_t*)&X[152]);
          *(uint16x8_t*)(Y + 160) = vaddq_u16(*(uint16x8_t*)&Y[160], *(uint16x8_t*)&X[160]);
          *(uint16x8_t*)(Y + 168) = vaddq_u16(*(uint16x8_t*)&Y[168], *(uint16x8_t*)&X[168]);
          *(uint16x8_t*)(Y + 176) = vaddq_u16(*(uint16x8_t*)&Y[176], *(uint16x8_t*)&X[176]);
          *(uint16x8_t*)(Y + 184) = vaddq_u16(*(uint16x8_t*)&Y[184], *(uint16x8_t*)&X[184]);
          *(uint16x8_t*)(Y + 192) = vaddq_u16(*(uint16x8_t*)&Y[192], *(uint16x8_t*)&X[192]);
          *(uint16x8_t*)(Y + 200) = vaddq_u16(*(uint16x8_t*)&Y[200], *(uint16x8_t*)&X[200]);
          *(uint16x8_t*)(Y + 208) = vaddq_u16(*(uint16x8_t*)&Y[208], *(uint16x8_t*)&X[208]);
          *(uint16x8_t*)(Y + 216) = vaddq_u16(*(uint16x8_t*)&Y[216], *(uint16x8_t*)&X[216]);
          *(uint16x8_t*)(Y + 224) = vaddq_u16(*(uint16x8_t*)&Y[224], *(uint16x8_t*)&X[224]);
          *(uint16x8_t*)(Y + 232) = vaddq_u16(*(uint16x8_t*)&Y[232], *(uint16x8_t*)&X[232]);
          *(uint16x8_t*)(Y + 240) = vaddq_u16(*(uint16x8_t*)&Y[240], *(uint16x8_t*)&X[240]);
          *(uint16x8_t*)(Y + 248) = vaddq_u16(*(uint16x8_t*)&Y[248], *(uint16x8_t*)&X[248]);
  }

  /// <summary>
  /// 无符号短整形直方图数据相减，Y = Y + X， 整理时间2014.12.28;
  /// </summary>
  /// <param name="X">减数。</param>
  /// <param name="Y">被减数，结果保存于该数中。</param>
  /// <remarks>使用了SSE优化。</remarks>
  void HistgramSubShort(unsigned short *X, unsigned short *Y)
  {
          *(uint16x8_t*)(Y + 0) = vsubq_u16(*(uint16x8_t*)&Y[0], *(uint16x8_t*)&X[0]);
          *(uint16x8_t*)(Y + 8) = vsubq_u16(*(uint16x8_t*)&Y[8], *(uint16x8_t*)&X[8]);
          *(uint16x8_t*)(Y + 16) = vsubq_u16(*(uint16x8_t*)&Y[16], *(uint16x8_t*)&X[16]);
          *(uint16x8_t*)(Y + 24) = vsubq_u16(*(uint16x8_t*)&Y[24], *(uint16x8_t*)&X[24]);
          *(uint16x8_t*)(Y + 32) = vsubq_u16(*(uint16x8_t*)&Y[32], *(uint16x8_t*)&X[32]);
          *(uint16x8_t*)(Y + 40) = vsubq_u16(*(uint16x8_t*)&Y[40], *(uint16x8_t*)&X[40]);
          *(uint16x8_t*)(Y + 48) = vsubq_u16(*(uint16x8_t*)&Y[48], *(uint16x8_t*)&X[48]);
          *(uint16x8_t*)(Y + 56) = vsubq_u16(*(uint16x8_t*)&Y[56], *(uint16x8_t*)&X[56]);
          *(uint16x8_t*)(Y + 64) = vsubq_u16(*(uint16x8_t*)&Y[64], *(uint16x8_t*)&X[64]);
          *(uint16x8_t*)(Y + 72) = vsubq_u16(*(uint16x8_t*)&Y[72], *(uint16x8_t*)&X[72]);
          *(uint16x8_t*)(Y + 80) = vsubq_u16(*(uint16x8_t*)&Y[80], *(uint16x8_t*)&X[80]);
          *(uint16x8_t*)(Y + 88) = vsubq_u16(*(uint16x8_t*)&Y[88], *(uint16x8_t*)&X[88]);
          *(uint16x8_t*)(Y + 96) = vsubq_u16(*(uint16x8_t*)&Y[96], *(uint16x8_t*)&X[96]);
          *(uint16x8_t*)(Y + 104) = vsubq_u16(*(uint16x8_t*)&Y[104], *(uint16x8_t*)&X[104]);
          *(uint16x8_t*)(Y + 112) = vsubq_u16(*(uint16x8_t*)&Y[112], *(uint16x8_t*)&X[112]);
          *(uint16x8_t*)(Y + 120) = vsubq_u16(*(uint16x8_t*)&Y[120], *(uint16x8_t*)&X[120]);
          *(uint16x8_t*)(Y + 128) = vsubq_u16(*(uint16x8_t*)&Y[128], *(uint16x8_t*)&X[128]);
          *(uint16x8_t*)(Y + 136) = vsubq_u16(*(uint16x8_t*)&Y[136], *(uint16x8_t*)&X[136]);
          *(uint16x8_t*)(Y + 144) = vsubq_u16(*(uint16x8_t*)&Y[144], *(uint16x8_t*)&X[144]);
          *(uint16x8_t*)(Y + 152) = vsubq_u16(*(uint16x8_t*)&Y[152], *(uint16x8_t*)&X[152]);
          *(uint16x8_t*)(Y + 160) = vsubq_u16(*(uint16x8_t*)&Y[160], *(uint16x8_t*)&X[160]);
          *(uint16x8_t*)(Y + 168) = vsubq_u16(*(uint16x8_t*)&Y[168], *(uint16x8_t*)&X[168]);
          *(uint16x8_t*)(Y + 176) = vsubq_u16(*(uint16x8_t*)&Y[176], *(uint16x8_t*)&X[176]);
          *(uint16x8_t*)(Y + 184) = vsubq_u16(*(uint16x8_t*)&Y[184], *(uint16x8_t*)&X[184]);
          *(uint16x8_t*)(Y + 192) = vsubq_u16(*(uint16x8_t*)&Y[192], *(uint16x8_t*)&X[192]);
          *(uint16x8_t*)(Y + 200) = vsubq_u16(*(uint16x8_t*)&Y[200], *(uint16x8_t*)&X[200]);
          *(uint16x8_t*)(Y + 208) = vsubq_u16(*(uint16x8_t*)&Y[208], *(uint16x8_t*)&X[208]);
          *(uint16x8_t*)(Y + 216) = vsubq_u16(*(uint16x8_t*)&Y[216], *(uint16x8_t*)&X[216]);
          *(uint16x8_t*)(Y + 224) = vsubq_u16(*(uint16x8_t*)&Y[224], *(uint16x8_t*)&X[224]);
          *(uint16x8_t*)(Y + 232) = vsubq_u16(*(uint16x8_t*)&Y[232], *(uint16x8_t*)&X[232]);
          *(uint16x8_t*)(Y + 240) = vsubq_u16(*(uint16x8_t*)&Y[240], *(uint16x8_t*)&X[240]);
          *(uint16x8_t*)(Y + 248) = vsubq_u16(*(uint16x8_t*)&Y[248], *(uint16x8_t*)&X[248]);
  }

  /// <summary>
  /// 无符号短整形直方图数据相加减，Z = Z + Y - X， 整理时间2014.12.28;
  /// </summary>
  /// <param name="X">减数。</param>
  /// <param name="Y">加数。</param>
  /// <param name="Y">被加数，结果保存于该数中。</param>
  /// <remarks>使用了SSE优化，这样比分两次加 和 减 速度要快。</remarks>
  void HistgramSubAddShort(unsigned short *X, unsigned short *Y, unsigned short *Z)
  {
          *(uint16x8_t*)(Z + 0) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[0], *(uint16x8_t*)&Z[0]), *(uint16x8_t*)&X[0]);                                                //      不要想着用自己写的汇编超过他的速度了，已经试过了
          *(uint16x8_t*)(Z + 8) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[8], *(uint16x8_t*)&Z[8]), *(uint16x8_t*)&X[8]);
          *(uint16x8_t*)(Z + 16) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[16], *(uint16x8_t*)&Z[16]), *(uint16x8_t*)&X[16]);
          *(uint16x8_t*)(Z + 24) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[24], *(uint16x8_t*)&Z[24]), *(uint16x8_t*)&X[24]);
          *(uint16x8_t*)(Z + 32) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[32], *(uint16x8_t*)&Z[32]), *(uint16x8_t*)&X[32]);
          *(uint16x8_t*)(Z + 40) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[40], *(uint16x8_t*)&Z[40]), *(uint16x8_t*)&X[40]);
          *(uint16x8_t*)(Z + 48) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[48], *(uint16x8_t*)&Z[48]), *(uint16x8_t*)&X[48]);
          *(uint16x8_t*)(Z + 56) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[56], *(uint16x8_t*)&Z[56]), *(uint16x8_t*)&X[56]);
          *(uint16x8_t*)(Z + 64) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[64], *(uint16x8_t*)&Z[64]), *(uint16x8_t*)&X[64]);
          *(uint16x8_t*)(Z + 72) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[72], *(uint16x8_t*)&Z[72]), *(uint16x8_t*)&X[72]);
          *(uint16x8_t*)(Z + 80) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[80], *(uint16x8_t*)&Z[80]), *(uint16x8_t*)&X[80]);
          *(uint16x8_t*)(Z + 88) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[88], *(uint16x8_t*)&Z[88]), *(uint16x8_t*)&X[88]);
          *(uint16x8_t*)(Z + 96) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[96], *(uint16x8_t*)&Z[96]), *(uint16x8_t*)&X[96]);
          *(uint16x8_t*)(Z + 104) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[104], *(uint16x8_t*)&Z[104]), *(uint16x8_t*)&X[104]);
          *(uint16x8_t*)(Z + 112) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[112], *(uint16x8_t*)&Z[112]), *(uint16x8_t*)&X[112]);
          *(uint16x8_t*)(Z + 120) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[120], *(uint16x8_t*)&Z[120]), *(uint16x8_t*)&X[120]);
          *(uint16x8_t*)(Z + 128) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[128], *(uint16x8_t*)&Z[128]), *(uint16x8_t*)&X[128]);
          *(uint16x8_t*)(Z + 136) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[136], *(uint16x8_t*)&Z[136]), *(uint16x8_t*)&X[136]);
          *(uint16x8_t*)(Z + 144) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[144], *(uint16x8_t*)&Z[144]), *(uint16x8_t*)&X[144]);
          *(uint16x8_t*)(Z + 152) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[152], *(uint16x8_t*)&Z[152]), *(uint16x8_t*)&X[152]);
          *(uint16x8_t*)(Z + 160) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[160], *(uint16x8_t*)&Z[160]), *(uint16x8_t*)&X[160]);
          *(uint16x8_t*)(Z + 168) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[168], *(uint16x8_t*)&Z[168]), *(uint16x8_t*)&X[168]);
          *(uint16x8_t*)(Z + 176) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[176], *(uint16x8_t*)&Z[176]), *(uint16x8_t*)&X[176]);
          *(uint16x8_t*)(Z + 184) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[184], *(uint16x8_t*)&Z[184]), *(uint16x8_t*)&X[184]);
          *(uint16x8_t*)(Z + 192) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[192], *(uint16x8_t*)&Z[192]), *(uint16x8_t*)&X[192]);
          *(uint16x8_t*)(Z + 200) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[200], *(uint16x8_t*)&Z[200]), *(uint16x8_t*)&X[200]);
          *(uint16x8_t*)(Z + 208) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[208], *(uint16x8_t*)&Z[208]), *(uint16x8_t*)&X[208]);
          *(uint16x8_t*)(Z + 216) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[216], *(uint16x8_t*)&Z[216]), *(uint16x8_t*)&X[216]);
          *(uint16x8_t*)(Z + 224) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[224], *(uint16x8_t*)&Z[224]), *(uint16x8_t*)&X[224]);
          *(uint16x8_t*)(Z + 232) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[232], *(uint16x8_t*)&Z[232]), *(uint16x8_t*)&X[232]);
          *(uint16x8_t*)(Z + 240) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[240], *(uint16x8_t*)&Z[240]), *(uint16x8_t*)&X[240]);
          *(uint16x8_t*)(Z + 248) = vsubq_u16(vaddq_u16(*(uint16x8_t*)&Y[248], *(uint16x8_t*)&Z[248]), *(uint16x8_t*)&X[248]);
  }

  void Calc(unsigned short *Hist, int Intensity, unsigned char *&Pixel, int Threshold)
  {
          int K, Low, High, Sum = 0, Weight = 0;
          Low = Intensity - Threshold; High = Intensity + Threshold;
          if (Low < 0) Low = 0;
          if (High > 255) High = 255;
          for (K = Low; K <= High; K++)
          {
                  Weight += Hist[K];
                  Sum += Hist[K] * K;
          }
          if (Weight != 0) *Pixel = Sum / Weight;
  }

  RET_CODE selectBlurOneChannel(uint8* src_data, uint8* des_data, int32 width, int32 height, RowHistogram& rowHistogram,
          ColHistogram& colHistogram, int32 Radius, int32 Threshold)
  {
          uint16 *ColHist = (uint16 *)malloc(256 * (width + 2 * Radius) * sizeof(uint16));
          if (ColHist == NULL)
          {
                  return RET_ERR_OUTOFMEMORY;
          }
          memset(ColHist, 0, 256 * (width + 2 * Radius) * sizeof(uint16));

          uint16 *Hist = (uint16 *)malloc(256 * sizeof(uint16));
          if (Hist == NULL) { return RET_ERR_OUTOFMEMORY; }

          ColHist += Radius * 256;                //      ����ƫ�� ���ͷſռ��ʱ��ע��ָ�ƫ�ƣ�

          int32 *RowOffset, *ColOffSet;
          ColOffSet = (int32 *)(colHistogram.data) + Radius;
          RowOffset = (int32 *)(rowHistogram.data) + Radius;

          unsigned char *LinePS, *LinePD;

          for (int32 Y = 0; Y < height; Y++)
          {
              if (Y == 0)                                                                                     //      ��һ�е���ֱ��ͼ,Ҫ��ͷ����
              {
                  for (int32 K = -Radius; K <= Radius; K++)
                  {
                      LinePS = src_data + ColOffSet[K] * width;
                      for (int32 X = -Radius; X < width + Radius; X++)
                      {
                              ColHist[X * 256 + LinePS[RowOffset[X]]]++;
                      }
                  }
              }
              else                                                                                                                    //      �����е���ֱ��ͼ�����¾Ϳ�����
              {
                  LinePS = src_data + ColOffSet[Y - Radius - 1] * width;
                  for (int32 X = -Radius; X < width + Radius; X++)                        // ɾ���Ƴ���Χ�ڵ���һ�е�ֱ��ͼ����
                  {
                          ColHist[X * 256 + LinePS[RowOffset[X]]]--;
                  }

                  LinePS = src_data + ColOffSet[Y + Radius] * width;
                  for (int32 X = -Radius; X < width + Radius; X++)                // ���ӽ��뷶Χ�ڵ���һ�е�ֱ��ͼ����
                  {
                          ColHist[X * 256 + LinePS[RowOffset[X]]]++;
                  }
              }

              memset(Hist, 0, 256 * sizeof(uint16));                                          //      ÿһ��ֱ��ͼ����������

              LinePS = src_data + Y * width;
              LinePD = des_data + Y * width;

              for (int32 X = 0; X < width; X++)
              {
                  if (X == 0)
                  {
                          for (int32 K = -Radius; K <= Radius; K++)                       //      �е�һ�����أ���Ҫ���¼���
                                                                                                                                  //�޷��Ŷ�����ֱ��ͼ������ӣ�Y = X + Y
                                  HistgramAddShort(ColHist + K * 256, Hist);
                  }
                  else
                  {
                          //�޷��Ŷ�����ֱ��ͼ������Ӽ���Z = Z + Y - X
                          HistgramSubAddShort(ColHist + RowOffset[X - Radius - 1] * 256, ColHist + RowOffset[X + Radius] * 256, Hist);  //        �����������أ�����ɾ�������ӾͿ�����
                  }
                  Calc(Hist, LinePS[0], LinePD, Threshold);
                  LinePS++;
                  LinePD++;
              }
          }

          ColHist -= Radius * 256;                //      �ָ�ƫ�Ʋ������ͷſռ�
          free(ColHist);
          free(Hist);

          return RET_OK;
  }


  //扩充行映射表(会分配内存、不需要主动释放)
  RET_CODE SelectiveBlur::ValidCoordinateRow(RowHistogram& rowHistogram,int32 width, int32 left, int32 right, EdgeMode edgeMode)
  {
          rowHistogram.width = width + left + right;
          rowHistogram.data = (int32*)malloc(rowHistogram.width*sizeof(int32));
          if (rowHistogram.data == nullptr)
          {
                  return RET_ERR_OUTOFMEMORY;
          }
          int32 temp;

          for (int32 x = -left; x < width + right; x++)
          {
                  if (x < 0)
                  {
                          if (edgeMode == Edge_Mode_Repeat)               //重复
                          {
                                  rowHistogram.data[x + left] = 0;
                          }
                          else // 做镜像数据
                          {
                                  temp = -x;
                                  temp %= width;                                                  //防止越界
                                  rowHistogram.data[x + left] = temp;
                          }
                  }
                  else if (x > width)
                  {
                          if (edgeMode == Edge_Mode_Repeat)               //重复
                                  rowHistogram.data[x + left] = width - 1;
                          else
                          {
                                  temp = width - (x - width + 2);
                                  while (temp < 0) temp += width;
                                  rowHistogram.data[x + left] = temp;
                          }
                  }
                  else
                  {
                          rowHistogram.data[x + left] = x;
                  }
          }
          return RET_OK;
  }

  //扩充列映射表(会分配内存、不需要主动释放)
  RET_CODE SelectiveBlur::ValidCoordinateCol(ColHistogram& colHistogram, int32 heigth, int32 top, int32 bottom, EdgeMode edgeMode)
  {
          colHistogram.heigth = heigth + top + bottom;
          colHistogram.data = (int32*)malloc(colHistogram.heigth * sizeof(int32));
          if (colHistogram.data == nullptr)
          {
                  return RET_ERR_OUTOFMEMORY;
          }
          int32 temp;

          for (int32 h = -top; h < heigth + bottom; h++)
          {
                  if (h < 0)
                  {
                          if (edgeMode == Edge_Mode_Repeat)               //重复
                          {
                                  colHistogram.data[h + top] = 0;
                          }
                          else // 做镜像数据
                          {
                                  temp = -h;
                                  temp %= heigth;                                                 //防止越界
                                  colHistogram.data[h + top] = temp;
                          }
                  }
                  else if (h > heigth)
                  {
                          if (edgeMode == Edge_Mode_Repeat)               //重复
                                  colHistogram.data[h + top] = heigth - 1;
                          else
                          {
                                  temp = heigth - (h - heigth + 2);
                                  while (temp < 0) temp += heigth;
                                  colHistogram.data[h + top] = temp;
                          }
                  }
                  else
                  {
                          colHistogram.data[h + top] = h;
                  }
          }
          return RET_OK;
  }

    int32 SelectiveBlur::selectBlurYUV420p(YUV420p &src420p,YUV420p &des420p,int32 Radius,int32 Threshold, EdgeMode Edge)
    {
      if(src420p.size != des420p.size)
        {
          return -1;
        }
      if (Radius >= 127)
              Radius = 126;
      if (Threshold < 2)
              Threshold = 2;
      else if (Threshold > 255)
              Threshold = 255;
      RowHistogram YrowHistogram,UVrowHistogram;                    //行映射表
      ColHistogram YcolHistogram,UVcolHistogram;                      //列映射表
      //openMP优化
#pragma omp parallel num_threads(2)
      {
#pragma omp sections
       {
#pragma omp section     //Y单独一个section
         {
        // 按照指定的边缘模式计算扩展后各坐标的有理值
        ValidCoordinateRow(YrowHistogram,src420p.width,Radius,Radius, Edge);
        ValidCoordinateCol(YcolHistogram,src420p.height,Radius,Radius, Edge);
        selectBlurOneChannel((uint8*)src420p.Y,(uint8*)des420p.Y,src420p.width,src420p.height,YrowHistogram,YcolHistogram,Radius,Threshold);
         }
#pragma omp section      //UV合用一个section
         {
        ValidCoordinateRow(UVrowHistogram,src420p.width/4,Radius,Radius, Edge);
        ValidCoordinateCol(UVcolHistogram,src420p.height/4,Radius,Radius, Edge);
        selectBlurOneChannel((uint8*)src420p.U,(uint8*)des420p.U,src420p.width/4,src420p.height/4,UVrowHistogram,UVcolHistogram,Radius/4,Threshold);
        selectBlurOneChannel((uint8*)src420p.V,(uint8*)des420p.V,src420p.width/4,src420p.height/4,UVrowHistogram,UVcolHistogram,Radius/4,Threshold);
         }
        }
      }
      return RET_OK;
    }

    int32 SelectiveBlur::selectBlurYUV420p_Y(YUV420p &src420p,int32 Radius,int32 Threshold, EdgeMode Edge)
    {
      if (Radius >= 127)
           Radius = 126;
      if (Threshold < 2)
              Threshold = 2;
      else if (Threshold > 255)
              Threshold = 255;
      int frame = src420p.width*src420p.height;
      uint8* des_y = (uint8*)malloc(frame);
      RowHistogram YrowHistogram;                    //行映射表
      ColHistogram YcolHistogram;                    //列映射表
      // 按照指定的边缘模式计算扩展后各坐标的有理值
      ValidCoordinateRow(YrowHistogram,src420p.width,Radius,Radius, Edge);
      ValidCoordinateCol(YcolHistogram,src420p.height,Radius,Radius, Edge);
      selectBlurOneChannel((uint8*)src420p.Y,des_y,src420p.width,src420p.height,YrowHistogram,YcolHistogram,Radius,Threshold);
      memcpy(src420p.Y,des_y,frame);
      free(des_y);
      return 0;

    }
}
