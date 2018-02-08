LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := native-codec-jni
LOCAL_SRC_FILES := native-codec-jni.cpp \
					pusher-jni.cpp \
					player-jni.cpp 
# for native multimedia
LOCAL_LDLIBS    += -lOpenMAXAL 
#LOCAL_LDLIBS    += -lOpenMAXAL -lmediandk
# for logging
LOCAL_LDLIBS    += -llog
# for native windows
LOCAL_LDLIBS    += -landroid 
LOCAL_CFLAGS 	+= -std=c++11
LOCAL_CPPFLAGS 	+= -std=c++11
LOCAL_CFLAGS    += -UNDEBUG
LOCAL_SHARED_LIBRARIES += \
    libbinder \
    libmedia \
    libstagefright \
    libstagefright_foundation \
    liblog \
    libutils \
    libandroid_runtime \
    libbinder \

LOCAL_LDLIBS += -lOpenSLES #录音 -lGLESv1_CM
LOCAL_LDLIBS += -lGLESv2 	#GLES
LOCAL_LDLIBS += -fopenmp    #openmp
LOCAL_LDLIBS += -lz
    
LOCAL_STATIC_LIBRARIES +=  liblooper librtmp libutil libvideo libNativeCodec libaudio libbase libGLES#会生成依赖关系，当库不存在时会去编译这个库
#LOCAL_SHARED_LIBRARIES +=  libNativeCodec
include $(BUILD_SHARED_LIBRARY)
