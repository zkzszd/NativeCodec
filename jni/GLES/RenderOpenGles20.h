#ifndef RENDER_OPENGLES20_H_
#define RENDER_OPENGLES20_H_


#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <jni.h>
#include <android/log.h>


#ifdef DEBUG
#define WEBRTC_TRACE(a,b,c,...)  __android_log_print(ANDROID_LOG_DEBUG, "zkzszd_native_GL", __VA_ARGS__)
#else
#define WEBRTC_TRACE(a,b,c,...)
#endif


class RenderOpenGles20 {
 public:
  RenderOpenGles20();
  //angle 旋转角度
  RenderOpenGles20(int angle);
  ~RenderOpenGles20();

  int32_t Setup(int32_t widht, int32_t height);
  int32_t Render(void * data, int32_t widht, int32_t height);
  int32_t SetCoordinates(int32_t zOrder, const float left, const float top,
                         const float right, const float bottom);

 private:
  void printGLString(const char *name, GLenum s);
  void checkGlError(const char* op);
  GLuint loadShader(GLenum shaderType, const char* pSource);
  GLuint createProgram(const char* pVertexSource,
                       const char* pFragmentSource);
  void SetupTextures(int32_t width, int32_t height);
  void UpdateTextures(void* data, int32_t widht, int32_t height);

  int32_t _id;
  GLuint _textureIds[3]; // Texture id of Y,U and V texture.
  GLuint _program;
  GLsizei _textureWidth;
  GLsizei _textureHeight;

  GLfloat _vertices[20];
  static const char g_indices[];

  static const char g_vertextShader[];
  static const char g_fragmentShader[];

};


#endif  // RENDER_OPENGLES20_H_
