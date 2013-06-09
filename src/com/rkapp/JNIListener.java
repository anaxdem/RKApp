package com.rkapp;

public interface JNIListener {
	
	public void onFinalizeEnd(boolean res);
	
	public void onMassage(String msg);
	
	public void onNewFrame();

}
