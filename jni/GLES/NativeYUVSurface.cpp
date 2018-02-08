#include <jni.h>
#include <stdlib.h>
#include <stdio.h>

#include "NativeYUVSurface.h"


static jint CreateOpenGLNativeStatic(JNIEnv * env,
									jobject,
									jlong context,
									jint width,
									jint height)
{
	NativeYUVSurface* renderChannel =
	reinterpret_cast<NativeYUVSurface*> (context);

	WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, -1, "%s:", __FUNCTION__);

	return renderChannel->CreateOpenGLNative(width, height);
}

static void DrawNativeStatic(JNIEnv * env,jobject, jlong context)
{
	NativeYUVSurface* renderChannel =
	reinterpret_cast<NativeYUVSurface*>(context);
	renderChannel->DrawNative();
}

NativeYUVSurface::NativeYUVSurface(JavaVM* jvm,void* window)
{
	_jvm = jvm;
	_ptrWindow = window;
	_buffer = (uint8_t*)malloc(1024000);
	_p_openGLRenderer = new RenderOpenGles20();
}

NativeYUVSurface::NativeYUVSurface(JavaVM* jvm,void* window,int angle)
{
	_jvm = jvm;
	_ptrWindow = window;
	_buffer = (uint8_t*)malloc(1024000);
	_p_openGLRenderer = new RenderOpenGles20(angle);
}


NativeYUVSurface::~NativeYUVSurface()
{
	if (_jvm)
	{
		bool isAttached = false;
		JNIEnv* env = NULL;
		if (_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
			// try to attach the thread and get the env
			// Attach this thread to JVM
			jint res = _jvm->AttachCurrentThread(&env, NULL);

			// Get the JNI env for this thread
			if ((res < 0) || !env) {
				WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
							 "%s: Could not attach thread to JVM (%d, %p)",
							 __FUNCTION__, res, env);
				env = NULL;
			} else {
				isAttached = true;
			}
		}

		if (env && _deRegisterNativeCID) {
			env->CallVoidMethod(_javaRenderObj, _deRegisterNativeCID);
		}

		env->DeleteGlobalRef(_javaRenderObj);
		env->DeleteGlobalRef(_javaRenderClass);

		if (isAttached) {
			if (_jvm->DetachCurrentThread() < 0) {
				WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id,
							 "%s: Could not detach thread from JVM",
							 __FUNCTION__);
			}
		}
	}

	free(_buffer);
	free(_p_openGLRenderer);
}

int32_t NativeYUVSurface::Init()
{
	if (!_ptrWindow)
	{
		WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id,
					 "(%s): No window have been provided.", __FUNCTION__);
		return -1;
	}

	if (!_jvm)
	{
		WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id,
					 "(%s): No JavaVM have been provided.", __FUNCTION__);
		return -1;
	}

	// get the JNI env for this thread
	bool isAttached = false;
	JNIEnv* env = NULL;
	if (_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		// try to attach the thread and get the env
		// Attach this thread to JVM
		jint res = _jvm->AttachCurrentThread(&env, NULL);

		// Get the JNI env for this thread
		if ((res < 0) || !env) {
			WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
						 "%s: Could not attach thread to JVM (%d, %p)",
						 __FUNCTION__, res, env);
			return -1;
		}
		isAttached = true;
	}

	// get the AndroidGLES20 class
	jclass javaRenderClassLocal = reinterpret_cast<jclass> (env->FindClass("zkzszd/opengl/AndroidGLES20"));
	if (!javaRenderClassLocal) {
		WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
					 "%s: could not find AndroidGLES20", __FUNCTION__);
		return -1;
	}

	_javaRenderClass = reinterpret_cast<jclass> (env->NewGlobalRef(javaRenderClassLocal));
	if (!_javaRenderClass) {
		WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
					 "%s: could not create Java SurfaceHolder class reference",
					 __FUNCTION__);
		return -1;
	}

	// Delete local class ref, we only use the global ref
	env->DeleteLocalRef(javaRenderClassLocal);
	jmethodID cidUseOpenGL = env->GetStaticMethodID(_javaRenderClass,
													"UseOpenGL2",
													"(Ljava/lang/Object;)Z");
	if (cidUseOpenGL == NULL) {
		WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, -1,
					 "%s: could not get UseOpenGL ID", __FUNCTION__);
		return false;
	}
	jboolean res = env->CallStaticBooleanMethod(_javaRenderClass,
												cidUseOpenGL, (jobject) _ptrWindow);

	// create a reference to the object (to tell JNI that we are referencing it
	// after this function has returned)
	_javaRenderObj = reinterpret_cast<jobject> (env->NewGlobalRef((jobject)_ptrWindow));
	if (!_javaRenderObj)
	{
		WEBRTC_TRACE(
					 kTraceError,
					 kTraceVideoRenderer,
					 _id,
					 "%s: could not create Java SurfaceRender object reference",
					 __FUNCTION__);
		return -1;
	}

	// get the method ID for the ReDraw function
	_redrawCid = env->GetMethodID(_javaRenderClass, "ReDraw", "()V");
	if (_redrawCid == NULL) {
		WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
					 "%s: could not get ReDraw ID", __FUNCTION__);
		return -1;
	}

	_registerNativeCID = env->GetMethodID(_javaRenderClass,
										  "RegisterNativeObject", "(J)V");
	if (_registerNativeCID == NULL) {
		WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
					 "%s: could not get RegisterNativeObject ID", __FUNCTION__);
		return -1;
	}

	_deRegisterNativeCID = env->GetMethodID(_javaRenderClass,
											"DeRegisterNativeObject", "()V");
	if (_deRegisterNativeCID == NULL) {
		WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
					 "%s: could not get DeRegisterNativeObject ID",
					 __FUNCTION__);
		return -1;
	}

	JNINativeMethod nativeFunctions[2] = {
		{ "DrawNative",
			"(J)V",
			(void*) &DrawNativeStatic, },
		{ "CreateOpenGLNative",
			"(JII)I",
			(void*) &CreateOpenGLNativeStatic },
	};
	if (env->RegisterNatives(_javaRenderClass, nativeFunctions, 2) == 0) {
		WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, -1,
					 "%s: Registered native functions", __FUNCTION__);
	}
	else {
		WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, -1,
					 "%s: Failed to register native functions", __FUNCTION__);
		return -1;
	}

	env->CallVoidMethod(_javaRenderObj, _registerNativeCID, (jlong) this);

	if (isAttached) {
		if (_jvm->DetachCurrentThread() < 0) {
			WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id,
						 "%s: Could not detach thread from JVM", __FUNCTION__);
		}
	}

	WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s done",
				 __FUNCTION__);

//        if (_p_openGLRenderer->SetCoordinates(zOrder, left, top, right, bottom) != 0) {
//            return -1;
//        }

	return 0;
}

void NativeYUVSurface::DeliverFrame(int32_t widht, int32_t height)
{
	if (_jvm)
	{
		bool isAttached = false;
		JNIEnv* env = NULL;
		if (_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
			// try to attach the thread and get the env
			// Attach this thread to JVM
			jint res = _jvm->AttachCurrentThread(&env, NULL);

			// Get the JNI env for this thread
			if ((res < 0) || !env) {
				WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
							 "%s: Could not attach thread to JVM (%d, %p)",
							 __FUNCTION__, res, env);
				env = NULL;
			} else {
				isAttached = true;
			}
		}

		if (env && _redrawCid)
		{
			_widht = widht;
			_height = height;

			env->CallVoidMethod(_javaRenderObj, _redrawCid);
		}

		if (isAttached) {
			if (_jvm->DetachCurrentThread() < 0) {
				WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id,
							 "%s: Could not detach thread from JVM",
							 __FUNCTION__);
			}
		}
	}
}

void NativeYUVSurface::GetDataBuf(uint8_t*& pbuf, int32_t& isize)
{
	pbuf = _buffer;
	isize = 1024000;
}

void NativeYUVSurface::PutNV12DataAndRender(uint8_t* buf,int32_t length,int32_t width,int32_t height)
{
	if(buf == NULL || length >1024000)
	{
	  return;
	}
	int framesize = width*height;
	uint8_t* YUV420p = (uint8_t*)malloc(length*sizeof(uint8_t));
	memcpy(YUV420p,buf,framesize);

	uint8_t* u = YUV420p + framesize;
	uint8_t* v = u + framesize/4;
	uint8_t* src = buf + framesize;
	for(int i = 0;i<framesize/4;i++)
	{
		u[i] = src[i*2];
		v[i] = src[i*2+1];
	}
	this->PutYUV420pDataAndRender(YUV420p,length,width,height);
}

void NativeYUVSurface::PutYUV420pDataAndRender(uint8_t* buf,int32_t length,int32_t widht,int32_t height)
{
  if(buf == NULL || length >1024000)
    {
      return;
    }
  memcpy(_buffer,buf,length);
  this->DeliverFrame(widht, height);
}

jint NativeYUVSurface::CreateOpenGLNative(int width, int height)
{
	return _p_openGLRenderer->Setup(width, height);
}

void NativeYUVSurface::DrawNative()
{
	_p_openGLRenderer->Render(_buffer, _widht, _height);
}
