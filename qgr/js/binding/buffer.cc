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

#include "qgr/js/wrap.h"
#include "qgr/utils/buffer.h"
#include "qgr/js/str.h"

/**
 * @ns qgr::js
 */

JS_BEGIN

/**
 * @class WrapBuffer
 */
class WrapBuffer: public WrapObject {
 public:
	
	/**
	 * @constructor([arg[,encoding]])
	 * @arg [arg=0] {uint|String|ArrayBuffer|Array}
	 * @arg [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}
	 */
	static void constructor(FunctionCall args) {
		JS_WORKER(args);
		
		if (args.Length() < 1) {
			New<WrapBuffer>(args, new Buffer()); return;
		}
		
		if (!(args[0]->IsUint32(worker) ||
					args[0]->IsString(worker) ||
					args[0]->IsArrayBuffer(worker) ||
					args[0]->IsArray(worker))
		) { // 参数错误
			JS_THROW_ERR(
										"* @constructor([arg[,encoding]])\n"
										"* @arg [arg=0] {uint|String|ArrayBuffer|Array}\n"
										"* @arg [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}\n"
			);
		}
		
		Encoding en = Encoding::utf8;
		
		if ( args[0]->IsUint32(worker) ) {
			int fill = 0;
			if ( args.Length() > 1 ) {
				fill = args[1]->ToInt32Value(worker);
			}
			int len = args[0]->ToUint32Value(worker);
			auto buf = new Buffer(len);
			if ( !buf->is_null() )
				memset(**buf, fill, buf->length());
			New<WrapBuffer>(args, buf);
		}
		else if ( args[0]->IsString(worker) ) {
			if ( args.Length() > 1 ) {
				if ( ! parse_encoding(args, args[1], en) ) return;
			}
			auto buff = new Buffer( args[0]->ToBuffer(worker, en) );
			New<WrapBuffer>(args, buff);
		}
		else if ( args[0]->IsArrayBuffer(worker) ) {
			Local<JSArrayBuffer> arr = args[0].To<JSArrayBuffer>();
			WeakBuffer buff(arr->Data(worker), arr->ByteLength(worker));
			New<WrapBuffer>(args, new Buffer(buff.copy()));
		}
		else if ( args[0]->IsArray(worker) ) {
			Buffer buff;
			if (args[0].To<JSArray>()->ToBufferMaybe(worker).To(buff)) {
				New<WrapBuffer>(args, new Buffer(buff));
			}
		}
	}
	
	/**
	 * @get length {uint}
	 */
	static void length(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Buffer);
		JS_RETURN( self->length() );
	}
	
	/**
	 * @indexed[] getter(uint index)
	 * @ret {uint}
	 */
	static void indexed_getter(uint index, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Buffer);
		if ( index < self->length() ) {
			JS_RETURN( (byte)self->item(index) );
		} else {
			JS_THROW_ERR("Buffer access out of bounds.");
		}
	}
	
	/**
	 * @indexed[] setter(uint index, uint value)
	 */
	static void indexed_setter(uint index, Local<JSValue> value, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Buffer);
		if ( index < self->length() ) {
			if (value->IsUint32(worker)) {
				(*self)[index] = value->ToUint32Value(worker);
				JS_RETURN( (byte)self->item(index) );
			} else {
				JS_THROW_ERR(
					"* @indexed setter[](uint index, uint value)\n"
				);
			}
		} else {
			JS_THROW_ERR("Buffer access out of bounds.");
		}
	}
	
	/**
	 * @func copy()
	 * @ret {Buffer} return new Buffer
	 */
	static void copy(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Buffer);
		JS_RETURN( self->copy() );
	}
	
	/**
	 * @func is_null()
	 * @ret {bool}
	 */
	static void is_null(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Buffer);
		JS_RETURN( self->is_null() );
	}
	
	/**
	 * @func parse_encoding()
	 */
	static bool parse_encoding(FunctionCall args, const Local<JSValue>& arg, Encoding& en) {
		JS_WORKER(args);
		String s = arg->ToStringValue(worker);
		en = Coder::parse_encoding( s );
		if ( en == Encoding::unknown ) {
			worker->throw_err( 
				"Unknown encoding \"%s\", the optional value is "
				"[binary|ascii|base64|hex|utf8|ucs2|utf16|utf32]", *s ); return false;
		}
		return true;
	}

	/**
	 * @func write(buffer[,to[,size[,form]]])
	 * @func write(string[,to[,encoding]])
	 * @func write(string[,encoding])
	 * @arg buffer {Buffer|ArrayBuffer|Array}
	 * @arg string {String}
	 * @arg [to=-1]   {int}  当前Buffer开始写入的位置
	 * @arg [size=-1] {int}  需要写入项目数量,超过要写入数据的长度自动取写入数据长度
	 * @arg [form=0]  {uint} 从要写入数据的form位置开始取数据
	 * @arg [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf32}
	 * @ret {uint} 返回写入的长度
	 */
	/**
	 * @func write_()
	 */
	static void write_(FunctionCall args, cchar* err_msg) {
		JS_WORKER(args);
		
		if (args.Length() == 0 || !(args[0]->IsString(worker) ||
																args[0]->IsArrayBuffer(worker) ||
																args[0]->IsArray(worker) ||
																worker->has_buffer(args[0])
																)
		) { // 参数错误
			JS_THROW_ERR( err_msg );
		}
		
		JS_SELF(Buffer);
		
		int  to = -1;
		int  size = -1;
		uint form = 0;
		Encoding en = Encoding::utf8;
		
		if ( args.Length() > 1 ) {
			if ( args[1]->IsInt32(worker) ) { // 写入到目标位置
				to = args[1]->ToInt32Value(worker);
				if ( args.Length() > 2 ) {
					if ( args[2]->IsInt32(worker) ) { // 写入大小
						size = args[2]->ToInt32Value(worker);
						if ( args.Length() > 3 ) { // 编码格式
							if ( args[3]->IsUint32(worker) ) {
								form = args[3]->ToInt32Value(worker);
							}
						}
					} else if (args[2]->IsString(worker)) { // 编码格式
						if ( ! parse_encoding(args, args[2], en) ) return;
					}
				}
			} else if (args[1]->IsString(worker)) { // 编码格式
				if ( ! parse_encoding(args, args[1], en) ) return;
			}
		}
		
		if ( args[0]->IsString(worker) ) { // 写入字符串
			Buffer buff = args[0]->ToBuffer(worker, en);
			JS_RETURN( self->write(buff, to) );
		}
		else if (args[0]->IsArrayBuffer(worker)) { // 写入原生 ArrayBuffer
			Local<JSArrayBuffer> ab = args[0].To<JSArrayBuffer>();
			WeakBuffer buff(ab->Data(worker), ab->ByteLength(worker));
			JS_RETURN( self->write(buff, to, size, form) );
		}
		else if ( args[0]->IsArray(worker) ) { // 写入数组 [ 0...255 ]
			Buffer buff;
			if (args[0].To<JSArray>()->ToBufferMaybe(worker).To(buff)) {
				JS_RETURN( self->write(buff, to, size, form) );
			}
		}
		else { // Buffer
			Buffer* buff = Wrap<Buffer>::unpack(args[0].To<JSObject>())->self();
			JS_RETURN( self->write(*buff, to, size, form) );
		}
	}


	static void write(FunctionCall args) {
		cchar* err_msg =
		"* @func write(src[,to[,size[,form]][,encoding]][,encoding])\n"
		"* @arg src {String|Buffer|ArrayBuffer|Array}\n"
		"* @arg [to=-1]    {int} 当前Buffer开始写入的位置,-1从结尾开始写入\n"
		"* @arg [size=-1]  {int} 需要写入项目数量,超过要写入数据的长度自动取写入数据长度,-1为src源长度\n"
		"* @arg [form=0]   {uint} 从要写入数据的form位置开始取数据,默认为0\n"
		"* @arg [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}\n"
		"* @ret {uint} return new length\n";
		write_(args, err_msg);
	}
	
	/**
	 * @func push(item[,encoding])
	 * @arg src {String|Buffer|ArrayBuffer|Array}
	 * @arg [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}
	 * @ret {uint} return buffer length
	 */
	static void push(FunctionCall args) {
		JS_WORKER(args);
		
		cchar* err_msg =
		"* @func push(item[,encoding])\n"
		"* @arg item {String|Buffer|ArrayBuffer|Array}\n"
		"* @arg [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}\n"
		"* @ret {uint} return buffer length\n";
		
		if ( args.Length() > 1 ) {
			if ( !args[1]->IsString(worker) ) { // non encoding
				JS_THROW_ERR(err_msg);
			}
		}
		
		write_(args, err_msg);
	}
	
	/**
	 * @func to_string([encoding[,start[,end]]])
	 * @arg [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}
	 * @arg [start=0] {uint}
	 * @arg [end] {uint}
	 */
	static void to_string(FunctionCall args) {
		JS_WORKER(args);
		
		JS_SELF(Buffer);
		
		Encoding encoding = Encoding::utf8;
		uint start = 0;
		uint end = self->length();
		int args_index = 0;
		
		if (args.Length() > args_index && args[args_index]->IsString(worker)) {
			if ( ! parse_encoding(args, args[0], encoding) ) return;
			args_index++;
		}
		if (args.Length() > args_index && args[args_index]->ToUint32Maybe(worker).To(start)) {
			start = XX_MIN(self->length(), start);
			args_index++;
		}
		if (args.Length() > args_index && args[args_index]->ToUint32Maybe(worker).To(end)) {
			end = XX_MIN(self->length(), end);
			args_index++;
		}
		
		if ( end <= start ) {
			JS_RETURN( JSString::Empty(worker) );
		}
		
		switch (encoding) {
			case Encoding::hex: // 编码
			case Encoding::base64: {
				Buffer buff = Coder::encoding(encoding, **self + start, end - start);
				JS_RETURN( worker->NewString(buff) );
				break;
			} default: {// 解码to ucs2
				Ucs2String str( Coder::decoding_to_uint16(encoding, **self + start, end - start) );
				JS_RETURN( worker->New(str) );
				break;
			}
		}
	}
	
	/**
	 * @func collapse()
	 * @ret {ArrayBuffer}
	 */
	static void collapse(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Buffer);
		uint len = self->length();
		char* s = self->collapse();
		Local<JSArrayBuffer> ab = JSArrayBuffer::New(worker, s, len);
		JS_RETURN( ab );
	}
	
	/**
	 * @func slice([start,[end]])
	 * @arg [start = 0] {uint}
	 * @arg [end = -1] {int}
	 * @ret {Buffer}
	 */
	static void slice(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Buffer);
		
		if (args.Length() == 0) {
			JS_RETURN( self->copy() );
		}
		
		uint start = 0;
		int end = -1;
		
		if ( args.Length() > 0 ) {
			if ( !args[0]->ToUint32Maybe(worker).To(start) ) return;
			if ( args.Length() > 1 ) {
				if ( !args[1]->ToInt32Maybe(worker).To(end) ) return;
			}
		}
		
		start = XX_MIN(self->length(), start);
		
		if ( end == -1 ) {
			end = self->length();
		} else {
			end = XX_MIN(self->length(), end);
		}
		
		if ( end <= start ) {
			JS_RETURN(  Buffer() );
		}
		
		auto arr = self->slice(start, end);
		JS_RETURN(  move(*static_cast<Buffer*>(&arr)) );
	}
	
	/**
	 * @func clear()
	 * @ret {Buffer} return self
	 */
	static void clear(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Buffer);
		self->clear();
		JS_RETURN( args.This() );
	}
	
	/**
	 * @func pop([count])
	 * @arg [count=1] {uint}
	 * @ret {uint} 返回移除后的长度
	 */
	static void pop(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Buffer);
		uint rv;
		if (args.Length() > 0 && args[0]->IsUint32(worker)) {
			rv = self->pop( args[0]->ToUint32Value(worker) );
		} else {
			rv = self->pop();
		}
		JS_RETURN( rv );
	}
	
	/**
	 * @func to_array_
	 */
	static Local<JSArray> to_array_(Worker* worker, cBuffer& buff) {
		Local<JSArray> arr = worker->NewArray(buff.length());
		for (uint i = 0, len = buff.length(); i < len; i++) {
			arr->Set(worker, i, worker->New((byte)buff[i]) );
		}
		return arr;
	}
	
	/**
	 * @func toJSON()
	 * @ret {Array}
	 */
	static void toJSON(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Buffer);
		Local<JSObject> rv = worker->NewObject();
		rv->Set(worker, worker->strs()->type(), worker->strs()->Buffer()); // type:
		rv->Set(worker, worker->strs()->data(), to_array_(worker, *self)); // data:
		JS_RETURN( rv );
	}
	
	/**
	 * @func to_array()
	 * @ret {Array}
	 */
	static void to_array(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Buffer);
		JS_RETURN( to_array_(worker, *self) );
	}
	
	/**
	 * @func fill(value)
	 * @arg value {uint}
	 * @ret {Buffer} return self
	 */
	static void fill(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || ! args[0]->IsUint32(worker)) {
			JS_THROW_ERR(
										"* @func fill(value)\n"
										"* @arg value {uint}\n"
										"* @ret {Buffer} return self\n"
			);
		}
		JS_SELF(Buffer);
		memset(**self, args[0]->ToUint32Value(worker), self->size());
		JS_RETURN( args.This() );
	}
	
	/**
	 * @func for_each(Function)
	 * @ret {Buffer} return self
	 */
	static void for_each(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || ! args[0]->IsFunction(worker) ) {
			JS_THROW_ERR(
				"* @func for_each(Function)\n"
			);
		}
		JS_HANDLE_SCOPE();
		Local<JSValue> recv = args.Length() > 1 ? args[1] : args.This().To<JSValue>();
		Local<JSFunction> cb = args[0].To<JSFunction>();
		
		JS_SELF(Buffer);
		Buffer& buff = *self;
		Array<Local<JSValue>> arr(3);
		arr[2] = args.This();
		
		for (uint i = 0; i < buff.length(); i++) {
			arr[0] = worker->New((byte)buff[i]);
			arr[1] = worker->New(i);
			if ( cb->Call(worker, 3, &arr[0], recv).IsEmpty() ) {
				return;
			}
		}
		
		JS_RETURN( args.This() );
	}
	
	/**
	 * @func map(Function)
	 * @ret {Buffer} return new Buffer
	 */
	static void map(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || ! args[0]->IsFunction(worker) ) {
			JS_THROW_ERR(
										"* @func map(Function)\n"
										"* @ret {Buffer} return new Buffer\n"
										);
		}
		JS_HANDLE_SCOPE();
		Local<JSValue> recv = args.Length() > 1 ? args[1] : args.This().To<JSValue>();
		Local<JSFunction> cb = args[0].To<JSFunction>();
		
		JS_SELF(Buffer);
		Buffer& buff = *self;
		Local<JSValue> arr[3];
		arr[2] = args.This();
		Buffer result;
		
		for (uint i = 0; i < buff.length(); i++) {
			arr[0] = worker->New((byte)buff[i]);
			arr[1] = worker->New(i);
			Local<JSValue> rv = cb->Call(worker, 3, arr, recv);
			if ( !rv.IsEmpty() ) {
				result.push( rv->ToInt32Value(worker) );
			} else { // 异常结束
				return;
			}
		}
		JS_RETURN( move( result ) );
	}
	
	/**
	 * @func filter(Function)
	 * @ret {Buffer} return new Buffer
	 */
	static void filter(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || ! args[0]->IsFunction(worker) ) {
			JS_THROW_ERR(
				"* @func filter(Function)\n"
				"* @ret {Buffer} return new Buffer\n"
			);
		}
		JS_HANDLE_SCOPE();
		
		Local<JSValue> recv = args.Length() > 1 ? args[1] : args.This().To<JSValue>();
		Local<JSFunction> cb = args[0].To<JSFunction>();
		
		JS_SELF(Buffer);
		Buffer& buff = *self;
		Local<JSValue> arr[3];
		arr[2] = args.This();
		Buffer result;
		
		for (uint i = 0; i < buff.length(); i++) {
			arr[0] = worker->New((byte)buff[i]);
			arr[1] = worker->New(i);
			
			Local<JSValue> rev = cb->Call(worker, 3, arr, recv);
			if ( !rev.IsEmpty() ) {
				if ( rev->ToBooleanValue(worker) ) {
					result.push( buff[i] );
				}
			} else { // 异常结束
				return;
			}
		}
		JS_RETURN( move(result) );
	}
	
	/**
	 * @func some(Function)
	 * @ret {bool} 
	 */
	static void some(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || ! args[0]->IsFunction(worker) ) {
			JS_THROW_ERR(
				"* @func some(Function)\n"
				"* @ret {bool}\n"
			);
		}
		JS_HANDLE_SCOPE();
		
		Local<JSValue> recv = args.Length() > 1 ? args[1] : args.This().To<JSValue>();
		Local<JSFunction> cb = args[0].To<JSFunction>();
		
		JS_SELF(Buffer);
		Buffer& buff = *self;
		Local<JSValue> arr[3];
		arr[2] = args.This();
		
		for (uint i = 0; i < buff.length(); i++) {
			arr[0] = worker->New((byte)buff[i]);
			arr[1] = worker->New(i);
			Local<JSValue> rv = cb->Call(worker, 3, arr, recv);
			if ( !rv.IsEmpty() ) {
				if (rv->ToBooleanValue(worker)) {
					JS_RETURN( true );
				}
			} else { // 异常结束
				return;
			}
		}
		JS_RETURN( false );
	}
	
	/**
	 * @func every(Function)
	 * @ret {bool}
	 */
	static void every(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || ! args[0]->IsFunction(worker) ) {
			JS_THROW_ERR(
				"* @func every(Function)\n"
				"* @ret {bool}\n"
			);
		}
		JS_HANDLE_SCOPE();
		Local<JSValue> recv = args.Length() > 1 ? args[1] : args.This().To<JSValue>();
		Local<JSFunction> cb = args[0].To<JSFunction>();
		
		JS_SELF(Buffer);
		Buffer& buff = *self;
		Local<JSValue> arr[3];
		arr[2] = args.This();
		
		for (uint i = 0; i < buff.length(); i++) {
			arr[0] = worker->New((byte)buff[i]);
			arr[1] = worker->New(i);
			Local<JSValue> rv = cb->Call(worker, 3, arr, recv);
			if ( !rv.IsEmpty() ) {
				if ( !rv->ToBooleanValue(worker) ) {
					JS_RETURN( false );
				}
			} else { // 异常结束
				return;
			}
		}
		JS_RETURN( true );
	}

	/**
	 * @func binding
	 */
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_NEW_CLASS(Buffer, constructor, {
			JS_SET_CLASS_ACCESSOR(length, length);
			JS_SET_CLASS_INDEXED(indexed_getter, indexed_setter);
			JS_SET_CLASS_METHOD(copy, copy);
			JS_SET_CLASS_METHOD(isNull, is_null);
			JS_SET_CLASS_METHOD(write, write);
			JS_SET_CLASS_METHOD(toString, to_string);
			JS_SET_CLASS_METHOD(collapse, collapse);
			JS_SET_CLASS_METHOD(slice, slice);
			JS_SET_CLASS_METHOD(clear, clear);
			JS_SET_CLASS_METHOD(toJSON, toJSON);
			JS_SET_CLASS_METHOD(fill, fill);
			JS_SET_CLASS_METHOD(forEach, for_each);
			JS_SET_CLASS_METHOD(map, map);
			JS_SET_CLASS_METHOD(filter, filter);
			JS_SET_CLASS_METHOD(some, some);
			JS_SET_CLASS_METHOD(every, every);
			//JS_SET_CLASS_METHOD(push, push);
			//JS_SET_CLASS_METHOD(pop, pop);
			//JS_SET_CLASS_METHOD(toArray, to_array);
		}, 0);
		
		cls->Export(worker, "Buffer", exports);
	}
};

JS_REG_MODULE(_buffer, WrapBuffer);
JS_END
