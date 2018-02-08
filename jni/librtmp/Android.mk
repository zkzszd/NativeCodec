LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := librtmp
LOCAL_SRC_FILES := amf.c \
				   	hashswf.c \
					log.c \
					parseurl.c \
					rtmp.c \
					librtmp.c \
					rtmpclient.c \
					CRTMPStream.cpp \
					RTMPObject.cpp \
					SpsUtil.cpp \
					RTMPPlayer.cpp \
					RTMPPusher.cpp \
					
LOCAL_CFLAGS 	+= -std=c++11
LOCAL_CPPFLAGS 	+= -std=c++11
LOCAL_LDLIBS += -lz
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../
LOCAL_LDLIBS +=-L$(SYSROOT)/usr/lib -llog
LOCAL_CFLAGS += -DUSE_POLARSSL
LOCAL_SHARED_LIBRARIES += polarssl 
LOCAL_STATIC_LIBRARIES += util audio

include $(BUILD_STATIC_LIBRARY)