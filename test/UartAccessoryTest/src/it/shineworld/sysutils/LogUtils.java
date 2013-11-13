/*
 * Copyright 2013 Silverio Diquigiovanni. All rights reserved.
 *  
 * 
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 * 
 *    1. Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright notice, this list
 *       of conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ARSHAN POURSOHI OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied.
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
