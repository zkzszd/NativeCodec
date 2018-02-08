LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
	
LOCAL_MODULE := audio
LOCAL_SRC_FILES := AudioEncode.cpp \
				   AudioDecode.cpp \
				   opensl_io.c \
				   NXSLSpeaker.cpp

LOCAL_CFLAGS 	+= -std=c++11
LOCAL_CPPFLAGS 	+= -std=c++11

LOCAL_LDLIBS := -lGLESv1_CM -llog -lOpenSLES 
LOCAL_STATIC_LIBRARIES += libbase
				 		 
include $(BUILD_STATIC_LIBRARY)