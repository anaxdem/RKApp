package com.rkapp;

public interface JNIListener {
	
	public void onCleanupEnd(boolean res);
	
	public void onMassage(String msg);
	
	public void onNewFrame();

}
