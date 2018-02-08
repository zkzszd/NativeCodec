package com.example.nativecodec;


import java.util.Arrays;

import com.example.nativecodec.EFCameraView.EFCameraViewListener;
import com.zkzszd.zkzszdnativecode.R;

import android.R.string;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

/** 
 * @author  zkzszd E-mail:lnb@love.com
 * @date 创建时间：2017-11-23 下午5:42:29 
 * @version 1.0 
 * @parameter  
 * @since  
 * @return  
 */
public class NativeCodec extends Activity {

	private static boolean useNDK = true;
	private boolean useOld = false;
	//new File(Environment.getExternalStorageDirectory(), "Download/video_encoded.264")
	private static final String SDK_save_path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES).getPath() + "/SDK_recoder.h264";
	private static final String BASE_PATH = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES).getPath() + "/NDK_recoder.h264";
	private Button bt_open;
	private EFCameraView camera_view;
	SurfaceView mSurfaceView1;
    SurfaceHolder mSurfaceHolder1;
    VideoSink mSelectedVideoSink;
    SurfaceHolderVideoSink mSurfaceHolder1VideoSink;
    private String TAG = "1111";
    private AvcEncoder encoder;// = new AvcEncoder();
    private SurfaceView svYUVsurface = null;
    private boolean audioState = true;
    
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		quit();
	}
	
	@SuppressLint("NewApi")
    private int getSupportColorFormat() {
        int numCodecs = MediaCodecList.getCodecCount();
        MediaCodecInfo codecInfo = null;
        for (int i = 0; i < numCodecs && codecInfo == null; i++) {
            MediaCodecInfo info = MediaCodecList.getCodecInfoAt(i);
            if (!info.isEncoder()) {
                continue;
            }
            String[] types = info.getSupportedTypes();
            boolean found = false;
            for (int j = 0; j < types.length && !found; j++) {
                if (types[j].equals("video/avc")) {
                    System.out.println("found");
                    found = true;
                }
            }
            if (!found)
                continue;
            codecInfo = info;
        }

        Log.e("AvcEncoder", "Found " + codecInfo.getName() + " supporting " + "video/avc");

        // Find a color profile that the codec supports
        MediaCodecInfo.CodecCapabilities capabilities = codecInfo.getCapabilitiesForType("video/avc");
        Log.e("AvcEncoder",
                "length-" + capabilities.colorFormats.length + "==" + Arrays.toString(capabilities.colorFormats));

        for (int i = 0; i < capabilities.colorFormats.length; i++) {

            switch (capabilities.colorFormats[i]) {
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedSemiPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_QCOM_FormatYUV420SemiPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar:

                Log.e("AvcEncoder", "supported color format::" + capabilities.colorFormats[i]);
                return capabilities.colorFormats[i];
            default:
                Log.e("AvcEncoder", "unsupported color format " + capabilities.colorFormats[i]);
                break;
            }
        }

        return -1;
    }
	
	//@TargetApi(Build.VERSION_CODES.JELLY_BEAN)  
	private void selectCodec(String mimeType) {  
	    int numCodecs = MediaCodecList.getCodecCount();  
	    for (int i = 0; i < numCodecs; i++) {  
	        MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);  
	  
	        if (codecInfo.isEncoder()) {  
	            continue;  
	        }  
	  
	        for (String type : codecInfo.getSupportedTypes()) {  
	            //if (type.equalsIgnoreCase(mimeType)) {  
	                Log.i("TAG", "zkzszd SelectCodec : " + type);//codecInfo.getName());  
	            //}  
	        }  
	    }  
	}  
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.camera);
		
		//判断编码支持
		selectCodec("video/avc");
		//判断编码"video/avc"支持的颜色格式
		getSupportColorFormat();
		
		svYUVsurface = (SurfaceView)findViewById(R.id.sv_yuv);
		mSurfaceView1 = (SurfaceView)findViewById(R.id.sv_native);
		mSurfaceHolder1 = mSurfaceView1.getHolder();
		
		mSurfaceHolder1.addCallback(new SurfaceHolder.Callback() {

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                Log.v(TAG, "surfaceChanged format=" + format + ", width=" + width + ", height="
                        + height);
            }

            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Log.v(TAG, "surfaceCreated");
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                Log.v("111", "surfaceDestroyed");
            }

        });
        mSelectedVideoSink = new SurfaceHolderVideoSink(mSurfaceHolder1);
        
        
		findViewById(R.id.bt_end).setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				mSelectedVideoSink.useAsSinkForNative();
				
				if(useNDK)
				{
					
				}
				else
				{
					encoder.close();
				}
			}
		});
		
		findViewById(R.id.bt_player).setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				String url = "rtmp://live.hkstv.hk.lxdns.com/live/hks";
				//String url = "rtmp://pubsec.mudu.tv/watch/0t5ejs?auth_key=2082733261-0-0-8f2e55e5dcd07229b89bf88d29b08220";
				NativePlayer.createPlayer(url, mSelectedVideoSink.getSurface());
//				createStreamingMediaPlayer(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES).getPath() + "/recoder.h264", 
//						mSelectedVideoSink.getSurface());
			}
		});
		
		bt_open = (Button) findViewById(R.id.bt_open);
		bt_open.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				
				if(useNDK)
				{
					//Player();
					//createSendH264File(Environment.getExternalStorageDirectory().getPath() + "/receive.h264");
//					new Runnable() {
//						public void run() {
//							camera_view.OpenCamera();
//							camera_view.pausePreview();
//							createStreamingRecode(BASE_PATH);
//						}
//					}.run();
					
//					createStreamingMediaPlayer(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES).getPath() + "/recoder.h264", mSelectedVideoSink.getSurface());
					
					//家
					//if(NativePusher.initPusherWorkPusher("rtmp://192.168.31.139:1935/live/live11.sdp"))
				    //公司
					//if(NativePusher.initPusherWorkPusher("rtmp://192.168.152.177:1935/live/live11.sdp"))
					//目睹直播地址
					if(NativePusher.initPusherWorkPusher("rtmp://pubsec.mudu.tv/watch/0t5ejs?auth_key=2082733261-0-0-8f2e55e5dcd07229b89bf88d29b08220"))
					//if(NativePusher.initPusherWorkPusher("rtmp://push.shangzhibo.tv/HyeQ4PCSrf/SJZm4PCHBG?auth_key=1548323499-0-0-11e8f440d2fbb7b0a5ebcad0e716cf49"))
					//if(NativePusher.initPusherWorkPusher("rtmp://192.168.83.67:1935/live/live11.sdp"))
					{
						int bitRate = (int) (640*320*1.5*25/1000*8);
						Log.d("at java","bitRate"+bitRate);
						//OMX_COLOR_FormatYUV420SemiPlanar 21这是手机默认格式编码格式，低端手机不支持yuv420p
						NativePusher.initVideoEncode(640, 480,8,bitRate, 1, 21,svYUVsurface);
						NativePusher.initAudioEncode(2, 44100);
						camera_view.OpenCamera(camera_view.TYPE_FRONT);
						camera_view.pausePreview();
					}
				}
				else
				{
					camera_view.OpenCamera(camera_view.TYPE_FRONT);
					camera_view.pausePreview();
					encoder = new AvcEncoder(SDK_save_path);
				}
			}
		});
		
		findViewById(R.id.bt_audio).setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				audioState = !audioState;
			
				Log.e("zkzszd","zkzszd :"+NativePlayer.setPlayAudioState(audioState));
			}
		});
		
		findViewById(R.id.bt_old).setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				new Runnable() {
				public void run() {
					
					createStreamingRecode(BASE_PATH);
					camera_view.OpenCamera(camera_view.TYPE_FRONT);
					camera_view.pausePreview();
					useOld = true;
				}
			}.run();
			}
		});
		
		
		
		camera_view = (EFCameraView) findViewById(R.id.camera_view);
		camera_view.setCameraListenrer(new EFCameraViewListener() {
			
			@Override
			public void onCameraPreviewFrame(int width, int height, byte[] data,
					int format) {
				// TODO Auto-genrated method stub
				//Log.d("nativecode","zkzszd  at java put data");
				//Log.d("nativecode","zkzszd  at java put data:" + data[0] + data[1] + data[2]);
				byte[] nv12 = new byte[(int) (width*height*1.5)];
				
				NV21ToNV12(data,nv12,width,height);
				if(useNDK)
				{
					//putCameraData(width,height,nv12);
					if(!useOld)
					{
						NativePusher.putCameraData(data, data.length,width,height);
						//NativePusher.putCameraData(nv12, nv12.length,width,height);//width*height*3/2);
					}
					else
					{
						putCameraData(width,height,nv12);
					}
				}
				else
				{
					encoder.offerEncoder(nv12);
				}
				
			}
		});
	}
	
	private void NV21ToNV12(byte[] nv21,byte[] nv12,int width,int height){
		if(nv21 == null || nv12 == null)return;
		int framesize = width*height;
		int i = 0,j = 0;
		System.arraycopy(nv21, 0, nv12, 0, framesize);
		for(i = 0; i < framesize; i++){
		nv12[i] = nv21[i];
		}
		for (j = 0; j < framesize/2; j+=2)
		{
		 nv12[framesize + j-1] = nv21[j+framesize];
		}
		for (j = 0; j < framesize/2; j+=2)
		{
		 nv12[framesize + j] = nv21[j+framesize-1];
		}
	}
	
	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
		NativePlayer.PlayerQuit();
	}

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
	
    /** Native methods, implemented in jni folder */
    public static native void createEngine();
    public static native boolean createStreamingRecode(String filename);
    public static native void setPlayingStreamingMediaPlayer(boolean isPlaying);
    public static native void shutdown();
    public static native void setSurface(Surface surface);
    public static native void rewindStreamingMediaPlayer();
    public static native void EOS();
    public static native void putCameraData(int width,int height,byte[] data);
    public static native boolean createStreamingMediaPlayer(String filename,Surface surface);
    public static native boolean createSendH264File(String filename);
    public static native boolean Player();
    public static native void quit();
    
    
 // VideoSink abstracts out the difference between Surface and SurfaceTexture
    // aka SurfaceHolder and GLSurfaceView
    static abstract class VideoSink {

        abstract void setFixedSize(int width, int height);
        abstract void useAsSinkForNative();
        abstract Surface getSurface();

    }

    static class SurfaceHolderVideoSink extends VideoSink {

        private final SurfaceHolder mSurfaceHolder;

        SurfaceHolderVideoSink(SurfaceHolder surfaceHolder) {
            mSurfaceHolder = surfaceHolder;
        }

        @Override
        void setFixedSize(int width, int height) {
            mSurfaceHolder.setFixedSize(width, height);
        }
        
        @Override
        Surface getSurface()
        {
        	return mSurfaceHolder.getSurface();
        }

        @Override
        void useAsSinkForNative() {
            Surface s = mSurfaceHolder.getSurface();
            Log.i("@@@", "java setting surface " + s);
            if(useNDK)
            {
            	 setSurface(s);
            }
            else
            {
            	
            }
        }

    }
	
	
}
