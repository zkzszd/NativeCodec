#include "MediaCodecUtil.h"
#include <linux/time.h>
#include <time.h>

namespace WeiYu
{
  int64_t systemnanotime() {
      timespec now;
      clock_gettime(CLOCK_MONOTONIC, &now);
      return now.tv_sec * 1000000000LL + now.tv_nsec;
  }

  //从buf中获取一个nalu结束位置，如果没找到，返回length-1，如果buf开始不是00000001返回0
  int readOneNaluFromBuf(char* data,int length)
  {
    int pos = 4;
    if (data[0] == 0x00 &&
              data[1] == 0x00 &&
              data[2] == 0x00 &&
              data[3] == 0x01)
      {
        while(pos < length)
          {
            if (data[pos++] == 0x00 &&
                data[pos++] == 0x00 &&
                data[pos++] == 0x00 &&
                data[pos++] == 0x01)
              {
                pos -= 4;
                break;
              }
          }
      }
    else
      {
        return 0;
      }

    return pos;
  }
}
