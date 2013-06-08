package com.rkapp;

import android.os.Handler;
import android.view.KeyEvent;

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

	public native boolean init();

	public native boolean keyEvent(int action, int unicodeChar, int keyCode,
			KeyEvent event);

	public native void cleanup();

	public native boolean openSync();

	public native boolean turnLedRed();

	public native boolean turnLedGreen();


	@Override
	public void onCleanupEnd(final boolean res) {
		mHandler.post(new Runnable() {
			public void run() {
				mDelegateListener.onCleanupEnd(res);
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
	
}
