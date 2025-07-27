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

function getBasenamePrefix(str) {
	var basename = path.basename(str);
	var ext = path.extname(str);
	return basename.substring(0, basename.length - ext.length);
}

function escapingFilePath(file) {
	return file.replace(/^_/, '$');
}

function key(s) {
	return `\`${s}\``;
}

function head(s) {
	return `* \`@${s}\``;
}

function startExec(input,output) {
	var name = getBasenamePrefix(input);
	var code = fs.readFileSync(input, 'utf-8');
	var imports = {};
	var comments = [];
	var packIn = null;
	var doc = [];

	output = output || `${__dirname}/../out/md/${name}.md`;

	var refs = {
		// Object: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object',
		Object: '_ext.md#object',
		// Date: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date',
		Date: '_ext.md#date',
		RegExp: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/RegExp',
		// Function: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Function',
		Function: '_ext.md#function',
		ArrayBuffer: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer',
		TypedArray: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/TypedArray',
		Uint8Array: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Uint8Array',
		Int8Array: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Int8Array',
		// Array: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array',
		Array: '_ext.md#array',
		// String: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String',
		String: '_ext.md#string',
		// Number: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number',
		Number: '_ext.md#number',
		// Int: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number',
		Int: '_ext.md#int',
		Uint: '_ext.md#uint',
		Int8: '_ext.md#int8',
		Uint8: '_ext.md#uint8',
		Int16: '_ext.md#int16',
		Uint16: '_ext.md#uint16',
		// Float: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number',
		Float: '_ext.md#float',
		// Boolean: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Boolean',
		Boolean: '_ext.md#boolean',
		// Error: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Error',
		Error: '_ext.md#error',
		null: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/null',
		undefined: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/undefined',
		// string: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String',
		string: '_ext.md#string',
		// object: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object',
		object: '_ext.md#object',
		// number: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number',
		number: '_ext.md#number',
		// boolean: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Boolean',
		boolean: '_ext.md#boolean',
		void: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/void',
		this: 'https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/this',
	};

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

	////////////// parse types //////////////

	function parseTypesList(args, comment) {
		let typesList = [];
		do {
			let item = {types: [], defaultTypes: []};
			typesList.push(item);
			parseTypes(args, item.types, comment);

			reg = /\s*\=\s*(\{[^\}]*\})/y; // =default
			reg.lastIndex = args.index;
			mat = args.typeStr.match(reg);
			if (mat) {
				args.index = mat.index + mat[0].length;
				if (mat[1]) {
					item.defaultTypes = [{ first: mat[1], isConstObject: true }];
				} else {
					parseTypes(args, item.defaultTypes, comment);
				}
			}
			reg = /\s*,/y;
			reg.lastIndex = args.index;
			mat = args.typeStr.match(reg);
		} while(mat ? (args.index = mat.index + mat[0].length, true): false);
		return typesList;
	}

	function parseTypesTempls(args, comment) {
		let templates = [];
		let reg = /\s*\</y;
		reg.lastIndex = args.index;
		let mat = args.typeStr.match(reg);
		if (mat) {
			args.index = mat.index + mat[0].length;
			templates = parseTypesList(args);
			reg = /\s*\>/y;
			reg.lastIndex = args.index;
			mat = args.typeStr.match(reg);
			if (mat) {
				args.index = mat.index + mat[0].length
			} else {
				if (comment && comment.__main__) {
					let main = comment.__main__;
					console.warn(`Type syntax error: ${main.kind} ${main.value}, ${args.typeStr}`);
				} else {
					console.warn(`Type syntax error: ${args.typeStr}`);
				}
			}
		}
		return templates;
	}

	function parseTypes(args, types, comment) {
		let reg = /\s*([\w\.]+|\'[^\']+\'|\"[^\"]+\"|\[[^\]]+\]|\{[^\}]+\})\s*/y;
		reg.lastIndex = args.index;
		let mat = args.typeStr.match(reg);
		if (!mat) {
			return;
		}
		args.index = mat.index + mat[0].length;

		if (mat[1][0] == "'" || mat[1][0] == '"') {
			types.push({
				first: mat[1].substring(1, mat[1].length - 1),
				isConstString: true,
			});
		} else if (mat[1][0] == '[' || mat[1][0] == '{') {
			types.push({
				first: mat[1],
				isConstArray: mat[1][0] == '[',
				isConstObject: mat[1][0] == '{',
			});
		} else {
			types.push({
				first: mat[1],
				templates: parseTypesTempls(args, comment),
			});
		}

		reg = /\s*\[\s*\]/y;
		reg.lastIndex = args.index;
		mat = args.typeStr.match(reg);
		if (mat) {
			args.index = mat.index + mat[0].length;
			types.at(types.length - 1).isArray = true;
		}

		reg = /\s*\|/y;
		reg.lastIndex = args.index;
		mat = args.typeStr.match(reg);
		if (mat) {
			args.index = mat.index + mat[0].length;
			parseTypes(args, types, comment);
		}
	}

	////////////// get types link //////////////

	function getTypeLinkBy(types, linkStr, comment) {
		types.forEach(({first,templates,isArray,isConstString,isConstArray,isConstObject},i)=>{
			if (i)
				linkStr.push('|');
			if (isConstString) {
				linkStr.push(`\`\"${first}"\``);
			} else if (isConstArray || isConstObject) {
				linkStr.push(`\`${first}\``);
			} else if (comment && first == '...') {
				linkStr.push(' {\n');
				let isType = comment.__main__.kind == 'type';
				let separator = isType ? ';': ',';
				for (let ch of comment.__child__ || []) {
					ch.disable = true;
					let {__main__:it,firstMsgs} = ch;
					// ...firstMsgs, ...it.msgs
					linkStr.push(`${key(it.name)}: ${getTypeLink(it.type)}${separator}${it.desc ? '// '+ it.desc: ''} \n`);
				}
				linkStr.push('}');
			} else {
				first = first.trim();
				if (!refs[first]) {
					let [refName,second,...suffix] = first.split(/\s*\.\s*/);
					let item = imports[refName];
					if (item) {
						let file = escapingFilePath(item.file);
						if (item.ref == '*') {
							refs[first] = `${file}.md#${(second||'').toLowerCase()}`;
						} else if (item.ref == 'default') {
							refs[first] = `${file}.md#default${second?'-'+second.toLowerCase():''}`;
						} else {
							refs[first] = `${file}.md#${item.ref.toLowerCase()}${second?'-'+second.toLowerCase():''}`;
						}
					}
				}
				if (refs[first]) {
					linkStr.push(`[\`${first}\`]`);
				} else {
					linkStr.push(`\`${first}\``);
				}
				if (templates && templates.length) {
					linkStr.push('<');
					templates.forEach(({types,defaultTypes},i)=>{
						if (i) {
							linkStr.push(',');
						}
						getTypeLinkBy(types, linkStr);
						if (defaultTypes.length) {
							linkStr.push('=');
							getTypeLinkBy(defaultTypes, linkStr);
						}
					});
					linkStr.push('>');
				}
				if (isArray)
					linkStr.push('`[]`');
			}
		});
	}

	function getTypeLink(typeStr, comment) {
		let linkStr = [];
		if (!typeStr)
			return '';
		let types = [];
		parseTypes({typeStr,index:0}, types, comment);
		getTypeLinkBy(types, linkStr, comment);
		return linkStr.join('');
	}

	function getTypeListLink(typeStr, comment) {
		let linkStr = [];
		if (!typeStr)
			return '';
		parseTypesList({typeStr,index:0}, comment).forEach(({types},i)=>{
			if (i)
				linkStr.push(',');
			getTypeLinkBy(types, linkStr);
		});
		return linkStr.join('');
	}

	function getTemplTypeLink(typesStr) {
		let linkStr = [];
		if (!typesStr.length)
			return '';

		parseTypesList({ typeStr: typesStr.join(','), index:0 }).forEach(({types,defaultTypes},i)=>{
			if (i) {
				linkStr.push(',');
			}
			linkStr.push(key(types[0].first));
			if (defaultTypes.length) {
				linkStr.push('=');
				getTypeLinkBy(defaultTypes, linkStr);
			}
		});
		return linkStr.join('');
	}

	///////////////////////////////////////////

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
			templMat = templMat.trim();
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
		let re = comment.return || comment.return;
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
					comment.return = re = new Item('return', '', {type:valueMat[3]});
					comment.__items__.push(comment.return);
				} else if (!comment.return.type) {
					comment.return.type = valueMat[3];
				}
			}
			comment.__main__ = method;
		}

		// Match to type: ([\w\[\]\<\>\|,\.\s]+)
		let reg = /\W*(?:(export|static|protected|private|public)\s+)?(default\s+)?(?:(async|declare)\s+)?(function|get|set)?(?:\s+(\w+)\s*)?(\<[^\>]+\>)?\s*\(([\w\[\]\<\>,\.\?:\|\s]*)\)\s*(?::\s*([\w\[\]\<\>\|,\.\s]+))?/my;
		reg.lastIndex = lastIndex;
		let mat = code.match(reg);
		if (!mat)
			return;
		let [,modifiers,defaultMat,asyncMat,key,name,templMat,argsMat,returnV] = mat;

		if (packIn) {
			if (modifiers == 'export' || key == 'function' || defaultMat) {
				packIn = null; // cancel pack in
				comment.__packIn__ = null;
			}
		} else {
			if (modifiers && modifiers != 'export')
				return;
			if (key != 'function')
				return;
		}

		if (defaultMat) {
			name = 'default';
		}

		if (!name)
			return;

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
			comment.__items__.unshift(item)
			comment.__main__ = item;
			comment[kind] = item;
			return item;
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
			item = item || newMainItem(kind,value,{name});
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
		let reg = /[^\w@]*(?:(export|static|protected|private|public|@event|@link|@link\.acc)\s+)?(?:(const|readonly)\s+)?(?:(\w+\??)|(\[[\w:\s]+\]))\s*(\:\s*([\w\[\]\<\>\|,\.\s]+)|\(|\w)?/my;
		reg.lastIndex = lastIndex;
		let mat = code.match(reg);
		if (!mat)
			return;
		let [,modifiers,key,name='indexed',indexed,_t,type] = mat;

		if (_t && _t[0] != ':')
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
			type ? 'getset': 'enumItem';
		if (!item) {
			item = new Item(kind,'',{name:name,type});
			comment[kind] = item;
			comment.__items__.unshift(item);
			comment.__main__ = item;
		} else {
			if (!item.name)
				item.name = name;
			if (!item.type)
				item.type = type;
		}

		if (comment.__packIn__) {
			let kind = comment.__packIn__.__main__.kind;
			if (kind == 'const' || kind == 'type' || kind == 'enum') {
				if (!comment.__packIn__.__child__)
					comment.__packIn__.__child__ = [];
				comment.__packIn__.__child__.push(comment);
			}
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
			let valueMat = item.value.match(/(\w+)(\:([\w\[\]\<\>\|,\.\'\"-\$　\(\)\?]+))?/);
			if (valueMat) {
				if (valueMat[3]) {
					item.type = valueMat[3];
				}
				item.name = valueMat[1];
			}
			comment.__main__ = item;
		}

		// Match to type: ([\w\[\]\<\>\|,\.\s]+)
		let reg = /\W*(?:(export)\s+)?(type)\s+(\w+)\s*(\<[^\>]+\>)?\s*=\s*([\w\[\]\<\>\|,\.\'\"\-\$　\s]+)/my;
		reg.lastIndex = lastIndex;
		let mat = code.match(reg);
		if (!mat)
			return;
		let [,modifiers,kind,name,templMat,type] = mat;

		if (!comment.type) {
			comment.type = new Item(kind,'',{name,type});
			comment.__items__.unshift(comment.type);
			comment.__main__ = comment.type;
		} else {
			if (!comment.type.name)
				comment.type.name = name;
			if (!comment.type.type)
				comment.type.type = type;
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
		if (pickItem) {
			pickItem.name = pickItem.value;
			comment.__main__ = pickItem;
		}

		// Match to type: ([\w\[\]\<\>\|,\.\s]+)
		let reg = /\W*(?:(export)\s+)?(?:(declare)\s+)?(?:(class|interface|enum)\s+)(\w+)(\s*\<[^\>]+\>)?(?:\s+extends\s+([\w\[\]\<\>\|,\.\s]+))?(?:\s+implements\s+([\w\[\]\<\>\|,\.\s]+))?/my;
		reg.lastIndex = lastIndex;
		let mat = code.match(reg);
		if (!mat)
			return;
		let [,modifiers,,kind,name,templMat,extend,implements] = mat;

		if (!pickItem) {
			comment[kind] = pickItem = new Item(kind,name,{name});
			comment.__items__.unshift(pickItem);
			comment.__main__ = pickItem;
		} else if (!pickItem.name) {
			pickItem.name = name;
		}

		fixTempl(comment, templMat);

		for (let [k,v] of [['extends',extend],['implements',implements]]) {
			if (v && !comment[k]) {
				comment[k] = new Item(k,v);
				comment.__items__.push(comment[k]);
			}
		}

		pickItem.set_modifiers(modifiers);

		packIn = comment;
		comment.__packIn__ = null;

		return comment;
	}

	function replaceLink(str) {
		return str.replace(/(?<!\:\s+|\]\()(https?:\/\/(\w+\.\w+)[^\s\'\"\`\(\)\[\]]*)/ig, '[`$1`]($1)');
	}

	function parseCommentA(str, index) { /** ... */
		let len = str.length;
		let lines = str.replaceAll('\\\n', '').split('\n').map(e=>e.replace(/\s*\*?\s?/, ''));
		let lastIndex = index + len + 4;

		let items = [
			// {kind: class|interface|enum|method|getset|type|callbackconst|param|return|example}
		];
		let comment = {
			firstMsgs: [],
			param: [],
			__items__: items,
			__main__: null,
		};

		if (lines.at(lines.length - 1) == '')
			lines.pop();

		for (let line of lines) {
			if (line[0] == '@') {
				let m = line.match(/^@(\w+)(\s+([^ \t\r]+))?(\s+(.+))?/);
				let kind = m[1];
				if (kind == 'returns')
					kind = 'return';
				let item = new Item(kind,m[3],{desc:m[5]});
				if (Array.isArray(comment[kind])) {
					comment[kind].push(item);
				} else {
					comment[kind] = item;
				}
				items.push(item);
			} else if (items.length) {
				items.at(items.length-1).msgs.push(replaceLink(line));
			} else if (line) {
				comment.firstMsgs.push(replaceLink(line));
			}
		}

		for (let pa of comment.param) {
			let [,name,type] = pa.value.match(/(\w+\??)(?::([\w\[\]\|]+))?/);
			pa.name = name;
			pa.type = type;
		}

		let template = comment.template;
		if (template && template.value) {
			template.types = template.value.split(',');
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
			tryTypeOrGetset(comment, comment.const, lastIndex);
		}
		else if (comment.default) {
			packIn = comment;
			items[0].value = 'default';
			items[0].name = 'default';
			comment.__main__ = items[0];
		}
		else if (comment.end) {
			packIn = null;
			return;
		}
		else if (comment.information) {
			packIn = null;
			comment.__main__ = items[0];
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

		return comment;
	}

	function parseCommentB(str, index) { //!< ...
		let lastIndex = code.lastIndexOf('\n', index);
		if (lastIndex != -1) {
			let items = [];
			let comment = {
				firstMsgs: [],
				param: [],
				__packIn__: packIn,
				__items__: items,
				__main__: null,
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
		/import\s+((?:(\w+)\s*,\s*)?\{([\w$_,\s]+)\}|(?:\*\s+as\s+(\w+)))\s+from\s+(?:\'|\")(.+?)(?:\'|\")/gm
	);

	for (let m of imports_mat) {
		let file = getBasenamePrefix(m[5]);

		if (m[4]) { // import * as all from '.'
			imports[m[4]] = { file, name: m[4], ref: '*' };
		} else {
			if (m[2]) { // import Default from '.'
				imports[m[2]] = { file, name: m[2], ref: 'default' };
			}
			if (m[3]) { // import {AA} from '.'
				for (let it of m[3].split(',')) {
					let [ref,name=ref] = it.trim().split(/\s+as\s+/);
					imports[name] = { file, name, ref };
				}
			}
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

	fs.mkdirSync(path.dirname(output), {recursive: true});

	if (!comments[0] || !comments[0].information) {
		let information = new Item('information');
		comments.unshift({
			firstMsgs: [],
			param: [],
			__packIn__: packIn,
			__items__: [information],
			__main__: information,
		});
	}

	for (let comment of comments) {
		let {__items__,__packIn__} = comment;
		let pack = __packIn__ && __packIn__.__main__;
		for (let it of __items__) {
			switch (it.kind) {
				case 'class':
				case 'interface':
				case 'enum':
					refs[it.name] = `#${it.kind}-${it.name.toLowerCase()}`;
					break;
				case 'method':
					refs[`${pack?pack.name+'.':''}${it.value}`] =
						`#${pack?pack.name.toLowerCase()+'-':''}${it.value.toLowerCase().replace(/[^\w]+/g, '-')}`;
					break;
				case 'getset':
					// doc.push(`### ${pack.name.toLowerCase()}.${it.name}`, ...firstMsgs);
					break;
				case 'callback':
				case 'type':
				case 'const':
					refs[it.name] = `#${it.name.toLowerCase()}`;
					break;
			}
		}
	}

	for (let comment of comments) {
		let {disable,__items__,__packIn__,firstMsgs,__main__} = comment;
		let pack = __packIn__ && __packIn__.__main__;
		if (disable)
			continue;
		if (__items__[0] !== __main__) {
			__items__.unshift(__main__);
		}
		for (let it of __items__) {
			if (it.finished)
					continue;
			switch (it.kind) {
				case 'class':
				case 'interface':
				case 'enum':
					doc.push(`## ${it.kind[0].toUpperCase() + it.kind.substring(1)}: ${it.name}`);
					doc.push(`### ${it.name}`, ...firstMsgs);
					it.desc && doc.push(it.desc);
					doc.push(...it.msgs);
					for (let ch of comment.__child__|| []) {
						let {enumItem:it,firstMsgs} = ch;
						ch.disable = true;
						doc.push(`* ${key(it.name)} ${it.type ? ' = ' + key(it.type): ''} ${it.desc}`, ...firstMsgs, ...it.msgs);
					}
					break;
				case 'extends':
				case 'implements':
					if (comment.class || comment.interface) {
						doc.push(`${head(it.kind)} ${getTypeListLink(it.value, comment)}`);
					}
					break;
				case 'template':
					doc.push(`${head('template')} <${getTemplTypeLink(it.types)}>`);
					break;
				case 'enumItem':
					break;
				case 'method':
					doc.push(`##${pack?'# '+pack.name.toLowerCase()+'.':' '}${it.value}`, ...firstMsgs);
					it.desc && doc.push(it.desc);
					doc.push(...it.msgs);
					break;
				case 'return':
					if (comment.method) {
						doc.push(`${head('return')} ${getTypeLink(it.type)} ${it.desc}`, ...it.msgs);
					}
					break;
				case 'get':
				case 'set':
				case 'event':
					doc.push(`### ${pack.name.toLowerCase()}.${it.name}`, ...firstMsgs);
					doc.push(`${head(it.kind)} ${key(it.name)}: ${getTypeLink(it.type)} ${it.desc}`);
					break;
				case 'getset':
					doc.push(`### ${pack.name.toLowerCase()}.${it.name}`, ...firstMsgs);
					doc.push(`* ${key(it.name)}: ${getTypeLink(it.type)} ${it.desc}`);
					break;
				case 'param':
					doc.push(`${head('param')} ${key(it.name)}: ${getTypeLink(it.type)} ${it.desc}`, ...it.msgs);
					break;
				case 'callback':
					doc.push(`## ${it.name}`, ...firstMsgs);
					doc.push(`${head('callback')} ${key(it.value)} ${it.desc}`, ...it.msgs);
					break;
				case 'type':
					doc.push(`## ${it.name}`, ...firstMsgs);
					it.desc && doc.push(it.desc);
					doc.push(...it.msgs);
					doc.push(`${head('type')} ${key(it.name)} = ${getTypeLink(it.type, comment)}`);
					break;
				case 'const':
					doc.push(`## ${it.name}`, ...firstMsgs);
					it.desc && doc.push(it.desc);
					doc.push(...it.msgs);
					doc.push(`${head('const')} ${key(it.name)}: ${getTypeLink(it.type, comment)}`);
					break;
				case 'default':
					doc.push(`## default`, ...firstMsgs, ...it.msgs);
					break;
				case 'information':
					doc.push(`# ${it.value||'quark/'+name}`, ...firstMsgs, it.desc, ...it.msgs);
					break;
				case 'example':
					doc.push('', `For example:`, ...it.msgs);
					break;
				case 'end':
					break;
				default:
					doc.push(`${head(it.kind)} ${it.value} ${it.desc}`, ...it.msgs);
					break;
			}
			it.finished = true;
		}
		doc.push('');
	}

	for (let [k,v] of Object.entries(refs)) {
		doc.push(`[\`${k}\`]: ${escapingFilePath(v)}`);
	}

	fs.writeFileSync(output, doc.join('\n'));
}

exports.getBasenamePrefix = getBasenamePrefix;
exports.escapingFilePath = escapingFilePath;
exports.startExec = startExec;

if (require.main === module) {
	let main_input = process.argv[2];
	if (main_input && fs.existsSync(main_input)) {
		startExec(main_input, process.argv[3]);
	}
}