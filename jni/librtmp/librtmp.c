#include "librtmp.h"

JNIEXPORT jint JNICALL Java_org_limlee_rtmplib_RTMPUtil_open
        (JNIEnv *env, jobject obj, jstring url) {
    char *open_url = (char *) (*env)->GetStringUTFChars(env, url, 0);
    int result = rtmpclient_open(open_url);
    (*env)->ReleaseStringUTFChars(env, url, open_url);
    return result;
}

jint JNICALL Java_org_limlee_rtmplib_RTMPUtil_close
        (JNIEnv *env, jobject obj) {
    return rtmpclient_close();
}

jint JNICALL Java_org_limlee_rtmplib_RTMPUtil_isConnected
        (JNIEnv *env, jobject obj) {
    return rtmpclient_isConnected();
}

jint JNICALL Java_org_limlee_rtmplib_RTMPUtil_writeFlvFrame
        (JNIEnv *env, jobject obj, jbyteArray flv_data, jint flv_size) {
    jbyte *pFlv_data = (*env)->GetByteArrayElements(env, flv_data, NULL);
    int result = rtmpclient_writeFlvFrame((char *) pFlv_data, flv_size);
    (*env)->ReleaseByteArrayElements(env, flv_data, pFlv_data, 0);
    return result;
}
