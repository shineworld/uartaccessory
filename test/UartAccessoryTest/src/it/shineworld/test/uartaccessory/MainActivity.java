package it.shineworld.test.uartaccessory;

import java.util.HashMap;

import it.shineworld.sysutils.LogUtils;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends Activity {
	private TextView mTextAOA;
	private Button mButtonCheckAOA;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		// recover view references
		mTextAOA = (TextView) findViewById(R.id.textAOA);
		mButtonCheckAOA = (Button) findViewById(R.id.buttonCheckAOA);

		// implement button click listener
		OnClickListener buttonClickListener = new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (v == mButtonCheckAOA) {

					UsbManager usbManager = (UsbManager) getSystemService(USB_SERVICE);
					if (usbManager != null) {
						UsbAccessory[] deviceList = usbManager.getAccessoryList();
						if (deviceList != null) {
							mTextAOA.setText(String.valueOf(deviceList.length));
						} else
							mTextAOA.setText("deviceList null");
					} else
						mTextAOA.setText("null");
					
					

				}
			}

		};

		// set button click listener
		mButtonCheckAOA.setOnClickListener(buttonClickListener);
	}

}
