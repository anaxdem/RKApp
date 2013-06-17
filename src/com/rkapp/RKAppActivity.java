package com.rkapp;

import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.widget.ToggleButton;

public class RKAppActivity extends Activity {
		
	 private JNICaller mJNICaller;
	 public EditText mLog; 
	 public Button mSyncUsbButton;
	 private Button mCloseDevice;
	 ToggleButton mLed ;
	 String mPath;
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
	        
	        mCloseDevice = (Button) findViewById(R.id.btn_stop_device);
	        mCloseDevice.setOnClickListener(new OnClickListener() {
				
				@Override
				public void onClick(View v) {
					mJNICaller.finalizeWorker();
					
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
	        if(!systemInit() ) 
	        	RKAppLog( "Cant initialize native part");
	        else {
	        	//TODO something to exit or Oops! buttons block
	        }	
	        
	        mPath="";
			try {
				mPath = createFile();
			} catch (IOException e) {
				e.printStackTrace();
			}
			
			RKAppLog("File to write locatied in : " + mPath);
	        mJNICaller.setFilePath(mPath);
	    }
	 
	  
		protected boolean systemInit(){			
			mJNICaller = new JNICaller(mJNIListener) ;			
			return mJNICaller.initializeWorker();
		}
	 
//		public void onBackPressed() {				
//			
//		};
			

	    public void getStringFromJNI(){
	    	Toast.makeText(RKAppActivity.this, "text", Toast.LENGTH_LONG).show();
	    };
   
	    
		private void startSync() {
			 Process process;
 			try {
 				process = Runtime.getRuntime().exec("su");
 				DataOutputStream os = new DataOutputStream(process.getOutputStream());
 				os.writeBytes("chown root:root /dev/bus/usb/002/ \n");
 				os.writeBytes("chmod -R 777 /dev/bus/usb/002/ \n");
 				os.writeBytes("chown root:root "+ mPath+" \n");
 				os.writeBytes("chmod -R 777 "+ mPath + "\n");
 				os.writeBytes("exit\n");
 				os.flush();
 				os.close();
 				process.waitFor();
 			} catch (IOException e1) {
 				RKAppLog( "Chmod error cause IOException");
 				e1.printStackTrace();
 			} catch (InterruptedException e) {
 				RKAppLog("Chmod error cause InterruptedException"); 
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
				RKAppLog(msg );				
			}
			
			@Override
			public void onMassageByte(char[] msg) {				
				RKAppLog("new frame");				
			}
			
			
			@Override
			public void onFinalizeEnd(boolean res) {
			 if (res){
				// finish();
			 }
				
			}
		};
		
		
		private void RKAppLog(String msg){
			mLog.getText().append(msg + "\n");	
		}
		
		String createFile() throws IOException{
			if (!Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)){
			    //handle case of no SDCARD present
				Toast.makeText(RKAppActivity.this, "No sdcard present", Toast.LENGTH_LONG).show();
				return "";
			} else {
			    File file = new File(Environment.getExternalStorageDirectory().getAbsoluteFile()
			        			         +File.separator
			         +"testfile.txt"); //file name
			    //Удаляем старые записи
			    if(file.exists()){
			    	file.delete();
			    }
			    
			    file.createNewFile();
			    
			    return file.getAbsolutePath();
			}
		}

}
