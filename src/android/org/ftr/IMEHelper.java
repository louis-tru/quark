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

package org.ftr;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.text.InputType;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.CompletionInfo;
import android.view.inputmethod.CorrectionInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.ExtractedText;
import android.view.inputmethod.ExtractedTextRequest;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputContentInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;

import java.util.Date;

public class IMEHelper extends EditText {

	private boolean m_has_open = false;
	private long m_action_time = 0;
	private InputMethodManager m_imm = null;
	private ImplementsInputConnection m_ic = null;
	private boolean m_can_backspace = false;
	private boolean m_can_delete = false;
	private int m_keyboard_type = 0;
	private int m_keyboard_return_type = 0;
	private int m_inputType = InputType.TYPE_CLASS_TEXT;
	private int m_imeOptions = EditorInfo.IME_ACTION_NONE;
	private StringBuilder m_text = new StringBuilder();
	private CharSequence  m_marked = null;

	private static native void dispatchIMEDelete(int count);
	private static native void dispatchIMEInsert(String text);
	private static native void dispatchIMEMarked(String text);
	private static native void dispatchIMEUnmark(String text);
	private static native void dispatchKeyboardInput(int keycode, boolean ascii, boolean down,
																									 int repeat, int device, int source);

	static private class ImplementsInputConnection implements InputConnection {

		private IMEHelper m_host = null;

		ImplementsInputConnection(IMEHelper host) {
			m_host = host;
		}
		// implements InputConnection

		public CharSequence getTextBeforeCursor(int n, int flags) {
			if ( m_host.m_text.length() > 4 ) {
				return m_host.m_text.substring(m_host.m_text.length() - 4, m_host.m_text.length());
			}
			return m_host.m_text;
		}

		public CharSequence getTextAfterCursor(int n, int flags) {
			return "";
		}

		public CharSequence getSelectedText(int flags) {
			return "";
		}

		public int getCursorCapsMode(int reqModes) {
			return 0;
		}

		public ExtractedText getExtractedText(ExtractedTextRequest request, int flags) {
			return null;
		}

		public boolean deleteSurroundingText(int beforeLength, int afterLength) {
			if (m_host.m_text.length() > beforeLength) {
				m_host.m_text.delete(m_host.m_text.length() - beforeLength, m_host.m_text.length());
			} else {
				m_host.m_text.delete(0, m_host.m_text.length());
			}
			if ( beforeLength == 1 ) {
				dispatchIMEDelete(-1);
			} else {
				dispatchIMEDelete(-beforeLength);
			}
			return true;
		}

		public boolean deleteSurroundingTextInCodePoints(int beforeLength, int afterLength) {
			return true;
		}

		public boolean setComposingText(CharSequence text, int newCursorPosition) {
			m_host.m_marked = text;
			dispatchIMEMarked(text.toString());
			return true;
		}

		public boolean setComposingRegion(int start, int end) {
			return true;
		}

		public boolean finishComposingText() {
			 m_host.finish();
			return true;
		}

		private void ime_insert(String s) {
			m_host.m_text.append(s);
			if ( m_host.m_marked == null ) {
				dispatchIMEInsert(s.toString());
			} else {
				m_host.m_marked = null;
				dispatchIMEUnmark(s.toString());
			}
		}

		public boolean commitText(CharSequence text, int newCursorPosition) {
			boolean ascii = text.length() == 1 && text.charAt(0) < 128; // ascii

			if ( ascii ) { // ascii
				dispatchKeyboardInput(text.charAt(0), true, true, 0, -1, 0);
			}
			ime_insert(text.toString());
			if ( ascii ) { // ascii
				dispatchKeyboardInput(text.charAt(0), true, false, 0, -1, 0);
			}
			return true;
		}

		public boolean commitCompletion(CompletionInfo text) {
			return true;
		}

		public boolean commitCorrection(CorrectionInfo correctionInfo) {
			return true;
		}

		@Override
		public boolean commitContent(InputContentInfo inputContentInfo, int i, Bundle bundle) {
			return true;
		}

		public boolean setSelection(int start, int end) {
			return true;
		}

		public boolean performEditorAction(int editorAction) {
			m_host.m_text.append("\n");
			dispatchKeyboardInput(13, true, true, 0, -1, 0);
			dispatchIMEInsert("\n");
			dispatchKeyboardInput(13, true, false, 0, -1, 0);
			return true;
		}

		public boolean performContextMenuAction(int id) {
			return true;
		}

		public boolean beginBatchEdit() {
			return false;
		}

		public boolean endBatchEdit() {
			return false;
		}

		public boolean sendKeyEvent(KeyEvent event) {
			dispatchKeyboardInput(
							event.getKeyCode(), false,
							event.getAction() == KeyEvent.ACTION_DOWN,
							event.getRepeatCount(), event.getDeviceId(), event.getSource()
			);

			if ( event.getAction() == KeyEvent.ACTION_DOWN ) {
				if ( event.getKeyCode() == KeyEvent.KEYCODE_DEL ) {
					if (m_host.m_text.length() > 0) {
						m_host.m_text.deleteCharAt(m_host.m_text.length() - 1);
					}
					dispatchIMEDelete(-1);
				} else {
					if ( event.getKeyCode() >= KeyEvent.KEYCODE_0 && event.getKeyCode() <= KeyEvent.KEYCODE_9 ) {
						char c = '0';
						c += (event.getKeyCode() - KeyEvent.KEYCODE_0);
						ime_insert(Character.toString(c));
					} else if ( event.getKeyCode() >= KeyEvent.KEYCODE_A && event.getKeyCode() <= KeyEvent.KEYCODE_Z ) {
						char c = 'A';
						c += (event.getKeyCode() - KeyEvent.KEYCODE_A);
						ime_insert(Character.toString(c));
					} else {
						// TODO..
						Log.d("IMEHelper", String.format("keycode:%s", event.getKeyCode()));
					}
				}
			}
			return true;
		}

		public boolean clearMetaKeyStates(int states) {
			return true;
		}

		public boolean reportFullscreenMode(boolean enabled) {
			return true;
		}

		public boolean performPrivateCommand(String action, Bundle data) {
			return true;
		}

		public boolean requestCursorUpdates(int cursorUpdateMode) {
			return true;
		}

		public Handler getHandler() {
			return null;
		}

		public void closeConnection() {
			m_host.finish();
		}

	}

	public IMEHelper(final Context context) {
		super(context);
		m_imm = (InputMethodManager) context.getSystemService(Context.INPUT_METHOD_SERVICE);
	}

	public void open() {
		m_has_open = true;
		m_action_time = new Date().getTime();
		setVisibility(View.VISIBLE);
		requestFocus();
		m_imm.showSoftInput(this, 0);
	}

	public void close() {
		m_has_open = false;
		setVisibility(View.INVISIBLE);
		clearFocus();
		m_imm.hideSoftInputFromWindow(getWindowToken(), 0);
	}

	private void finish() {
		if ( m_marked != null ) {
			dispatchIMEUnmark(m_marked.toString());
			m_marked = null;
		}
	}

	public void clear() {
		m_text = new StringBuilder();
		m_marked = null;
		m_can_backspace = true;
		m_can_delete = true;
		m_keyboard_type = 0;
		m_keyboard_return_type = 0;
		m_inputType = InputType.TYPE_CLASS_TEXT;
		m_imeOptions = EditorInfo.IME_ACTION_NONE;
		setInputType(m_inputType);
		setImeOptions(m_imeOptions);
	}

	void set_can_backspace(boolean can_backspace, boolean can_delete) {
		m_can_backspace = can_backspace;
		m_can_delete = can_delete;
	}

	void set_keyboard_type(int type) {

		if ( type == m_keyboard_type ) return;

		m_action_time = new Date().getTime();
		m_keyboard_type = type;

		switch(type) {
			default:
			case 45: // NORMAL
				m_inputType = InputType.TYPE_CLASS_TEXT;
				break;
			case 53: // ASCII
			case 63: // ASCII_NUMBER
				m_inputType = InputType.TYPE_CLASS_TEXT;
				m_imeOptions = EditorInfo.IME_FLAG_FORCE_ASCII;
				break;
			case 54: // NUMBER
			case 56: // NUMBER_PAD
				m_inputType = InputType.TYPE_CLASS_NUMBER;
				break;
			case 60: // DECIMAL
				m_inputType = InputType.TYPE_CLASS_NUMBER |
								InputType.TYPE_NUMBER_FLAG_DECIMAL | InputType.TYPE_NUMBER_FLAG_SIGNED;
			case 55: // URL
				m_inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI;
				break;
			case 57: // PHONE
				m_inputType = InputType.TYPE_CLASS_PHONE;
				break;
			case 58: // NAME_PHONE
				m_inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PERSON_NAME;
				break;
			case 59: // EMAIL
				m_inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS;
				break;
			case 61: // TWITTER
				m_inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_SHORT_MESSAGE;
				break;
			case 62: // SEARCH
				m_inputType = InputType.TYPE_CLASS_TEXT;
				m_imeOptions = EditorInfo.IME_ACTION_GO;
				break;
		}

		setInputType(m_inputType);
		setInputType(m_imeOptions);
	}

	void set_keyboard_return_type(int type) {

		if ( type == m_keyboard_return_type ) return;

		m_action_time = new Date().getTime();
		m_keyboard_return_type = type;

		switch(type) {
			default:
			case 45: // NORMAL
				m_imeOptions = EditorInfo.IME_ACTION_NONE;
				break;
			case 64: // GO
				m_imeOptions = EditorInfo.IME_ACTION_GO;
				break;
			case 65: // JOIN
				m_imeOptions = EditorInfo.IME_ACTION_NONE;
				break;
			case 66: // NEXT
				m_imeOptions = EditorInfo.IME_ACTION_NEXT;
				break;
			case 67: // ROUTE
				m_imeOptions = EditorInfo.IME_ACTION_NONE;
				break;
			case 68: // SEARCH
				m_imeOptions = EditorInfo.IME_ACTION_SEARCH;
				break;
			case 69: // SEND
				m_imeOptions = EditorInfo.IME_ACTION_SEND;
				break;
			case 70: // DONE
				m_imeOptions = EditorInfo.IME_ACTION_DONE;
				break;
			case 71: // EMERGENCY
				m_imeOptions = EditorInfo.IME_ACTION_NONE;
				break;
			case 72: // CONTINUE
				m_imeOptions = EditorInfo.IME_ACTION_NONE;
				break;
		}

		setImeOptions(m_imeOptions);
	}

	@Override
	public boolean onKeyDown(final int pKeyCode, final KeyEvent pKeyEvent) {
		super.onKeyDown(pKeyCode, pKeyEvent);
		return true;
	}

	@Override
	public boolean onCheckIsTextEditor() {
		return m_has_open;
	}

	@Override
	public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
		if ( m_ic == null ) {
			m_ic = new ImplementsInputConnection(this);
		}
		outAttrs.inputType = m_inputType;
		outAttrs.imeOptions = m_imeOptions |
						EditorInfo.IME_FLAG_NO_FULLSCREEN |
						EditorInfo.IME_FLAG_NO_EXTRACT_UI;
		return m_ic;
	}

}

