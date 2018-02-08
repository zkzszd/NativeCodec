#include "NaluLoop.hpp"

namespace WeiYu
{
  NaluLoop::NaluLoop(int QueueNaluLen):_MaxNalu(QueueNaluLen),looper()
  {}

  //队列里面既有音频，也有视频
  void NaluLoop::addmsg(loopermessage *msg, bool flush)
  {
    sem_wait(&headwriteprotect);
    if (flush) {
        _msgQueue.clear();
    }
    if(_msgQueue.size() >= _MaxNalu)  //移除消息,直到下一个I帧，或者队列为空
    {
        loopermessage *tempMsg =  _msgQueue.front();
        _msgQueue.pop_front();
        delete (NaluStruct*)tempMsg->obj;
        delete tempMsg;

        while(_msgQueue.size() > 0)
        {
            tempMsg =  _msgQueue.front();
            if(((NaluStruct*)tempMsg->obj)->type == 5)
            {
               break;
            }
            _msgQueue.pop_front();
            delete tempMsg->obj;
            delete tempMsg;
        }
    }
    _msgQueue.push_back(msg);
    sem_post(&headwriteprotect);
    sem_post(&headdataavailable);
  }
}
