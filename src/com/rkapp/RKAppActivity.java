package com.rkapp;

import java.util.HashMap;
import java.util.Iterator;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class RKAppActivity extends Activity {
	 private static final String TAG = "RKAppActivity";
	 private static final String ACTION_USB_PERMISSION = "com.test.USB_PERMISSION";
	 
	 @Override
	    public void onCreate(Bundle savedInstanceState)
	    {
	        super.onCreate(savedInstanceState);

	        /* Create a TextView and set its content.
	         * the text is retrieved by calling a native
	         * function.
	         */
	        Button button = new Button(this);
	        button.setText("Sync usb");
	        button.setOnClickListener(new OnClickListener() {
	            
	            @Override
	            public void onClick(View v) {
	                UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);
	                HashMap<String, UsbDevice> deviceList = manager.getDeviceList();
	                Iterator<UsbDevice> deviceIterator  = deviceList.values().iterator();

	                while(deviceIterator.hasNext()) {
	                    UsbDevice device = deviceIterator.next();
	                    Log.d(TAG, "device ID: " + device.getDeviceId());
	                    Log.d(TAG, "device NAME: " + device.getDeviceName());
	                    Log.d(TAG, "device string: " + device.toString());

	                    if ( device.getProductId() == 688 ) {
	                    	Log.d(TAG, "requesting permission for productID 688");
	                    	PendingIntent mPermissionIntent = PendingIntent.getBroadcast(v.getContext(), 0, new Intent(ACTION_USB_PERMISSION), 0);
	                    	IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
	                    	registerReceiver(mUsbReceiver, filter);
	                    	
	                    	manager.requestPermission(device, mPermissionIntent);
	                    }
	                }
	            }
	        });
	        setContentView(button);
	    }

	    /* A native method that is implemented by the
	     * 'hello-jni' native library, which is packaged
	     * with this application.
	     */
	    public native String  stringFromJNI();
	    private native void openUsb(int fd);
	    

	    /* this is used to load the 'hello-jni' library on application
	     * startup. The library has already been unpacked into
	     * /data/data/com.example.HelloJni/lib/libhello-jni.so at
	     * installation time by the package manager.
	     */
	    static {
	        System.loadLibrary("rkapp");
	    }
	
	   private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {

	        public void onReceive(Context context, Intent intent) {
	            String action = intent.getAction();
	            if (ACTION_USB_PERMISSION.equals(action)) {
	                synchronized (this) {
	                    UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

	                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
	                        if(device != null) {
	                            UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);
	                            UsbDeviceConnection a = manager.openDevice(device);
	                            int fd = a.getFileDescriptor();
	                            Log.d(TAG, "Got permission to open device " + device.toString()+ " fd= " + fd);
	                            openUsb(fd);
	                            int count = device.getInterfaceCount();
	                            for (int i = 0; i < count; i++) {
	                                UsbInterface intf = device.getInterface(i);
	                                Log.d(TAG, "intf= " + intf.toString());
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


}
