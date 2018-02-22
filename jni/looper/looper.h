/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _LOOPER_H_
#define _LOOPER_H_
#include <pthread.h>
#include <semaphore.h>
#include <deque>
using namespace std;

//post 消息对象基类
class LoopMsgObj
{
public:
  LoopMsgObj(){}
  ~LoopMsgObj(){}
};

struct loopermessage;
typedef struct loopermessage loopermessage;

// 消息载体
struct loopermessage {
    int what;
    LoopMsgObj *obj;
    bool quit;
};

class looper {
    public:
        looper();
        virtual ~looper();

        //flush 是否清空消息队列
        void post(int what, LoopMsgObj *data, bool flush = false);
        void quit();
        virtual void handle(int what, LoopMsgObj *data);
    private:
        virtual void addmsg(loopermessage *msg, bool flush);
        static void* trampoline(void* p);
        void loop();
    protected:
        std::deque< loopermessage * > _msgQueue;
        pthread_t worker;
        sem_t headwriteprotect;
        sem_t headdataavailable;
        bool running;
};
#endif
