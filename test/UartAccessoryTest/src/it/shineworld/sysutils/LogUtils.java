/*
 * Copyright (C) 2013 AChep@xda <artemchep@gmail.com>
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

package it.shineworld.sysutils;

import android.text.TextUtils;
import android.util.Log;

public class LogUtils {
	private static final String TAG = "shineworld";
	private static final boolean LOG = true;

	public static void track() {
		if (LOG)
			Log.d(TAG, getLocation());
	}

	public static void v(String msg) {
		if (LOG)
			Log.v(TAG, getLocation() + msg);
	}

	public static void d(String msg) {
		if (LOG)
			Log.d(TAG, getLocation() + msg);
	}

	public static void i(String msg) {
		if (LOG)
			Log.i(TAG, getLocation() + msg);
	}

	public static void e(String msg) {
		if (LOG)
			Log.e(TAG, getLocation() + msg);
	}

	public static void w(String msg) {
		if (LOG)
			Log.w(TAG, getLocation() + msg);
	}

	private static String getLocation() {
		final String className = LogUtils.class.getName();
		final StackTraceElement[] traces = Thread.currentThread().getStackTrace();
		boolean found = false;

		for (StackTraceElement trace : traces) {
			try {
				if (found) {
					if (!trace.getClassName().startsWith(className)) {
						Class<?> clazz = Class.forName(trace.getClassName());
						return "[" + getClassName(clazz) + ":" + trace.getMethodName() + ":" + trace.getLineNumber() + "]: ";
					}
				} else if (trace.getClassName().startsWith(className)) {
					found = true;
				}
			} catch (ClassNotFoundException e) {
			}
		}

		return "[]: ";
	}

	private static String getClassName(Class<?> clazz) {
		if (clazz != null) {
			if (!TextUtils.isEmpty(clazz.getSimpleName())) {
				return clazz.getSimpleName();
			}

			return getClassName(clazz.getEnclosingClass());
		}

		return "";
	}
}
