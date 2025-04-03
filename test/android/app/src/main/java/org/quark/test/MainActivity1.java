package org.quark.test;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import org.quark.test.databinding.ActivityMainBinding;

public class MainActivity1 extends AppCompatActivity {

	// Used to load the 'test' library on application startup.
	static {
		System.loadLibrary("test");
	}

	private ActivityMainBinding binding;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		binding = ActivityMainBinding.inflate(getLayoutInflater());
		setContentView(binding.getRoot());

		// Example of a call to a native method
		TextView tv = binding.sampleText;
		tv.setText(stringFromJNI());
	}

	/**
	 * A native method that is implemented by the 'test' native library,
	 * which is packaged with this application.
	 */
	public native String stringFromJNI();
}