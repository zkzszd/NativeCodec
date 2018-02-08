package com.example.nativecodec;

import android.os.Build;
import android.view.Surface;

/** 
 * @author  zkzszd E-mail:lnb@love.com
 * @date 创建时间：2018-2-1 下午2:19:00 
 * @version 1.0 
 * @parameter  
 * @since  
 * @return  
 */
public class NativePlayer 
{
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
    
    public static native boolean createPlayer(String url,Surface surface);
    public static native void PlayerQuit();
    public static native boolean setPlayAudioState(boolean isRecAudio);
    public static native boolean setPlayVideoState(boolean isRecVideo);
}
