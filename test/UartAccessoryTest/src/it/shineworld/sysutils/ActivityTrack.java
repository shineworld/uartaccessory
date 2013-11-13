package it.shineworld.sysutils;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;

public class ActivityTrack extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		LogUtils.track();
		super.onCreate(savedInstanceState);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		LogUtils.track();
		return true;
	}

	@Override
	protected void onDestroy() {
		LogUtils.track();
		super.onDestroy();
	}

	@Override
	protected void onPause() {
		LogUtils.track();
		super.onPause();
	}

	@Override
	protected void onRestart() {
		LogUtils.track();
		super.onRestart();
	}

	@Override
	protected void onRestoreInstanceState(Bundle savedInstanceState) {
		LogUtils.track();
		super.onRestoreInstanceState(savedInstanceState);
	}

	@Override
	protected void onResume() {
		LogUtils.track();
		super.onResume();
	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		LogUtils.track();
		super.onSaveInstanceState(outState);
	}

	@Override
	protected void onStart() {
		LogUtils.track();
		super.onStart();
	}

	@Override
	protected void onStop() {
		LogUtils.track();
		super.onStop();
	}

}
