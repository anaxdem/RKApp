package com.rkapp;

import java.io.DataOutputStream;
import java.io.IOException;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.widget.ToggleButton;

public class RKAppActivity extends Activity {
	 private static final String TAG = "RKAppActivity";
//	 private static final String ACTION_USB_PERMISSION = "com.test.USB_PERMISSION";
	 private  Handler mJNICallback = new Handler(){
			@Override
			public void handleMessage(Message msg) {
				
				Bundle b = msg.getData();
				mLog.getText().append(b.getString(JNICaller.DATA_KEY));
				mLog.getText().append(b.getString("\n"));
				//callVoidText.setText(b.getString("callback_string"));
			
			}
		};
		
	 private JNICaller mJNICaller;
	 public EditText mLog; 
	 public Button mSyncUsbButton;
	 ToggleButton mLed ;
	 @Override
	    public void onCreate(Bundle savedInstanceState)
	    {
	        super.onCreate(savedInstanceState);
	        setContentView(R.layout.activity_main);
	        
	        mLog = (EditText) findViewById(R.id.edit);
	        
	        
	        mSyncUsbButton = (Button)findViewById(R.id.btn_sync_device);
	        mSyncUsbButton.setOnClickListener(new OnClickListener() {
	            
	            @Override
	            public void onClick(View v) {
	            	startSync();  	
	            }
	        });     
	        
	        mLed = (ToggleButton) findViewById(R.id.tglLed);
	        mLed.setOnClickListener(new OnClickListener() {
				
				@Override
				public void onClick(View v) {
					if(mLed.isChecked()) mJNICaller.turnLedRed();
					else mJNICaller.turnLedGreen();
					
				}
			});
	        	        
	        //Start with kinect
	        if(!systemInit()) 
	        	Toast.makeText(RKAppActivity.this, "Cant initialize native part", Toast.LENGTH_LONG).show();
	        else {
	        	//TODO something to exit or Oops! buttons block
	        }	        
	    }
	 
	  
		protected boolean systemInit(){			
			mJNICaller = new JNICaller(mJNIListener) ;			
			return mJNICaller.initializeWorker();
		}
	 
		public void onBackPressed() {				
				mJNICaller.finalizeWorker();
		};
			

	 
	    
	    public void getStringFromJNI(){
	    	Toast.makeText(RKAppActivity.this, "text", Toast.LENGTH_LONG).show();
	    };
   
		private void startSync(){
			
			
			Process process;
			try {
				process = Runtime.getRuntime().exec("su");
				DataOutputStream os = new DataOutputStream(process.getOutputStream());
				os.writeBytes("chown root:root /dev/bus/usb/001/ \n");
				os.writeBytes("chmod -R 777 /dev/bus/usb/001/ \n");
				os.writeBytes("exit\n");
				os.flush();
				os.close();
				process.waitFor();
			} catch (IOException e1) {
				e1.printStackTrace();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			if(mJNICaller.openSync()){
				mLed.setChecked(true);
			}			
		}
		
		private JNIListener mJNIListener = new JNIListener() {
			
			@Override
			public void onNewFrame() {
				
			}
			
			@Override
			public void onMassage(String msg) {				
				mLog.getText().append(msg + "\n");				
			}
			
			@Override
			public void onFinalizeEnd(boolean res) {
			 if (res){
				 finish();
			 }
				
			}
		};

}
