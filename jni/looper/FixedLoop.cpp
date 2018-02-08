#include "FixedLoop.hpp"


namespace WeiYu
{
  FixedLoop::FixedLoop(int MaxMsgLen):_MaxMsgLen(MaxMsgLen),looper()
  {
  }

  void FixedLoop::addmsg(loopermessage *msg, bool flush)
  {
    sem_wait(&headwriteprotect);
    if (flush) {
        _msgQueue.clear();
    }
    if(_msgQueue.size() >= _MaxMsgLen)  //移除一个消息
    {
        loopermessage *tempMsg =  _msgQueue.front();
        _msgQueue.pop_front();
        delete tempMsg->obj;
        delete tempMsg;
    }
    _msgQueue.push_back(msg);
    sem_post(&headwriteprotect);
    sem_post(&headdataavailable);
  }
}
