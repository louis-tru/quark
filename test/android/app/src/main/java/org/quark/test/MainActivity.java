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

package org.quark.test;

import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import android.Manifest;
import android.widget.Toast;

import org.quark.Activity;

public class MainActivity extends Activity {

	static {
		System.loadLibrary("quark_test");
	}

	private static final int REQUEST_CODE_STORAGE_PERMISSION = 100;
	private static final String READ_EXTERNAL_STORAGE =
		Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU ? // Android 13+
			Manifest.permission.READ_MEDIA_VIDEO: Manifest.permission.READ_EXTERNAL_STORAGE;

	public boolean isStoragePermissionGranted() {
		return ContextCompat.checkSelfPermission(this, READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
	}

	public void requestStoragePermission() {
		// 检查是否需要展示权限请求解释对话框
		if (ActivityCompat.shouldShowRequestPermissionRationale(this, READ_EXTERNAL_STORAGE)) {
			// 显示解释对话框（例如 Toast 或 AlertDialog）
			Toast.makeText(this, "需要文件读取权限以加载本地视频", Toast.LENGTH_SHORT).show();
		}
		// 请求权限
		ActivityCompat.requestPermissions(
			this,
			new String[]{READ_EXTERNAL_STORAGE},
			REQUEST_CODE_STORAGE_PERMISSION
		);
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
		super.onRequestPermissionsResult(requestCode, permissions, grantResults);
		if (requestCode == REQUEST_CODE_STORAGE_PERMISSION) {
			if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
				// 权限已授予，执行文件读取操作
				// openVideoWithFFmpeg();
			} else {
				// 权限被拒绝，提示用户或关闭功能
				Toast.makeText(this, "权限被拒绝，无法读取文件", Toast.LENGTH_SHORT).show();
			}
		}
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		if (!isStoragePermissionGranted()) {
			requestStoragePermission();
		}
	}

	protected String startupArgv() {
		//return "http://192.168.0.11:1026/examples --inspect-brk=0.0.0.0:9229";
		//return ". --inspect-brk=0.0.0.0:9229";
		return getPathInAssets("jsapi aa action");
	}

}