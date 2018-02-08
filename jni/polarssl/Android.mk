LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)	#用AS的cmake编译
LOCAL_MODULE:= polarssl
LOCAL_SRC_FILES:= libpolarssl.so
include $(PREBUILT_SHARED_LIBRARY)
