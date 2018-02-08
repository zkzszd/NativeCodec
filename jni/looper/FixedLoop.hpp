#ifndef _FIXED_LOOP_HPP_
#define _FIXED_LOOP_HPP_

#include "looper.h"
#include <android/log.h>
#define TAG "FixedLoop"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

namespace WeiYu
{
  //固定长度loop，超出loop，删除最老消息
  class FixedLoop: public looper
  {
  public:
    FixedLoop(int messageLen);
  private:
   virtual void addmsg(loopermessage *msg, bool flush);
  private:
   int _MaxMsgLen;
  };
}

#endif
