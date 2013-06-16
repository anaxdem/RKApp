package com.rkapp;

import java.io.DataOutputStream;
import java.io.IOException;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
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
   
	    
//	    private static final String ACTION_USB_PERMISSION =
//	    	    "com.android.example.USB_PERMISSION";
//	    	private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
//	    	 
//	    	    public void onReceive(Context context, Intent intent) {
//	    	        String action = intent.getAction();
//	    	        if (ACTION_USB_PERMISSION.equals(action)) {
//	    	            synchronized (this) {
//	    	                UsbAccessory accessory = (UsbAccessory) intent.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
//
//	    	                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
//	    	                    if(accessory != null){
//	    	                    	if(mJNICaller.openSync()){
//	    	            				mLed.setChecked(true);
//	    	            			}
//	    	                    }
//	    	                }
//	    	                else {
//	    	                    Log.d(TAG, "permission denied for accessory " + accessory);
//	    	                }
//	    	            }
//	    	        }
//	    	    }
//	    	};
//		boolean mPermissionRequestPending = false;
	
	    

	    private static final String ACTION_USB_PERMISSION = "com.test.USB_PERMISSION";

	    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {

	        public void onReceive(Context context, Intent intent) {
	            String action = intent.getAction();
	            if (ACTION_USB_PERMISSION.equals(action)) {
	                synchronized (this) {
	                    UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

	                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
	                        if(device != null) {
//	                            UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);
//	                            UsbDeviceConnection a = manager.openDevice(device);
//	                            int fd = a.getFileDescriptor();
//	                            Log.d(TAG, "Got permission to open device " + device.toString()+ " fd= " + fd);
//	                				
//	                            int count = device.getInterfaceCount();
//	                            for (int i = 0; i < count; i++) {
//	                                UsbInterface intf = device.getInterface(i);
//	                                Log.d(TAG, "intf= " + intf.toString());
//	                            }
	                            
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
		                				Toast.makeText(RKAppActivity.this, "Chmod error 1", Toast.LENGTH_LONG).show();
		                				e1.printStackTrace();
		                			} catch (InterruptedException e) {
		                				Toast.makeText(RKAppActivity.this, "Chmod error 2", Toast.LENGTH_LONG).show();
		                				e.printStackTrace();
		                			}
		                			
		                			if(mJNICaller.openSync()){
		                				mLed.setChecked(true);
		                			}
	                       }
	                    } 
	                    else {
	                        Log.d(TAG, "permission denied for device " + device);
	                    }
	                }
	            }
	        }
	    };
		private void startSync() {
//			UsbManager manager = UsbManager.getInstance(this);
//			UsbAccessory[] accessories = manager.getAccessoryList();
//			UsbAccessory mAccessory = (accessories == null ? null : accessories[0]);
//			PendingIntent mPermissionIntent = PendingIntent.getBroadcast(this, 0,
//					new Intent(ACTION_USB_PERMISSION), 0);
//			if (mAccessory != null) {
//				if (manager.hasPermission(mAccessory)) {
//					if (mJNICaller.openSync()) {
//						mLed.setChecked(true);
//					}
//				} else {
//					synchronized (mUsbReceiver) {
//						if (!mPermissionRequestPending) {
//							IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
//							registerReceiver(mUsbReceiver, filter);
//							manager.requestPermission(mAccessory, mPermissionIntent);
//							mPermissionRequestPending = true;
//						}
//					}
//				}
//			} else {
//				Log.d(TAG, "mAccessory is null");
//			}
			
			
//			 UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);
//             HashMap<String, UsbDevice> deviceList = manager.getDeviceList();
//             Iterator<UsbDevice> deviceIterator  = deviceList.values().iterator();
//
//             while(deviceIterator.hasNext()) {
//                 UsbDevice device = deviceIterator.next();
//                 Log.d(TAG, "device ID: " + device.getDeviceId());
//                 Log.d(TAG, "device NAME: " + device.getDeviceName());
//                 Log.d(TAG, "device string: " + device.toString());
//
//                 if ( device.getProductId() == 688 ) {
//                 	Log.d(TAG, "requesting permission for productID 688");
//                 	PendingIntent mPermissionIntent = PendingIntent.getBroadcast(RKAppActivity.this, 0, new Intent(ACTION_USB_PERMISSION), 0);
//                 	IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
//                 	registerReceiver(mUsbReceiver, filter);
//                 	
//                 	manager.requestPermission(device, mPermissionIntent);
//                 }
//             }
			 Process process;
 			try {
 				process = Runtime.getRuntime().exec("su");
 				DataOutputStream os = new DataOutputStream(process.getOutputStream());
 				os.writeBytes("chown root:root /dev/bus/usb/002/ \n");
 				os.writeBytes("chmod -R 777 /dev/bus/usb/002/ \n");
 				os.writeBytes("exit\n");
 				os.flush();
 				os.close();
 				process.waitFor();
 			} catch (IOException e1) {
 				Toast.makeText(RKAppActivity.this, "Chmod error 1", Toast.LENGTH_LONG).show();
 				e1.printStackTrace();
 			} catch (InterruptedException e) {
 				Toast.makeText(RKAppActivity.this, "Chmod error 2", Toast.LENGTH_LONG).show();
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
			public void onMassageByte(char[] msg) {				
				mLog.getText().append("new frame" + "\n");				
			}
			
			
			@Override
			public void onFinalizeEnd(boolean res) {
			 if (res){
				 finish();
			 }
				
			}
		};

}
