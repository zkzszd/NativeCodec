package com.example.nativecodec;

import android.os.Build;

/** 
 * @author  zkzszd E-mail:lnb@love.com
 * @date 创建时间：2017-12-27 下午3:03:44 
 * @version 1.0 
 * @parameter  
 * @since  
 * @return  
 */
public class NativePusher {
	
	/** Load jni .so on initialization */
    static {
    	 //System.loadLibrary("rtmp");
         System.loadLibrary("native-codec-jni");
         System.loadLibrary("polarssl");
         switch(Build.VERSION.SDK_INT)
     	{
     	case Build.VERSION_CODES.JELLY_BEAN:
     		System.loadLibrary("native_codec16");
     		break;
     	case Build.VERSION_CODES.JELLY_BEAN_MR1:
     		System.loadLibrary("native_codec17");
     		break;
     	case Build.VERSION_CODES.JELLY_BEAN_MR2:
     		System.loadLibrary("native_codec18");
     		break;
     	case Build.VERSION_CODES.KITKAT:
     		System.loadLibrary("native_codec19");
     		break;
     	}
    }
	
	public final static int DEF_WIDTH = 480;
	public final static int DEF_HEIGHT = 640;
	public final static int DEF_FRAME_RATE = 25;
	public final static int DEF_I_FRAME_INTERVAL = 1;  //1秒
	public final static int DEF_Channel_Count = 2;
	public final static int DEF_Sample_Rate = 15;
	public final static int DEF_FORMAT = 21;//OMX_COLOR_FormatYUV420SemiPlanar;

	public static native void putNaluData(byte[] Nalu_data,int length);
	public static native void putCameraData(byte[] cameraData,int length,int widht,int height);
	//视频bitRate = 一副图像数据*Framerate/1000*8
	public static native void initVideoEncode(int videoWidth,int videoHeight,int videoFramerate,
				int bitRate,int I_FRAME_INTERVAL,int videoFormat,Object glSurface);
	public static native boolean initPusherWorkPusher(String url);
	public static native void PusherWorkDestory();
	//音频bitRate = sampleRate*位深度（或者叫采样精度）
	public static native void initAudioEncode(int channelCount,int sampleRate);
	
	public static native void audioEncode();
}
