/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015-2016, blue.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

var fs    = require('fs');
var path  = require('path');
var input = process.argv[2];
var code = fs.readFileSync(input, 'utf-8');
var name = getBasenamePrefix(input);
var imports = {};
var types = {};
var comments = [];
var packIn = null;
var resolved = new Set();

function getBasenamePrefix(str) {
	var basename = path.basename(str);
	var ext = path.extname(str);
	return basename.substring(0, basename.length - ext.length);
}

class Item {
	constructor(kind,value='',{desc='',msgs=[],...args}={}) {
		this.kind = kind;
		this.value = value;
		this.desc = desc;
		this.msgs = msgs;
		this.modifiers = [];
		Object.assign(this,args);
	}

	set_modifiers(modifiers) {
		if (modifiers)
			this.modifiers.push(modifiers);
	}

	set_key(key) {
		if (key)
			this.key = key;
	}
}

function fixParams(comment, args) {
	for (let arg of args) {
		let name = arg[0].replaceAll('.', ''), type = arg[1];
		let pa = comment.param.find(e=>e.name==name);
		if (pa) {
			if (!pa.type && type)
				pa.type = type;
		} else {
			let item = new Item('param','',{name,type});
			comment.param.push(item);
			comment.__items__.push(item);
		}
	}
}

function fixTempl(comment, templMat) {
	if (templMat) {
		let templ = comment.template;
		let types = templMat.substring(1, templMat.length - 1)
			.split(',')
			.map(e=>e.trim().split(' ')[0]);
		if (!templ) {
			comment.template = templ = new Item('template','',{types});
			comment.__items__.push(templ);
		} else {
			for (let type of types) {
				if (templ.types.indexOf(type) == -1) {
					templ.types.push(type);
				}
			}
		}
	}
}

// For:
// export function transition(view: View,to: KeyframeIn,
// 	fromOrCb?: KeyframeIn | ActionCb,cb?: ActionCb): KeyframeAction;
// protected triggerLoad(): any;
// get metaView();
// set metaView(val: View);
// addFrame(time: number, curve?: types.CurveIn): Keyframe;
// static hashCode();
// ---------------------
// update(cb?: ()=>void);
// function sheetConfirm(window: Window, buttons?: RenderData[], cb?: (index: number) => void): Sheet
// constructor(props: Readonly<P & { ref?: string, key?: string|number }>,{window,children,owner}: Args);
// setState<K extends keyof S>(newState: Pick<S, K>, cb?: ()=>void);
// asDom<T extends ViewController | View = View>();
function tryMethod(comment, method, lastIndex) {
	let re = comment.return;
	if (re) {
		let [,,type,desc] = `${re.value} ${re.desc}`.match(/(\{([^\}\s]+)\})?(.+)/);
		if (!type)
			re.value = '';
		re.type = type;
		re.desc = desc;
	}
	if (method) {
		// Match to type: \w\[\]\<\>\|,\.
		let valueMat = method.value.match(/(\w+)(?:\(([^\)]*)\)\s*([\w\[\]\<\>\|,\.]+)?)?/);
		if (valueMat[2]) {
			let argsComment = valueMat[2].split(/\s*,\s*/).map(e=>e.split(/\s*:\s*/));
			method.name = valueMat[1];
			method.value = `${valueMat[1]}(${argsComment.map(e=>e[0]).join(',')})`;
			fixParams(comment, argsComment);
		}
		if (valueMat[3]) {
			if (!comment.return) {
				comment.return = new Item('return', '', {type:valueMat[3]});
				comment.__items__.push(comment.return);
			} else if (!comment.return.type) {
				comment.return.type = valueMat[3];
			}
		}
	}

	// Match to type: ([\w\[\]\<\>\|,\.\s]+)
	let reg = /\W*(?:(export|static|protected|private|public)\s+)?(?:(async)\s+)?(?:(function|get|set)\s+)?(\w+)\s*(\<[^\>]+\>)?\s*\(([\w\[\]\<\>,\.\?:\|\s]*)\)\s*(?::\s*([\w\[\]\<\>\|,\.\s]+))?/my;
	reg.lastIndex = lastIndex;
	let mat = code.match(reg);
	if (!mat)
		return;
	let [,modifiers,asyncMat,key,name,templMat,argsMat,returnV] = mat;

	if (packIn) {
		if (modifiers == 'export' || key == 'function') {
			packIn = null; // cancel pack in
			comment.__packIn__ = null;
		}
	} else {
		if (modifiers && modifiers != 'export')
			return;
		if (key != 'function')
			return;
	}

	fixTempl(comment, templMat);

	let addReturnItems = false;

	if (returnV) { // return value
		if (!re) {
			comment.return = re = new Item('return', '', {type:returnV});
			addReturnItems = true;
		} else if (!re.type) {
			re.type = returnV;
		}
	}

	let newMainItem = (kind,value,args)=>{
		let item = new Item(kind,value,args);
		return comment.__items__.unshift(comment[kind] = item), item;
	}

	let kind = key == 'get' || key == 'set' ? key: 'method';
	let argsStr = argsMat.trim();
	let argsTemplRep = [];
	let args = !argsStr ? []: argsStr
		.replace(/\<[^\>]+\>/g, e=>(argsTemplRep.push(e),'$&'))
		.split(/\s*,\s*/).
		map(e=>e.replaceAll('$&',e=>argsTemplRep.shift()).split(/\s*:\s*/));
	let item = comment[kind];

	if (kind == 'method') {
		let value = `${name}(${args.map(e=>e[0]).join(',')})`;
		item = item  || newMainItem(kind,value,{name});
		item.name || Object.assign(item,{value,name});
	} else {
		item = item || comment.getset ||
			newMainItem(kind, '', {name,type: (args[0] && args[0][1]) || (re && re.type)});
	}

	fixParams(comment, args);

	if (addReturnItems) // add to end
		comment.__items__.push(re);

	item.set_modifiers(modifiers);
	item.set_modifiers(asyncMat);

	return comment;
}

// For:
//        readonly [index: number]  : Keyframe;
//        readonly length           : number;
//        readonly children         : (VirtualDOM | null)[] = [];
// static readonly isViewController : boolean               = true;
// export const    createCss        : object                =
function tryTypeOrGetset(comment, item/*get/set/getset*/, lastIndex) {
	let c = tryType(comment,item,lastIndex);
	if (c)
		return c;

	// Match to type: ([\w\[\]\<\>\|,\.\s]+)
	let reg = /\W*(?:(export|static|protected|private|public|@event|@link|@link\.acc)\s+)?(?:(const|readonly)\s+)?(?:(\w+\??)|(\[[\w:\s]+\]))\s*(:\s*([\w\[\]\<\>\|,\.\s]+)|\()?/my;
	reg.lastIndex = lastIndex;
	let mat = code.match(reg);
	if (!mat)
		return;
	let [,modifiers,key,name='indexed',indexed,_t,type] = mat;

	if (_t == '(')
		return;

	if (['function','var','let'].indexOf(name) != -1)
		return;

	if (packIn) {
		if (modifiers == 'export' || key == 'const') {
			packIn = null; // cancel pack in
			comment.__packIn__ = null;
		}
	} else {
		if (modifiers && modifiers != 'export')
			return;
		if (key != 'const')
			return;
	}

	if (key == 'const') {
		packIn = comment;
		// export const Consts = {
		// 	Ok: 'OK', //!<
		// 	Cancel: 'Cancel', //!<
		// 	Placeholder: 'Please enter..', //!<
		// };
	}

	let kind = modifiers == '@event' ? 'event' :
		key ? (key == 'readonly' ? 'get': key):
		type ? 'getset': 'emunItem';
	if (!item) {
		item = new Item(kind,'',{name:name,type});
		comment[kind] = item;
		comment.__items__.unshift(item);
	} else {
		if (!item.name)
			item.name = name;
		if (!item.type)
			item.type = type;
	}

	if (indexed) {
		let [,index,type] = indexed.match(/\[\s*(\w+)\s*:\s*(string|number)\s*\]/);
		item.indexed = [index,type];
	}

	item.set_modifiers(modifiers);
	item.set_key(key);

	return comment;
}

// export type KeyframeIn = StyleSheet | CSSNameExp
function tryType(comment,item,lastIndex) {
	if (item) {
		// Match to type: \w\[\]\<\>\|,\.
		let valueMat = item.value.match(/(\w+)(:([\w\[\]\<\>\|,\.]+))?/);
		if (valueMat[3]) {
			item.name = valueMat[1];
			item.type = valueMat[3];
		}
	}

	// Match to type: ([\w\[\]\<\>\|,\.\s]+)
	let reg = /\W*(?:(export)\s+)?(type)\s+(\w+)\s*(\<[^\>]+\>)?\s*=\s*([\w\[\]\<\>\|,\.\s]+)/my;
	reg.lastIndex = lastIndex;
	let mat = code.match(reg);
	if (!mat)
		return;
	let [,modifiers,kind,name,templMat,type] = mat;

	if (!comment.type) {
		comment.type = new Item(kind,'',{name,type});
		comment.__items__.unshift(comment.type);
	}

	fixTempl(comment, templMat);

	comment.type.set_modifiers(modifiers);

	packIn = comment;
	comment.__packIn__ = null;

	return comment;
}

// interface FlexJSX extends BoxJSX
// export interface TouchPoint
// export declare class Keyframe extends StyleSheet
// export declare class View extends Notification<UIEvent> implements DOM
// export enum FindDirection
function tryPack(comment, pickItem, lastIndex) {
	for (let item of [comment.implements,comment.extends]) {
		if (item)
			item.types = item.value.split(/\s*,\s*/);
	}

	// Match to type: ([\w\[\]\<\>\|,\.\s]+)
	let reg = /\W*(?:(export)\s+)?(?:(declare)\s+)?(?:(class|interface|enum)\s+)(\w+)\s*(\<[^\>]+\>)?(?:\s+extends\s+([\w,\s]+))?(?:\s+implements\s+([\w,\s]+))?/my;
	reg.lastIndex = lastIndex;
	let mat = code.match(reg);
	if (!mat)
		return;
	let [,modifiers,,kind,name,templMat,extend,implements] = mat;

	if (!pickItem) {
		comment[kind] = pickItem = new Item(kind,name);
		comment.__items__.unshift(pickItem);
	}

	fixTempl(comment, templMat);

	for (let [k,v] of [['extends',extend],['implements',implements]]) {
		if (v && !comment[k]) {
			comment[k] = new Item(k,'',{types:v.split(/\s*,\s*/)});
			comment.__items__.push(comment[k]);
		}
	}

	pickItem.set_modifiers(modifiers);

	packIn = comment;
	comment.__packIn__ = null;

	return comment;
}

function parseCommentA(str, index) { /** ... */
	let len = str.length;
	let lines = str.split('\n').map(e=>e.replace(/\s*\*?\s?/, ''));
	let lastIndex = index + len + 4;

	let items = [
		// {kind: class|interface|enum|method|getset|type|callbackconst|param|return|example}
	];
	let comment = {
		param: [],
		__items__: items,
	};
	let unknownMsgs = [];

	for (let line of lines) {
		if (line[0] == '@') {
			let m = line.match(/^@(\w+)(\s+([^\s]+))?(\s+(.+))?/);
			let kind = m[1];
			let item = new Item(kind,m[3],{desc:m[5]});
			if (Array.isArray(comment[kind])) {
				comment[kind].push(item);
			} else {
				comment[kind] = item;
			}
			items.push(item);
		} else if (items.length) {
			items.at(items.length-1).msgs.push(line);
		} else if (line) {
			unknownMsgs.push(line);
		}
	}

	for (let pa of comment.param) {
		let [,name,type] = pa.value.match(/(\w+\??)(?::([\w\[\]\|]+))?/);
		pa.name = name;
		pa.type = type;
	}

	let templ = comment.template;
	if (templ && templ.value) {
		templ.types = templ.value.split(',');
	}

	let pickItem = comment.class || comment.interface || comment.enum;

	if (pickItem) {
		packIn = comment;
		tryPack(comment, pickItem, lastIndex);
	}
	else if (comment.method) {
		comment.__packIn__ = packIn;
		tryMethod(comment, comment.method, lastIndex);
	}
	else if (comment.get) {
		comment.__packIn__ = packIn;
		tryTypeOrGetset(comment, comment.get, lastIndex) || tryMethod(comment, null, lastIndex);
	}
	else if (comment.set) {
		comment.__packIn__ = packIn;
		tryTypeOrGetset(comment, comment.set, lastIndex) || tryMethod(comment, null, lastIndex);
	}
	else if (comment.getset) {
		comment.__packIn__ = packIn;
		tryTypeOrGetset(comment, comment.getset, lastIndex) || tryMethod(comment, null, lastIndex);
	}
	else if (comment.event) {
		comment.__packIn__ = packIn;
		tryTypeOrGetset(comment, comment.event, lastIndex);
	}
	else if (comment.callback) {
		packIn = null;
		tryMethod(comment, comment.callback, lastIndex)
	}
	else if (comment.type) {
		packIn = comment;
		tryType(comment, comment.type, lastIndex);
	}
	else if (comment.const) {
		packIn = comment;
		tryGetset(comment, comment.const, lastIndex);
	}
	else if (comment.default) {
		packIn = comment;
	}
	else if (comment.end) {
		packIn = null;
		return;
	}
	else if (comment.information) {
		packIn = null;
	}
	else {
		comment.__packIn__ = packIn;
		comment =
			tryPack(comment, null, lastIndex) ||
			tryTypeOrGetset(comment, null, lastIndex) ||
			tryMethod(comment, null, lastIndex);
	}

	if (items.length == 0)
		return;

	if (unknownMsgs.length) {
		items[0].msgs.unshift(...unknownMsgs);
	}

	return comment;
}

function parseCommentB(str, index) { //!< ...
	let lastIndex = code.lastIndexOf('\n', index);
	if (lastIndex != -1) {
		let items = [];
		let comment = {
			param: [],
			__packIn__: packIn,
			__items__: items,
		};

		comment = tryTypeOrGetset(comment, null, lastIndex) || tryMethod(comment, null, lastIndex);

		if (comment) {
			let mat = str.match(/\/\/\!\<(?:\s*\{([^\}]+)\})?\s*(.*)/);
			if (mat) {
				let [,type,desc] = mat;
				if (type)
					items[0].type = type;
				if (desc)
					items[0].desc = desc;
			}
		}

		return comment;
	}
}

var imports_mat = code.matchAll(
	/import\s+(\{([\w$_,\s]+)\}|(\w+)|(?:\*\s+as\s+(\w+)))\s+from\s+(?:\'|\")(.+?)(?:\'|\")/gm
);

for (let m of imports_mat) {
	let file = getBasenamePrefix(m[5]);

	if (m[2]) { // import {AA} from '.'
		for (let it of m[2].split(',')) {
			let [ref,name=ref] = it.trim().split(/\s*as\s*/);
			imports[name] = { file, name, ref };
		}
	} else if (m[3]) { // import Default from '.'
		imports[m[3]] = { file, name: m[3], ref: 'default' };
	} else if (m[4]) { // import * as all from '.'
		imports[m[4]] = { file, name: m[4], ref: '*' };
	}
}

var comments_mat = code.matchAll(/\/\*(\*[\w\W]*?)\*\/|(\/\/\!\<[\w\W]*?$)/gm);

for (let m of comments_mat) {
	let comment;
	if (m[1]) {
		comment = parseCommentA(m[1], m.index);
	} else if (m[2]) {
		comment = parseCommentB(m[2], m.index);
	}
	if (comment)
		comments.push(comment);
}

console.log(imports);
comments.forEach(e=>console.log(e.__items__));
debugger;