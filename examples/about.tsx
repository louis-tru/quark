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

import { Jsx } from 'quark';
import { Page } from './tool';

const resolve = require.resolve;

export default (self: Page)=>{
	self.title = 'About';
	self.source = resolve(__filename);

	var code = `Quark
	===============
	
	Used C/C++/OpenGL/javascript to implement a GUI typesetting display engine and cross platform GUI application development framework
	Goal: developing GUI applications on this basis can take into account both the simplicity and speed of developing WEB applications, as well as the performance and experience of Native applications.
	
	使用C/C++/OpenGL/javascript实现的一个GUI排版显示引擎与跨平台GUI应用开发框架
	目标：在此基础上开发GUI应用程序可兼顾开发WEB应用程序的简单与速度同时拥有Native应用程序的性能与体验.
	
	Quark Source 
	===============
	https://github.com/louis-tru/quark.git
	
	Support
	===============
	http://quarks.cc
	louistru@hotmail.com`;

	return (
		<scroll width="match" height="match">
			<text width="match" margin={10} value={code} textWhiteSpace="preWrap" />
		</scroll>
	);
};
