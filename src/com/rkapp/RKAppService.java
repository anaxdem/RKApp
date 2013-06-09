package com.rkapp;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class RKAppService extends Service {
	// Debugging
	private static final String TAG = "RKAppService";
	private static final int NOTIFICATION_ID = 999;

	private NotificationManager mNotificationManager;

	
	@Override
	public IBinder onBind(Intent intent) {
		
		return null;
	}
	
	
	private void showNotification(String notify) {
		Log.d(TAG, "Notification");
		mNotificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);


		Notification notification = new Notification(R.drawable.ic_launcher, notify,
				System.currentTimeMillis());
//		// The PendingIntent to launch our activity if the user selects this
//		// notification
//		PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
//				new Intent(this, BlurossoMainActivity.class), 0);
		// Set the info for the views that show in the notification panel.
		notification.setLatestEventInfo(this, getText(R.string.service_label),
				notify, null);

		// Send the notification.
		mNotificationManager.notify(NOTIFICATION_ID, notification);
	}
}
