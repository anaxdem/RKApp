package com.rkapp;

import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;

import org.apache.http.TokenIterator;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;
import android.widget.ToggleButton;

public class RKAppActivity extends Activity {

	private JNICaller mJNICaller;
	public EditText mLog;
	public Button mSyncUsbButton;
	private Button mCloseDeviceButton, mSetWritableButton;
	private ToggleButton mLedToggleButton;
	private ToggleButton mWriteCheckBox;
	private Button mTiltUpButton, mTiltDownButton;
	private EditText mSizeOfWriting;

	String mPath;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mLog = (EditText) findViewById(R.id.log);

		mSyncUsbButton = (Button) findViewById(R.id.btn_sync_device);
		mSyncUsbButton.requestFocus();
		mSyncUsbButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				startSync();
			}
		});

		mCloseDeviceButton = (Button) findViewById(R.id.btn_stop_device);
		mCloseDeviceButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				mJNICaller.finalizeWorker();

			}
		});

		mLedToggleButton = (ToggleButton) findViewById(R.id.tglLed);
		mLedToggleButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (mLedToggleButton.isChecked())
					mJNICaller.turnLedRed();
				else
					mJNICaller.turnLedGreen();

			}
		});

		mWriteCheckBox = (ToggleButton) findViewById(R.id.chb_writable);
		mWriteCheckBox.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (mWriteCheckBox.isChecked()) {
					mJNICaller.setFileWritable(1);
				} else {
					mJNICaller.setFileWritable(0);
				}

			}
		});

		mTiltUpButton = (Button) findViewById(R.id.btn_tilt_up);
		mTiltUpButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				mJNICaller.tiltUP();

			}
		});

		mTiltDownButton = (Button) findViewById(R.id.btn_tilt_down);
		mTiltDownButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				mJNICaller.titlDown();
			}
		});

		mSizeOfWriting = (EditText) findViewById(R.id.edit_write_buffer);
		mSizeOfWriting.clearFocus();
		mSyncUsbButton.setFocusableInTouchMode(true);
		mSyncUsbButton.requestFocus();
		mSetWritableButton = (Button) findViewById(R.id.btn_set_writable);
		mSetWritableButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				String sizeS = mSizeOfWriting.getText().toString();
				if (sizeS != "") {
					int size = Integer.parseInt(sizeS);
					mJNICaller.setSizeToWrite(size);

				}

			}
		});

		// Start with kinect
		if (!systemInit())
			RKAppLog("Cant initialize native part");
		else {
			// TODO something to exit or Oops! buttons block
		}

		mPath = "";
		try {
			mPath = createFile();
		} catch (IOException e) {
			e.printStackTrace();
		}

		RKAppLog("File to write locatied in : " + mPath);
		mJNICaller.setFilePath(mPath);
	}

	protected boolean systemInit() {
		mJNICaller = new JNICaller(mJNIListener);
		return mJNICaller.initializeWorker();
	}

	// public void onBackPressed() {
	//
	// };

	public void getStringFromJNI() {
		Toast.makeText(RKAppActivity.this, "text", Toast.LENGTH_LONG).show();
	};

	private void startSync() {
		Process process;
		try {
			process = Runtime.getRuntime().exec("su");
			DataOutputStream os = new DataOutputStream(
					process.getOutputStream());
			os.writeBytes("chown root:root /dev/bus/usb/002/ \n");
			os.writeBytes("chmod -R 777 /dev/bus/usb/002/ \n");
			os.writeBytes("chown root:root " + mPath + " \n");
			os.writeBytes("chmod -R 777 " + mPath + "\n");
			os.writeBytes("exit\n");
			os.flush();
			os.close();
			process.waitFor();
		} catch (IOException e1) {
			RKAppLog("Chmod error cause IOException");
			e1.printStackTrace();
		} catch (InterruptedException e) {
			RKAppLog("Chmod error cause InterruptedException");
			e.printStackTrace();
		}

		if (mJNICaller.openSync()) {
			mLedToggleButton.setChecked(true);
		}

	}

	private JNIListener mJNIListener = new JNIListener() {

		@Override
		public void onNewFrame() {

		}

		@Override
		public void onMassage(String msg) {
			RKAppLog(msg);
		}

		@Override
		public void onMassageByte(char[] msg) {
			RKAppLog("new frame");
		}

		@Override
		public void onFinalizeEnd(boolean res) {
			if (res) {
				// finish();
			}

		}
	};

	private void RKAppLog(String msg) {
		mLog.getText().append(msg + "\n");
	}

	String createFile() throws IOException {
		if (!Environment.getExternalStorageState().equals(
				Environment.MEDIA_MOUNTED)) {
			// handle case of no SDCARD present
			Toast.makeText(RKAppActivity.this, "No sdcard present",
					Toast.LENGTH_LONG).show();
			return "";
		} else {
			File file = new File(Environment.getExternalStorageDirectory()
					.getAbsoluteFile() + File.separator + "testfile.txt"); // file
																			// name
			// Удаляем старые записи
			if (file.exists()) {
				file.delete();
			}

			file.createNewFile();

			return file.getAbsolutePath();
		}
	}

}
