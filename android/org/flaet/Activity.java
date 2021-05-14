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

package org.flare;

import android.app.KeyguardManager;
import android.app.NativeActivity;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.Color;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Looper;
import android.os.PowerManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.os.Handler;
import android.view.Display;
import android.view.Surface;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.content.pm.ApplicationInfo;

public class Activity extends NativeActivity implements View.OnSystemUiVisibilityChangeListener {

	private static String TAG = "Flare";
	private IMEHelper _ime = null;
	private Handler _handler = null;
	private PowerManager pm = null;
	private AudioManager am = null;
	private static boolean visible_status_bar = true;
	private static int status_bar_style = 0;
	private static boolean is_fullscreen = false;
	private static int screen_orientation = ActivityInfo.SCREEN_ORIENTATION_USER;
	private static boolean virtual_navigation_device;
	private static native void onStatucBarVisibleChange();

	private void set_system_ui_flags() {
		Window window = getWindow();

		int flags = View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN; // 全屏布局(这在全屏gl表面绘制下似乎没什么意义)

		flags |= View.SYSTEM_UI_FLAG_LAYOUT_STABLE;

		if ( is_fullscreen || !visible_status_bar ) {
			flags |= View.SYSTEM_UI_FLAG_FULLSCREEN; // 全屏（其实只是隐藏了状态栏）
			window.addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN); // 全屏（其实只是隐藏了状态栏）
		} else {
			window.clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
		}

		if ( is_fullscreen ) { // 全屏状态隐藏系统虚拟导航键
			flags |= View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;   // 隐藏导航栏
			flags |= View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION; // 布局位于导航栏下方
			flags |= View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY; // 触控屏幕不显示虚拟导航键
			flags |= View.SYSTEM_UI_FLAG_IMMERSIVE;
			// flags |= View.SYSTEM_UI_FLAG_LOW_PROFILE;
		}

		if (status_bar_style == 1) { // BLACK
			flags |= View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR; // 深色显示状态栏文字颜色
		}

		window.getDecorView().setSystemUiVisibility(flags);

		// 兼容7.0下系统,让状态栏为透明并融入用户界面

		window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS); // 自定义系统Bar背景颜色
		window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS); // 清空透明状态栏
		window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
		window.setStatusBarColor(Color.TRANSPARENT); // 设置自定义透明背景颜色
		window.setNavigationBarColor(Color.TRANSPARENT);
	}

	private boolean is_virtual_navigation_device() {
		Display display = getWindowManager().getDefaultDisplay();
		DisplayMetrics dm_0 = new DisplayMetrics();
		DisplayMetrics dm_1 = new DisplayMetrics();
		display.getRealMetrics(dm_0);
		display.getMetrics(dm_1);
		return dm_0.widthPixels != dm_1.widthPixels || dm_0.heightPixels != dm_1.heightPixels;
	}

	public void post(Runnable r) {
		_handler.post(r);
	}

	@Override
	public void onSystemUiVisibilityChange(int visibility) {
		set_system_ui_flags();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		virtual_navigation_device = is_virtual_navigation_device();
		_handler = new Handler(Looper.getMainLooper());
		pm = (PowerManager)getSystemService(POWER_SERVICE);
		am = (AudioManager) getSystemService(AUDIO_SERVICE);
		Android.initialize(this, new PrivateAPI(this));
		set_system_ui_flags();
		getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(this);
		setRequestedOrientation(screen_orientation);
		super.onCreate(savedInstanceState);
	}

	@Override
	protected void onDestroy() {
		Android.uninitialize(this);
		super.onDestroy();
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		// android:configChanges="keyboardHidden|orientation|screenSize"
		Log.d(TAG, "onConfigurationChanged");
		super.onConfigurationChanged(newConfig);
	}
	
	protected String startCommand() {
		return "";
	}

	protected boolean isDebugger() {
		ApplicationInfo info = getApplicationInfo();
		return (info.flags & ApplicationInfo.FLAG_DEBUGGABLE) != 0;
	}

	static public class PrivateAPI {

		private FlareActivity host = null;

		private PrivateAPI(FlareActivity host) {
			this.host = host;
		}

		public void ime_keyboard_open(boolean clear, int type, int return_type) {
			Log.d(TAG, "ime_keyboard_open");
			if ( host._ime == null ) {
				host._ime = new IMEHelper(host);
				((FrameLayout) host.findViewById(android.R.id.content)).addView(host._ime);
			}
			if ( clear ) {
				host._ime.clear();
			}
			host._ime.set_keyboard_return_type(return_type);
			host._ime.set_keyboard_type(type);
			host._ime.open();
		}

		public void ime_keyboard_can_backspace(boolean can_backspace, boolean can_delete) {
			Log.d(TAG, "ime_keyboard_can_backspace");
			if ( host._ime != null ) {
				host._ime.set_can_backspace(can_backspace, can_delete);
			}
		}

		public void ime_keyboard_close() {
			Log.d(TAG, "ime_keyboard_close");
			if ( host._ime != null ) {
				host._ime.close();
			}
		}

		public void keep_screen(boolean value) {
			if (value) {
				host.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			} else {
				host.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			}
		}

		public int  get_status_bar_height() {
			int result = 0;
			if ( visible_status_bar && !is_fullscreen ) {
				int resourceId = host.getResources().getIdentifier("status_bar_height", "dimen", "android");
				if (resourceId > 0) {
					result = host.getResources().getDimensionPixelSize(resourceId);
				}
			}
			return result;
		}

		public void set_visible_status_bar(boolean visible) {
			if ( visible_status_bar != visible ) {
				int rawh = get_status_bar_height();
				visible_status_bar = visible;
				host.set_system_ui_flags();
				int h = get_status_bar_height();
				if (rawh != h) {
					onStatucBarVisibleChange();
				}
			}
		}

		public void set_status_bar_style(int style) {
			if ( status_bar_style != style ) {
				status_bar_style = style;
				host.set_system_ui_flags();
			}
		}

		public void request_fullscreen(boolean fullscreen) {
			if ( is_fullscreen != fullscreen ) {
				int rawh = virtual_navigation_device ? 0 : get_status_bar_height();
				is_fullscreen = fullscreen;
				host.set_system_ui_flags();
				int h = get_status_bar_height();
				if (!virtual_navigation_device) {
					if (rawh != h) {
						onStatucBarVisibleChange();
					}
				}
			}
		}

		public int  get_orientation() {
			int orientation = host.getWindowManager().getDefaultDisplay().getRotation();
			switch (orientation) {
				default:
				case Surface.ROTATION_0: return 0; // PORTRAIT
				case Surface.ROTATION_90: return 1; // LANDSCAPE
				case Surface.ROTATION_180: return 2; // REVERSE_PORTRAIT
				case Surface.ROTATION_270: return 3; // REVERSE_LANDSCAPE
			}
		}

		public void set_orientation(int orientation) {
			switch (orientation) {
				case 0: // PORTRAIT
					screen_orientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT; break;
				case 1: // LANDSCAPE
					screen_orientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE; break;
				case 2: // REVERSE_PORTRAIT
					screen_orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT; break;
				case 3: // REVERSE_LANDSCAPE
					screen_orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE; break;
				default:
				case 4: // USER
					screen_orientation = ActivityInfo.SCREEN_ORIENTATION_USER; break;
				case 5: // USER_PORTRAIT
					screen_orientation = ActivityInfo.SCREEN_ORIENTATION_USER_PORTRAIT; break;
				case 6: // USER_LANDSCAPE
					screen_orientation = ActivityInfo.SCREEN_ORIENTATION_USER_LANDSCAPE; break;
				case 7: // USER_LOCKED
					screen_orientation = ActivityInfo.SCREEN_ORIENTATION_LOCKED; break;
			}
			host.setRequestedOrientation(screen_orientation);
		}

		public float get_display_scale() {
			DisplayMetrics dm = new DisplayMetrics();
			host.getWindowManager().getDefaultDisplay().getMetrics(dm);
			return dm.scaledDensity;
		}

		public boolean is_screen_on() {
			boolean on = host.pm.isScreenOn();
			return on;
		}

		public void set_volume_up() {
			host.am.adjustVolume (AudioManager.ADJUST_RAISE, AudioManager.FLAG_SHOW_UI | AudioManager.FLAG_PLAY_SOUND);
		}

		public void set_volume_down() {
			host.am.adjustVolume (AudioManager.ADJUST_LOWER, AudioManager.FLAG_SHOW_UI | AudioManager.FLAG_PLAY_SOUND);
		}

		public void open_url(String url) {
			Intent intent = new Intent();
			intent.setAction("android.intent.action.VIEW");
			Uri content_url = Uri.parse(url);
			intent.setData(content_url);
			host.startActivity(intent);
		}

		public void send_email(String recipient,
													 String subject,
													 String cc, String bcc, String body) {
			Intent intent = new Intent(Intent.ACTION_SENDTO);
			String url = "mailto:" + recipient;
			url += "?cc=" + cc;
			url += "&bcc=" + bcc;
			intent.setData(Uri.parse(url));
			intent.putExtra(Intent.EXTRA_SUBJECT, subject);
			intent.putExtra(Intent.EXTRA_TEXT, body);
			host.startActivity(intent);
		}
	};

}
