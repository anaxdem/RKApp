package com.rkapp;

import android.os.Handler;

public class JNICaller implements JNIListener{
	public static final String DATA_KEY = "callback_string";

	static {
		System.loadLibrary("rkapp");
	}

	private Handler mHandler;
	public JNIListener mDelegateListener;

	// public JNICaller() {}
	public JNICaller(JNIListener delegate) {
		this.mHandler = new Handler();
		mDelegateListener = delegate;
	}

	public native boolean initializeWorker();
	
	public native void finalizeWorker();

	public native boolean openSync();

	public native boolean turnLedRed();

	public native boolean turnLedGreen();
	
	public native void setFilePath(String path);


	@Override
	public void onFinalizeEnd(final boolean res) {
		mHandler.post(new Runnable() {
			public void run() {
				mDelegateListener.onFinalizeEnd(res);
			}
		});		
	}

	@Override
	public void onMassage(final String msg) {
		mHandler.post(new Runnable() {
			public void run() {
				mDelegateListener.onMassage(msg);
			}
		});
	}

	@Override
	public void onNewFrame() {
		mHandler.post(new Runnable() {
			public void run() {
				//TODO
			}
		});
		
	}

	@Override
	public void onMassageByte(char[] msg) {
		mDelegateListener.onMassageByte(msg);
		
	}
	
}
