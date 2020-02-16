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

import {Propery} from './propery';
import {Curve} from './value';

export type Options = Action | Dict;

export declare abstract class Action {
	play(): void;
	stop(): void;
	seek(time: number): void;
	seekPlay(time: number): void;
	seekStop(time: number): void;
	clear(): void;
	readonly duration: number;
	readonly parent: Action | null;
	readonly looped: number;
	readonly delayed: number;
	playing: boolean;
	loop: number;
	delay: number;
	speed: number;
}

export declare abstract class GroupAction extends Action {
	readonly length: number;
	append(child: Action): void;
	insert(index: number, child: Action): void;
	removeChild(index: number): void;
	children(index: number): Action | null;
}

export declare class SpawnAction extends GroupAction {}
export declare class SequenceAction extends GroupAction {}

export declare class KeyframeAction extends Action {
	hasProperty(name: Propery): boolean;
	matchProperty(name: Propery): boolean;
	frame(index: number): Frame | null;
	add(style: Dict): Frame;
	add(time: number, curve?: Curve | string): Frame;
	readonly first: Frame | null;
	readonly last: Frame | null;
	readonly length: number;
	readonly position: number;
	readonly time: number;
}

export declare abstract class Frame {}