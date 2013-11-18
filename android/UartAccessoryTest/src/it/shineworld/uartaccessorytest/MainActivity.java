/*
 * Copyright (C) 2013 Silverio Diquigiovanni <shineworld.software@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
import android.os.Handler;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.text.Layout;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
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
	private EditText mEditTextToSend;
	private TextView mTextReceivedText;

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
		mEditTextToSend = (EditText) findViewById(R.id.editTextToSend);
		mTextReceivedText = (TextView) findViewById(R.id.textReceivedText);

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

		// set movement method for received text
		mTextReceivedText.setMovementMethod(new ScrollingMovementMethod());
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
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
				accessoryReadThread.start();
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

	public synchronized void send() {
		if (mState == State.OPEN) {
			Log.d(TAG, "sending data start");
			try {
				if (!mEditTextToSend.getText().toString().equals(""))
					mOutputStream.write(mEditTextToSend.getText().toString().getBytes());
				Log.d(TAG, "sending data OK");
			} catch (IOException e) {
				Log.d(TAG, "sending data exception");
				e.printStackTrace();
			}
		}
	}

	private static final int ACCESSORY_MODE_BUFFER_SIZE = 16384;
	private static final int USB_HIGH_SPEED_BUFFER_SIZE = 512;
	private static final int USB_FULL_SPEED_BUFFER_SIZE = 64;

	Thread accessoryReadThread = new Thread() {

		@Override
		public void run() {
			byte[] buffer = new byte[ACCESSORY_MODE_BUFFER_SIZE];

			Log.d(TAG, "accessory read thread start");
			while (true) {
				try {
					int ret = mInputStream.read(buffer);
					if (ret < 0)
						break;
					if (ret > 0) {
						Message m = Message.obtain(mHandler, MESSAGE_READ_DATA);
						m.obj = new String(buffer, 0, ret);
						mHandler.sendMessage(m);
					}
				} catch (IOException e) {
					Log.d(TAG, "accessory read thread exception");
					break;
				}
			}
			Log.d(TAG, "accessory read thread stop");
		}

	};

	private static final int MESSAGE_READ_DATA = 1;

	private final Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
				case MESSAGE_READ_DATA:
					appendTextAndScroll(mTextReceivedText, (String) msg.obj);
					break;
			}
		}

	};

	private void appendTextAndScroll(TextView view, String text) {
		if (view != null) {
			view.append(text);
			final Layout layout = view.getLayout();
			if (layout != null) {
				int scrollDelta = layout.getLineBottom(view.getLineCount() - 1) - view.getScrollY() - view.getHeight();
				if (scrollDelta > 0)
					view.scrollBy(0, scrollDelta);
			}
		}
	}

}
