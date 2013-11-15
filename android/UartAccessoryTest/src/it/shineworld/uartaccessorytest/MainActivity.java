package it.shineworld.uartaccessorytest;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

public class MainActivity extends Activity {
	private static final String ACTION_USB_PERMISSION = "it.shineworld.uartaccessorytest.USB_PERMISSION";
	private static final String TAG = "shineworld";

	private enum State {
		CLOSED, WAIT_PERMISSION, OPEN
	}

	private AccessoryReceiver mAccessoryReceiver;
	private PendingIntent mPermissionIntent;
	private IntentFilter mAccessoryFilter;
	
	private ParcelFileDescriptor mFileDescriptor;
	private FileInputStream mInputStream;
	private FileOutputStream mOutputStream;
	
	private UsbManager mUsbManager;
	private UsbAccessory mUsbAccessory;

	private State mState = State.CLOSED;

	private Button mButtonOpen;
	private Button mButtonClose;
	private Button mButtonSend;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mUsbManager = (UsbManager) getSystemService(Context.USB_SERVICE);

		mAccessoryReceiver = new AccessoryReceiver();
		mAccessoryFilter = new IntentFilter();
		mAccessoryFilter.addAction(ACTION_USB_PERMISSION);
		mAccessoryFilter.addAction(UsbManager.ACTION_USB_ACCESSORY_DETACHED);

		// recover view references
		mButtonOpen = (Button) findViewById(R.id.buttonOpen);
		mButtonClose = (Button) findViewById(R.id.buttonClose);
		mButtonSend = (Button) findViewById(R.id.buttonSend);

		// implement button click listener
		OnClickListener buttonClickListener = new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (v == mButtonOpen) {
					open();
				}
				if (v == mButtonClose) {
					close();
				}
				if (v == mButtonSend) {
					send();
				}
			}

		};

		// set click listeners
		mButtonOpen.setOnClickListener(buttonClickListener);
		mButtonClose.setOnClickListener(buttonClickListener);
		mButtonSend.setOnClickListener(buttonClickListener);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public void onResume() {
		super.onResume();
		registerReceiver(mAccessoryReceiver, mAccessoryFilter);
	}

	@Override
	public void onPause() {
		super.onPause();
		unregisterReceiver(mAccessoryReceiver);
	}

	public class AccessoryReceiver extends BroadcastReceiver {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if (UsbManager.ACTION_USB_ACCESSORY_DETACHED.equals(action)) {
				close();
			} else if (ACTION_USB_PERMISSION.equals(action)) {
				if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
					openStreams();
				} else {
					Toast.makeText(context, "Permission denied", Toast.LENGTH_LONG).show();
					Log.e(TAG, "Permission denied");
					setState(State.CLOSED);
				}
			}
		}

	}

	public synchronized void open() {
		if (mState != State.CLOSED) {
			return;
		}
		UsbAccessory[] mUsbAccessories = mUsbManager.getAccessoryList();
		mUsbAccessory = (mUsbAccessories == null ? null : mUsbAccessories[0]);
		if (mUsbAccessory != null) {
			if (mUsbManager.hasPermission(mUsbAccessory)) {
				openStreams();
			} else {
				mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);
				mUsbManager.requestPermission(mUsbAccessory, mPermissionIntent);
				setState(State.WAIT_PERMISSION);
			}
		} else {
			Toast.makeText(this, "No accessory found", Toast.LENGTH_LONG).show();
			Log.d(TAG, "No accessory found");
		}
	}

	public synchronized void close() {
		if (mState == State.OPEN) {
			closeStreams();
		} else if (mState == State.WAIT_PERMISSION) {
			mPermissionIntent.cancel();
		}
		setState(State.CLOSED);
	}
	
	public synchronized void send() {
		if (mState == State.OPEN) {
			Log.d(TAG, "sending data...");
			try {
				String dummy = "ciao";
				mOutputStream.write(dummy.getBytes());
				Log.d(TAG, "data sent");
			} catch (IOException e) {
				// TODO Auto-generated catch block
				Log.d(TAG, "sending data error...");
				e.printStackTrace();
			}
		}
	}

	private void setState(State state) {
		mState = state;
	}

	private void openStreams() {
		Toast.makeText(this, "open streams", Toast.LENGTH_LONG).show();
		try {
			mFileDescriptor = mUsbManager.openAccessory(mUsbAccessory);
			if (mFileDescriptor != null) {
				FileDescriptor fd = mFileDescriptor.getFileDescriptor();
				mInputStream = new FileInputStream(fd);
				mOutputStream = new FileOutputStream(fd);
				setState(State.OPEN);
			} else {
				throw new IOException("Failed to open file descriptor");
			}
		} catch (IOException e) {
			Toast.makeText(this, "Failed to open streams", Toast.LENGTH_LONG).show();
			Log.e(TAG, "Failed to open streams", e);
			setState(State.CLOSED);
		}
		
	}

	private void closeStreams() {
		Toast.makeText(this, "close streams", Toast.LENGTH_LONG).show();
		try {
			mFileDescriptor.close();
		} catch (IOException e) {
			Toast.makeText(this, "Failed to properly close accessory", Toast.LENGTH_LONG).show();
			Log.e(TAG, "Failed to properly close accessory", e);
		}
		
	}

}
