#ifndef _NATIVE_YUV_SURFACE_H_
#define _NATIVE_YUV_SURFACE_H_

#include "RenderOpenGles20.h"

class NativeYUVSurface
{
public:
	NativeYUVSurface(JavaVM* jvm,void* window);
	//angle 旋转角度 支持（0,90,180,270，-90）
	NativeYUVSurface(JavaVM* jvm,void* window,int angle);
	~NativeYUVSurface();
	int32_t Init();
	void PutNV12DataAndRender(uint8_t* buf,int32_t length,int32_t widht,int32_t height);
	void PutYUV420pDataAndRender(uint8_t* buf,int32_t length,int32_t widht,int32_t height);
	void DeliverFrame(int32_t widht, int32_t height);
	void GetDataBuf(uint8_t*& pbuf, int32_t& isize);
	jint CreateOpenGLNative(int width, int height);
	void DrawNative();

private:
    JavaVM*     _jvm;
    void* _ptrWindow;

    jobject _javaRenderObj;
    jclass _javaRenderClass;
    JNIEnv* _javaRenderJniEnv;

    jmethodID      _redrawCid;
    jmethodID      _registerNativeCID;
    jmethodID      _deRegisterNativeCID;

    RenderOpenGles20 *_p_openGLRenderer;

    uint8_t* _buffer;
    int32_t _widht;
    int32_t _height;
};

#endif
