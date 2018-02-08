LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
	
LOCAL_MODULE := base
LOCAL_SRC_FILES := MyDef.cpp NXBuffer.cpp NXPtr.hpp NXLock.cpp NXDef.h

LOCAL_CFLAGS 	+= -std=c++11
LOCAL_CPPFLAGS 	+= -std=c++11
				 		 
include $(BUILD_STATIC_LIBRARY)