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

import { P, M, LOG, AM, VM, VP, CA } from './test';
import { GUIApplication, Root, Div, Image } from 'ftr';
import dp from 'ftr/display_port';

new GUIApplication().start(
	<Root>
		<Div width={200} height={200} background_color="#f00">
			<Image src={resolve('res/cc.jpg')} width="full" height="full" opacity={0.5} />
		</Div>
	</Root>
).onLoad = function() {
	CA(test);
};

async function test() {

	LOG('\nTest Display Port:\n')

	const cur = dp.current;

	P(dp, 'current')
	P(dp, 'atomPixel')
	await AM(dp, 'nextFrame', [()=>1])
	M(cur, 'lockSize', [200,200])
	P(dp, 'atomPixel')
	await AM(cur, 'nextFrame', [()=>1])

	P(cur, 'width')
	P(cur, 'height')
	P(cur, 'phyWidth')
	P(cur, 'phyHeight')
	P(cur, 'bestScale')
	P(cur, 'scale')
	P(cur, 'scaleWidth')
	P(cur, 'scaleHeight')
	P(cur, 'rootMatrix')
	P(cur, 'atomPixel')

}

