/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

import { LOG, Mv } from './tool'
import * as css from 'quark/css'

const resolve = require.resolve

export default async function(_: any) {

	LOG('\nTest CSS:\n')

	Mv(css, 'createCss', [{
		'.test': {
			time: 0,
			border: '0 #000',
			borderWidth: 0,
			borderColor: '#000',
			borderRadius: 0,
			minWidth: 'auto',
			minHeight: 'auto',
			maxWidth: 'match',
			maxHeight: 'match',
			opacity: 1,
			visible: true,
			width: 'match',
			height: '90%',
			margin: 0,
			marginLeft: 0,
			marginTop: 0,
			marginRight: 0,
			marginBottom: 0,
			padding: 0,
			paddingLeft: 0,
			paddingTop: 0,
			paddingRight: 0,
			paddingBottom: 0,
			borderLeft: '0 #000',
			borderTop: '0 #000',
			borderRight: '0 #000',
			borderBottom: '0 #000',
			borderLeftWidth: 0,
			borderTopWidth: 0,
			borderRightWidth: 0,
			borderBottomWidth: 0,
			borderLeftColor: '#000',
			borderTopColor: '#000',
			borderRightColor: '#000',
			borderBottomColor: '#000',
			borderTopLeftRadius: 0,
			borderTopRightRadius: 0,
			borderBottomRightRadius: 0,
			borderBottomLeftRadius: 0,
			backgroundColor: '#000',
			align: 'center',
			textAlign: 'center',
			textBackgroundColor: '#ff0',
			textColor: '#f00',
			fontSize: 'inherit',
			fontWeight: 'thin',
			fontSlant: 'italic',
			fontFamily: 'inherit',
			lineHeight: 'inherit',
			textShadow: '2 2 2 #000',
			textDecoration: 'overline',
			textOverflow: 'ellipsis',
			wordBreak: 'breakWord',
			whiteSpace: 'noWrap',
			boxShadow: '2 2 2 #f00',
			src: resolve('./res/cc.jpg'),
			background: `image(${resolve('./res/0.jpg')})`,
			keyboardType: 'ascii',
			returnType: 'done',
			readonly: true,
			security: false,
			//
			translate: '0 0',
			scale: '1 1',
			skew: '0 0',
			origin: [0,0],
			x: 0,
			y: 0,
			scaleX: 0,
			scaleY: 0,
			skewX: 0,
			skewY: 0,
			originX: 0,
			originY: 0,
			rotateZ: 0,
		},
	}])

}