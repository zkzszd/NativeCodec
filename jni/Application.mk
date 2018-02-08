APP_PLATFORM := android-16
#APP_PLATFORM := android-21
APP_ABI := armeabi-v7a
NDK_TOOLCHAIN_VERSION := 4.8
APP_STL=gnustl_static
APP_CPPFLAGS := -frtti -std=c++11 
LOCAL_CFLAGS 	+= -std=c++11
LOCAL_CPPFLAGS 	+= -std=c++11
APP_OPTIM := debug #release #debug