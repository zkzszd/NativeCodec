// for native window JNI
#include <android/native_window_jni.h>
#include "../librtmp/RTMPPlayer.hpp"

using namespace WeiYu;

#include <android/log.h>
#define TAG "NativeCodec"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

RTMPPlayer *player = NULL;

extern "C" {

  void Java_com_example_nativecodec_NativePlayer_PlayerQuit(JNIEnv* env,
      jclass clazz)
  {
    if(player != NULL)
      delete player;
  }

  jboolean Java_com_example_nativecodec_NativePlayer_setPlayAudioState(JNIEnv* env,jclass clazz,
      jboolean isRecAudio)
  {
    if(player != NULL)
      return player->setReceiveAudio(isRecAudio);
    return JNI_FALSE;
  }

  jboolean Java_com_example_nativecodec_NativePlayer_setPlayVideoState(JNIEnv* env,jclass clazz,
        jboolean isRecVideo)
    {
      if(player != NULL)
        return player->setReceiveVideo(isRecVideo);
      return JNI_FALSE;
    }

  jboolean Java_com_example_nativecodec_NativePlayer_createPlayer(JNIEnv* env,jclass clazz,
      jstring play_url,jobject surface)
  {
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    player = new RTMPPlayer(window);
    const char *utf8_url = env->GetStringUTFChars(play_url, NULL);


    env->ReleaseStringUTFChars(play_url, utf8_url);
    //rtmp://live.hkstv.hk.lxdns.com/live/hks
    //rtmp://192.168.152.177:1935/live/stream
    //亚太卫视 rtmp://58.61.150.198/live/Livestream
    //亚太第一卫视 rtmp://v1.one-tv.com/live/mpegts.stream
    //希望电视台 rtmp://203.69.85.37/livepkgr/hopestream1?adbe-live-event=hopetv
    //GOOD TV 1  rtmp://l.jdjys.net/livefy/livestream
    //GOOD TV 2  rtmp://l.jdjys.net/livefy2/livestream
    //台湾民视   rtmp://202.39.43.80:80/live_hich/BC000121_1 失败
    //if(player->connect("rtmp://192.168.83.67:1935/live/live11.sdp"))
    if(player->connect("rtmp://live.hkstv.hk.lxdns.com/live/hks"))    //香港卫视rtmp://send1.douyu.com/live
    //if(player->connect("rtmp://pubsec.mudu.tv/watch/0t5ejs?auth_key=2082733261-0-0-8f2e55e5dcd07229b89bf88d29b08220"))
    //if(player->connect("rtmp://send1.douyu.com/live/656505rBElAOPzEr?wsSecret=f9c648589c4852482b970befc8711de8&wsTime=5a55fd5d&wsSeek=off"))
      {
        LOGV("@@@ connect ok" );
        player->start();
        //player->readPacketThread();
      }
    else
      {
        LOGV("@@@ connect  failed" );
      }
    return JNI_TRUE;
  }

}
