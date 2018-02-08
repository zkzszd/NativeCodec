package com.example.nativecodec;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.nio.ByteBuffer;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Environment;
import android.util.Log;

/** 
 * @author  zkzszd E-mail:lnb@love.com
 * @date 创建时间：2017-12-4 下午5:45:41 
 * @version 1.0 
 * @parameter  
 * @since  
 * @return  
 */
public class AvcEncoder 
{
	private MediaCodec mediaCodec;  
	private BufferedOutputStream outputStream;  
	  
	public AvcEncoder(String savePath) 
	{   
	    File f = new File(savePath);  
	    try {  
	        outputStream = new BufferedOutputStream(new FileOutputStream(f));  
	        Log.i("AvcEncoder", "outputStream initialized");  
	    } catch (Exception e){   
	        e.printStackTrace();  
	    }  
	  
	    mediaCodec = MediaCodec.createEncoderByType("video/avc");  
	    MediaFormat mediaFormat = MediaFormat.createVideoFormat("video/avc", 640, 480);  
	    mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 125000);  
	    mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 25);  
	    mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar);  
	    mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 5);  
	    mediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);  
	    mediaCodec.start();  
	}  
	  
	public void close() 
	{  
	    try {  
	        mediaCodec.stop();  
	        mediaCodec.release();  
	        outputStream.flush();  
	        outputStream.close();  
	    } catch (Exception e){   
	        e.printStackTrace();  
	    }  
	}  
	  
	// called from Camera.setPreviewCallbackWithBuffer(...) in other class  
	public void offerEncoder(byte[] input) 
	{  
	    try {  
	        ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();  
	        ByteBuffer[] outputBuffers = mediaCodec.getOutputBuffers();  
	        int inputBufferIndex = mediaCodec.dequeueInputBuffer(-1);  
	        if (inputBufferIndex >= 0) {  
	        	Log.e("zkzszd","at put buf"+input.length);
	            ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];  
	            inputBuffer.clear();  
	            inputBuffer.put(input);  
	            mediaCodec.queueInputBuffer(inputBufferIndex, 0, input.length, 0, 0);  
	        }  
	  
	        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();  
	        int outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo,0);  
	        while (outputBufferIndex >= 0) {  
	            ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];  
	            byte[] outData = new byte[bufferInfo.size];  
	            outputBuffer.get(outData);  
	            outputStream.write(outData, 0, outData.length);  
	            Log.i("AvcEncoder", outData.length + " bytes written");  
	  
	            mediaCodec.releaseOutputBuffer(outputBufferIndex, false);  
	            outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 0);  
	  
	        }  
	    } catch (Throwable t) 
	    {  
	        t.printStackTrace();  
	    }  
	}
}
