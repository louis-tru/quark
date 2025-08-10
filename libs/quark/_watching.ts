/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
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

import {WSClient, WSConversation} from './ws';
import {URL} from './path';
import {_Package,Module} from "./pkg";
import {EventNoticer,Event} from "./event";

let mainPkg: _Package;
let mainPkgLocal: _Package|undefined;
let modules: Map<string, Module> = new Map();

function markModuleObject(v: any, filename: string) {
	if (v !== undefined) {
		if (v && typeof v == 'object')
			v.__filename__ = filename;
		return true;
	}
	return false;
}

function watchModule(mod: Module) {
	let pkg = mod.package;
	if (!pkg)
		return;
	if (pkg !== mainPkg && pkg !== mainPkgLocal)
		return;
	let filename = mod.filename.substring(pkg.path.length + 1);
	let oldMod = modules.get(filename);

	if (oldMod) {
		if (oldMod !== mod)
			throw new Error(`Module already exists: ${filename}, but different instance.`);
	}
	let ok = false;

	for (let v of Object.values<any>(mod.exports)) {
		ok = markModuleObject(v, filename) || ok;
	}

	if (ok)
		modules.set(filename, mod);
}

type Noticer = EventNoticer<Event<{},{name:string,hash:string}>>;

// Connect to the watching server
export function connectServer(pkg: _Package, notice: Noticer) {
	if (!pkg.isHttp || !pkg.json.watching)
		return;
	mainPkg = pkg;
	mainPkgLocal = (pkg as any)._local;

	let url = new URL(pkg.path);
	let cli = new WSClient('Message', new WSConversation(url.origin));

	cli.conv.autoReconnect = 5e2; // reconnect every 500ms after disconnect

	cli.addEventListener('FileChanged', e=>{
		let {name, hash} = e.data as {name: string, hash: string};
		let mod = modules.get(name);
		if (mod) {
			let filename = `${mainPkg.path}/${name}`;
			try {
				(mod as any).loaded = false; // force reload
				(mod as any)._load(`${filename}?${hash}`, filename);
			} catch (error) {
				console.error(`Error reloading module ${name}:`, error);
				(mod as any).loaded = true; // restore to last loaded state
				return;
			}
			notice.trigger({name,hash});
		}
	});

	cli.onLoad.on(()=>{
		console.log('Connected to watching server');
	});

	return watchModule;
}
