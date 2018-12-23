/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

package org.ngui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.BatteryManager;
import android.os.Build;
import android.os.Debug;
import android.telephony.TelephonyManager;
import java.util.List;
import java.util.Locale;
import android.app.ActivityManager;
import android.util.Log;

public class Android {

	private static String TAG = "Ngui";
	private static NGUIActivity activity = null;
	private static int battery_status = BatteryManager.BATTERY_STATUS_UNKNOWN;
	private static boolean is_ac_power_connected = false;
	private static NGUIActivity.PrivateAPI api = null;

	private static BroadcastReceiver receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if ( intent.getAction() == Intent.ACTION_BATTERY_CHANGED ) {
				Log.d(TAG, "ACTION_BATTERY_CHANGED");
				battery_status = intent.getIntExtra("status", BatteryManager.BATTERY_STATUS_UNKNOWN);
			} else if ( intent.getAction() == Intent.ACTION_POWER_CONNECTED ) {
				is_ac_power_connected = true;
				Log.d(TAG, "ACTION_POWER_CONNECTED");
			} else {
				Log.d(TAG, "ACTION_POWER_DISCONNECTED");
				is_ac_power_connected = false;
			}
		}
	};

	public static void initialize(NGUIActivity act, NGUIActivity.PrivateAPI api) {
		if ( !act.equals(activity) ) {
			activity = act;
			Android.api = api;
			activity.registerReceiver(receiver, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
			activity.registerReceiver(receiver, new IntentFilter(Intent.ACTION_POWER_CONNECTED));
			activity.registerReceiver(receiver, new IntentFilter(Intent.ACTION_POWER_DISCONNECTED));
		}
	}

	public static void uninitialize(NGUIActivity act) {
		if ( act.equals(activity) ) {
			activity.unregisterReceiver(receiver);
			activity = null;
			api = null;
		}
	}

	// -------------------- gui --------------------

	private static void ime_keyboard_open(final boolean clear, final int type, final int return_type) {
		activity.post(new Runnable() {
			public void run() { api.ime_keyboard_open(clear, type, return_type); }
		});
	}

	private static void ime_keyboard_can_backspace(final boolean can_backspace, final boolean can_delete) {
		activity.post(new Runnable() {
			public void run() { api.ime_keyboard_can_backspace(can_backspace, can_delete); }
		});
	}

	private static void ime_keyboard_close() {
		activity.post(new Runnable() {
			public void run() { api.ime_keyboard_close(); }
		});
	}

	private static void keep_screen(final boolean value) {
		activity.post(new Runnable() {
			public void run() {
				api.keep_screen(value);
			}
		});
	}

	private static int  get_status_bar_height() {
		return api.get_status_bar_height();
	}

	private static void set_visible_status_bar(final boolean visible) {
		activity.post(new Runnable() {
			public void run() {
				api.set_visible_status_bar(visible);
			}
		});
	}

	private static void set_status_bar_style(final int style) {
		activity.post(new Runnable() {
			public void run() {
				api.set_status_bar_style(style);
			}
		});
	}

	private static void request_fullscreen(final boolean fullscreen) {
		activity.post(new Runnable() {
			public void run() {
				api.request_fullscreen(fullscreen);
			}
		});
	}

	private static int  get_orientation() {
		return api.get_orientation();
	}

	private static void set_orientation(final int orientation) {
		activity.post(new Runnable() {
			public void run() {
				api.set_orientation(orientation);
			}
		});
	}

	private static float get_display_scale() {
		return api.get_display_scale();
	}

	private static boolean is_screen_on() {
		return api.is_screen_on();
	}

	private static void set_volume_up() {
		activity.post(new Runnable() {
			public void run() {
				api.set_volume_up();
			}
		});
	}

	private static void set_volume_down() {
		activity.post(new Runnable() {
			public void run() {
				api.set_volume_down();
			}
		});
	}

	private static void open_url(final String url) {
		activity.post(new Runnable() {
			public void run() {
				api.open_url(url);
			}
		});
	}

	private static void send_email(final String recipient,
																 final String subject,
																 final String cc, final String bcc, final String body) {
		activity.post(new Runnable() {
			public void run() {
				api.send_email(recipient, subject, cc, bcc, body);
			}
		});
	}

	// -------------------- util --------------------

	private static String start_path() {
		return activity.startPath();
	}
	
	private static String package_code_path() {
		return activity.getPackageCodePath();
	}

	private static String files_dir_path() {
		return activity.getFilesDir().getPath();
	}

	private static String cache_dir_path() {
		return activity.getCacheDir().getPath();
	}

	private static String version() {
		return Build.VERSION.RELEASE;
	}

	private static String brand() {
		return Build.BRAND;
	}

	private static String subsystem() {
		return Build.MODEL;
	}

	private static int network_status() {
		ConnectivityManager m = (ConnectivityManager)
						activity.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo info = m.getActiveNetworkInfo();

		if ( info != null && info.isAvailable() ) {
			int type = info.getType();
			if (type == ConnectivityManager.TYPE_WIFI) {
				return 2;
			} else if (type == ConnectivityManager.TYPE_MOBILE) {

				int sub_type = info.getSubtype();
				TelephonyManager tm = (TelephonyManager)
								activity.getSystemService(Context.TELEPHONY_SERVICE);

				if (sub_type == TelephonyManager.NETWORK_TYPE_LTE) {
					return 5; // 4G
				} else if (sub_type == TelephonyManager.NETWORK_TYPE_UMTS ||
								sub_type == TelephonyManager.NETWORK_TYPE_HSDPA ||
								sub_type == TelephonyManager.NETWORK_TYPE_EVDO_0) {
					return 5; // 3G
				} else if (sub_type == TelephonyManager.NETWORK_TYPE_GPRS ||
								sub_type == TelephonyManager.NETWORK_TYPE_EDGE ||
								sub_type == TelephonyManager.NETWORK_TYPE_CDMA) {
					return 4; // 2G
				} else {
					return 4; // 2G
				}
			}
		}
		return 0;
	}

	private static boolean is_ac_power() {
		if ( is_battery() ) {
			switch ( battery_status ) {
				case BatteryManager.BATTERY_STATUS_CHARGING: return true;
				case BatteryManager.BATTERY_STATUS_DISCHARGING: return false;
				case BatteryManager.BATTERY_STATUS_NOT_CHARGING:
				case BatteryManager.BATTERY_STATUS_FULL:
				case BatteryManager.BATTERY_STATUS_UNKNOWN: break;
			}
			return is_ac_power_connected;
		} else { // no battery
			return true;
		}
	}

	private static boolean is_battery() {
		BatteryManager bm =(BatteryManager)activity.getSystemService(Context.BATTERY_SERVICE);
		int capacity = bm.getIntProperty(BatteryManager.BATTERY_PROPERTY_CHARGE_COUNTER);
		return capacity > 0;
	}

	private static float battery_level() {
		if ( is_battery() ) {
			BatteryManager bm = (BatteryManager) activity.getSystemService(Context.BATTERY_SERVICE);
			float level = bm.getIntProperty(BatteryManager.BATTERY_PROPERTY_CAPACITY) / 100F;
			return level;
		} else {
			return 0;
		}
	}

	private static String language() {
		return Locale.getDefault().toString();
	}

	private static ActivityManager.MemoryInfo memory_info() {
		ActivityManager am = (ActivityManager)activity.getSystemService(Context.ACTIVITY_SERVICE);
		ActivityManager.MemoryInfo info = new ActivityManager.MemoryInfo();
		am.getMemoryInfo(info);
		return info;
	}

	private static long available_memory() {
		return memory_info().availMem;
	}

	private static long memory() {
		return memory_info().totalMem;
	}

	private static long used_memory() {
		ActivityManager am = (ActivityManager)activity.getSystemService(Context.ACTIVITY_SERVICE);
		List<ActivityManager.RunningAppProcessInfo> appProcessList = am.getRunningAppProcesses();
		int pid = android.os.Process.myPid();
		for (ActivityManager.RunningAppProcessInfo appProcessInfo : appProcessList) {
			if ( appProcessInfo.pid == pid ) {
				Debug.MemoryInfo[] memoryInfo = am.getProcessMemoryInfo(new int[] { pid });
				return memoryInfo[0].dalvikPrivateDirty * 1024;
			}
		}
		return 0;
	}

}
