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

import { _CVD } from 'quark';
import { HighlightedStatus, HighlightedEvent } from 'quark/event';
import { Navbar } from 'quark/nav';
import { Page } from './tool';
import {toolbar} from './review';

const resolve = require.resolve;

export default (self: Page)=>{
	if (!self.isMounted) {
		self.navbar = (
			<Navbar backTextColor="#fff" titleTextColor="#fff" />
		);
		self.title = 'Action';
		self.source = resolve(__filename);
		self.backgroundColor = '#333';
	}

	function highlighted(evt: HighlightedEvent) {
		var img1 = self.asRef('img1');
		var img2 = self.asRef('img2');
		var speed = 1;
		if ( evt.status == HighlightedStatus.Active ) {
			speed = img1 === evt.sender ? 2 : 0.5;
		}
		img1.action!.speed = speed;
		img2.action!.speed = speed;
	}

	return (
		<free width="match" height="match">
			<matrix width={600} align="centerMiddle" y={-15} opacity={0.5}>
				<matrix
					onHighlighted={highlighted} 
					action={{
						keyframe:[
							{ rotateZ: 0, time:0, curve:'linear' },
							{ rotateZ: -360, time: 4000, curve:'linear' },
						],
						loop: 1e8,
						playing: true,
					}}
					y={56}
					origin={[300,300]}
					align="center"
					ref="img1"
				>
					<image src={resolve('./gear0.png')} width={600} />
				</matrix>
				<matrix
					onHighlighted={highlighted}
					action={{
						keyframe: [
							{ rotateZ: 22.5, time:0, curve:'linear' }, 
							{ rotateZ: 22.5 + 360, time: 2000, curve:'linear' },
						],
						loop: 1e8,
						playing: true,
					}}
					origin={[180.5,180.5]}
					align="center"
					ref="img2"
				>
					<image src={resolve('./gear1.png')} width={361} />
				</matrix>
			</matrix>

			{toolbar(self)}
		</free>
	)
}
