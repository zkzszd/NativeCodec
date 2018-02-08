#video	
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := video
LOCAL_SRC_FILES := DecodeLoop.cpp \
				   EncodeLoop.cpp \
				   SelectiveBlur.cpp \
				   Blur.cpp
				 
LOCAL_CFLAGS 	+= -std=c++11 -fopenmp
LOCAL_CPPFLAGS 	+= -std=c++11 -fopenmp	#-fopenmp使用openmp
LOCAL_STATIC_LIBRARIES += liblooper libNativeCodec lib
# for native windows
LOCAL_LDLIBS    += -landroid 	

# -g 后面的一系列附加项目添加了才能使用 arm_neon.h 头文件  
# -mfloat-abi=softfp -mfpu=neon 使用 arm_neon.h 必须    
LOCAL_CFLAGS += -mfloat-abi=softfp -mfpu=neon -march=armv7-a -mtune=cortex-a8  

include $(BUILD_STATIC_LIBRARY)