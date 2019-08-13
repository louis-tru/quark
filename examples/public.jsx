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

import { NavPage } from 'langou/nav';
import { 
	ViewController, Button, CSS, Hybrid,
	Text, atomPixel as px, 
	Indep, Panel, Scroll, langou
} from 'langou';
import 'langou/util';

var resolve = require.resolve;

// CSS(<Style>
//   .long_btn {
//     margin: 10;
//     margin_bottom: 0;
//     width: full;
//     height: 36;
//     text_line_height: 36;
//     text_color: #0079ff;
//     border_radius: 8;
//     border: ${px} #0079ff;
//   }
//   .long_btn2 {
//     margin: 10;
//     margin_bottom: 0;
//     width: full;
//     height: 36
//     text_line_height: 36;
//     text_color: #fff;
//     border_radius: 8;
//     border: ${px} #fff;
//   }
// </Style>);

CSS({
	
	'.long_btn': {
		margin: 10,
		marginBottom: 0,
		width: "full",
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
		width: "full",
		height: 36,
		textLineHeight: 36,
		textColor: "#fff",
		borderRadius: 8,
		border: `${px} #fff`,
	},
	
	'.next_btn': {
		width: "full",
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
		width:"full",
		height:30,
		backgroundColor:"#eee",
		borderRadius:8,
	},

})

export class Navbutton extends ViewController {
	
	render(...vdoms) {
		//util.log('---------------------', px);
		return (
			<Button
				onClick="handle_click"
				class="next_btn"
				textColor="#0079ff"
				defaultHighlighted=0
				borderBottom=`${px} #c8c7cc`>
				<Hybrid marginLeft=16 marginRight=50>{vdoms}</Hybrid>
				<Indep x=-10 alignX="right" alignY="center">
					<Text value="\uedbe" textFamily="icomoon-ultimate" textColor="#aaa" />
				</Indep>
			</Button>
		);
	}
	
	handle_click(evt) {
		if (!this.next) return;
		var next = this.next();
		if ( ViewController.typeOf(next, Mynavpage) ) {
			var ctr = this.owner;
			while (ctr) {
				if ( ctr instanceof Mynavpage ) {
					ctr.collection.push(next, 1); break;
				}
				ctr = ctr.owner;
			}
		}
		// console.log('nav button click');
	}
}

export class Mynavpage extends NavPage {
	source = resolve(__filename);
	m_backgroundColor = '#f8f8f8';

	triggerForeground(e) {
		super.triggerForeground(e);
		/*
		// test TV keyboard
		var btn = null;
		var first = this.view.first;
		if (first && first instanceof Panel) {
			first.allow_leave = true;
			if ( first instanceof Scroll ) {
				first.enable_switch_scroll = true;
			}
			btn = first.first_button();
		}
		if ( !btn ) {
			btn = langou.root.first_button();
		}
		if ( btn ) {
			btn.focus();
		}
		*/
	}

}
