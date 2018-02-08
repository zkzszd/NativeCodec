#ifndef _NALU_LOOP_H_
#define _NALU_LOOP_H_

#include "looper.h"
#include "../base/MyDef.hpp"

#include <android/log.h>
#define TAG "NaluLoop"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

namespace WeiYu
{
  class NaluLoop : public looper
  {
  public:
    NaluLoop(int QueueNaluLen);
  private:
    virtual void addmsg(loopermessage *msg, bool flush);
  private:
     int _MaxNalu;
  };
}

#endif
