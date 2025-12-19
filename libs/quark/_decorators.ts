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

const JscRejectionListener = (globalThis as any)._rejectionListener; // unhandledrejection for jsc

// redefine class decorator to hook unhandledrejection
export const __jscAsync = JscRejectionListener ? function(target: any, key: string, desc: PropertyDescriptor) {
	const oriFn = target[key] as (...args: any[]) => Promise<any>;
	desc.value = function(this: any, ...args: any[]) {
		const result = oriFn.apply(this, args);
		(result as any)._hookUnhandledrejection();
		return result;
	};
	return desc;
}: function(target: any, key: string, desc: PropertyDescriptor) {
};

// wrap async function to hook unhandledrejection
export const __wrapAsync = JscRejectionListener ?
function <F extends (...args: Args) => Promise<R>, Args extends any[], R>(fn: F)
		: (...args: Args) => Promise<R>
{
	return function(this: any, ...args: Args): Promise<R> {
		const result = fn.apply(this, args);
		(result as any)._hookUnhandledrejection();
		return result;
	};
}: function <F extends (...args: Args) => Promise<R>, Args extends any[], R>(fn: F)
		: (...args: Args) => Promise<R>
{
	return fn;
};

export function __decorate(decorators: any, target: any, key?: any, desc?: any) {
	var c = arguments.length,
			r = c < 3 ? target : desc || Object.getOwnPropertyDescriptor(target, key);
	for (var i = decorators.length - 1; i >= 0; i--) {
		const d = decorators[i];
		if (d) {
			r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
		}
	}
	if (c > 3 && r)
		Object.defineProperty(target, key, r);
	return r;
}

const __createBinding = (Object as any).create ? (function(o: any, m: any, k?: any, k2?: any) {
	if (k2 === undefined) k2 = k;
	var desc = Object.getOwnPropertyDescriptor(m, k);
	if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
		desc = { enumerable: true, get: function() { return m[k]; } };
	}
	Object.defineProperty(o, k2, desc);
}) : (function(o: any, m: any, k: any, k2: any) {
	if (k2 === undefined)
		k2 = k;
	o[k2] = m[k];
});

export function __exportStar(m: any, exports: any) {
	for (var p in m)
		if (p !== "default" && !Object.prototype.hasOwnProperty.call(exports, p))
			__createBinding(exports, m, p);
};

(globalThis as any).__decorate = __decorate;
(globalThis as any).__exportStar = __exportStar;