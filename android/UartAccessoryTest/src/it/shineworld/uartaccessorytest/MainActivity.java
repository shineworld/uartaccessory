package it.shineworld.uartaccessorytest;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.view.Menu;
import android.widget.Toast;

public class MainActivity extends Activity {

	private AccessoryReceiver mAccessoryReceiver;
	private IntentFilter mAccessoryFilter;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mAccessoryReceiver = new AccessoryReceiver();
		mAccessoryFilter = new IntentFilter(Intent.ACTION_TIME_TICK);
		mAccessoryFilter.addAction(UsbManager.ACTION_USB_ACCESSORY_ATTACHED);
		mAccessoryFilter.addAction(UsbManager.ACTION_USB_ACCESSORY_DETACHED);
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
			Toast.makeText(context, action, Toast.LENGTH_LONG).show();
		}

	}

}
