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

#include "looper.h"
#include <assert.h>
#include <jni.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <semaphore.h>

// for __android_log_print(ANDROID_LOG_INFO, "YourApp", "formatted message");
#include <android/log.h>
#define TAG "looper"
#define LOGV(...) //__android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

void* looper::trampoline(void* p) {
    LOGV("at looper trampoline");
    ((looper*)p)->loop();
    return NULL;
}

looper::looper() {
    LOGV("at looper create");
//    head = NULL;
    sem_init(&headdataavailable, 0, 0);
    sem_init(&headwriteprotect, 0, 1);
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_create(&worker, &attr, trampoline, this);
    running = true;
}


looper::~looper() {
    if (running) {
        LOGV("Looper deleted while still running. Some messages will not be processed");
        quit();
    }
}

//
void looper::post(int what, LoopMsgObj *data, bool flush) {
    loopermessage *msg = new loopermessage();
    msg->what = what;
    msg->obj = data;
    msg->quit = false;
    addmsg(msg, flush);
}

void looper::addmsg(loopermessage *msg, bool flush) {
    sem_wait(&headwriteprotect);
    if (flush) {
        _msgQueue.clear();
    }
    _msgQueue.push_back(msg);
    LOGV("post msg %d", msg->what);
    sem_post(&headwriteprotect);
    sem_post(&headdataavailable);
}

void looper::loop() {
    LOGV("at loop");
    while(true)
      {
        // wait for available message
        sem_wait(&headdataavailable);
        LOGV("headdataavailable");
        // get next available message
        sem_wait(&headwriteprotect);
        LOGV("headwriteprotect");
        if(_msgQueue.size() > 0)
        {
            loopermessage *msg = _msgQueue.front();
            _msgQueue.pop_front();

            //quit 退出
            if (msg->quit)
            {
                delete msg->obj;
                delete msg;
                while(_msgQueue.size() > 0)
                {
                    msg = _msgQueue.front();
                    _msgQueue.pop_front();
                    delete msg->obj;
                    delete msg;
                }
                sem_post(&headwriteprotect);
                return;
            }

            sem_post(&headwriteprotect);

            LOGV("processing msg %d", msg->what);
            handle(msg->what, msg->obj);
            delete msg;
        }
        else
        {
            LOGV("no msg");
            sem_post(&headwriteprotect);
            continue;
        }
    }
}

void looper::quit() {
    LOGV("quit");
    loopermessage *msg = new loopermessage();
    msg->what = 0;
    msg->obj = NULL;
    msg->quit = true;
    addmsg(msg, true);
    void *retval;
    pthread_join(worker, &retval);
    sem_destroy(&headdataavailable);
    sem_destroy(&headwriteprotect);
    running = false;
}

void looper::handle(int what, LoopMsgObj* obj) {
    LOGV("dropping msg %d %p", what, obj);
}
