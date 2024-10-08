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

import { mainScreenScale, createCss, StyleSheet } from 'quark';
import { _CVD, ViewController, VirtualDOM, assertDom, link, VDom, RenderResult } from 'quark/ctr';
import { NavPage } from 'quark/nav';
import {ClickEvent} from 'quark/event';
import * as types from 'quark/types';


const px = 1 / mainScreenScale();
const resolve = require.resolve;

createCss({

	'.long_btn': {
		margin: 10,
		marginBottom: 0,
		width: "match",
		height: 36,
		textLineHeight: 36,
		textColor: "#0079ff",
		borderRadius: 8,
		border: `${px} #0079ff`,
		// border: `2 #0079ff`,
		// backgroundColor: '#f00',
		// borderRadius: 80,
		// border: `40 #0079ff`,
		// borderLeftColor: '#f00',
		// borderRightColor: '#f00',
	},

	'.long_btn2': {
		margin: 10,
		marginBottom: 0,
		width: "match",
		height: 36,
		textLineHeight: 36,
		textColor: "#fff",
		borderRadius: 8,
		border: `${px} #fff`,
	},

	'.next_btn': {
		width: 'match',
		textLineHeight: 45,
		textAlign: "left",
		borderRadius: 0,
	},

	'.next_btn:normal': {
		backgroundColor: '#fff0', time: 180
	},
	'.next_btn:hover': {
		backgroundColor: '#ececec', time: 50
	},
	'.next_btn:down': {
		backgroundColor: '#E1E4E4', time: 50
	},

	'.input': {
		margin:10,
		marginBottom:0,
		width:"match",
		height:30,
		backgroundColor:"#eee",
		borderRadius:8,
	},

})

export class NavButton extends ViewController<{style?: StyleSheet, next?: (self: Page)=>RenderResult}> {
	@link next?: (self: Page)=>RenderResult;

	render() {
		return (
			<button
				onClick={this._handleClick}
				class="next_btn"
				textColor="#0079ff"
				borderBottom={`${px} #c8c7cc`}
				style={this.props.style}
			>
				<text marginLeft={16} marginRight={50}>{this.children}</text>
				<matrix x={-10} align="rightMiddle">
					<text value={'\uedbe'} textFamily="icomoon-ultimate" textColor="#aaa" />
				</matrix>
			</button>
		);
	}

	private _handleClick = (e: ClickEvent)=>{
		if (!this.next)
			return;

		const render = this.next;
		class MyPage extends Page {
			renderBody(): RenderResult {
				return render(this);
			}
		}

		let ctr = this.owner;
		while (ctr) {
			if ( ctr instanceof Page ) {
				ctr.collection.push(<MyPage />, true);
					break;
			}
			ctr = ctr.owner;
		}
	}
}

export class Page extends NavPage<{source?: string}> {
	@link
	source = resolve(__filename);
	backgroundColor: types.ColorStrIn = '#f8f8f8';
}