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

import 'langou/util';
import 'langou/sys';
import 'langou/reader';
import 'langou/font';
import {
	GUIApplication, Root, Scroll, CSS, atomPixel as px,
	Div, Hybrid, Clip, Text, Button, TextNode as T, langou
} from 'langou';
import { NavpageCollection, Toolbar } from 'langou/nav';
import { Navbutton, Mynavpage } from './public';
import './examples';
import about_vx from './about';
import review_vx from './review';

var resolve = require.resolve;

CSS({
	
	'.category_title': {
		width: 'full',
		textLineHeight: 30,
		textColor: '#6d6d72',
		textSize: 14,
		margin: 16,
	},

	'.rm_margin_top': {
		marginTop: 0,
	},

	'.text_mark': {

	},
	
	'.hello': {
		width: 'full',
		textSize:46, 
		textAlign:"center",
		textColor:"#000",
		margin: 16,
		marginTop: 18,
		marginBottom: 18,
	},
	
	'.category': {
		width: 'full',
		borderTop: `${px} #c8c7cc`,
		// borderBottom: `${px} #c8c7cc`,
		backgroundColor: '#fff',
		// borderBottomWidth: 0,
	},

	'.toolbar_btn': {
		margin: 8,
		textFamily: 'icomoon-ultimate',
		textSize: 24,
	},

	'.codepre': {
		width:'full',
		margin:10,
		textColor:"#000",
	},

	'.codepre .tag_name': { textColor: '#005cc5' },
	'.codepre .keywork': { textColor: '#d73a49' },
	'.codepre .identifier': { textColor: '#6f42c1' },
	'.codepre .str': { textColor: '#007526' },
	
})

function review_code(evt) {
	evt.sender.owner.collection.push(review_vx(), 1);
}

const langou_tools = 'https://www.npmjs.com/package/lmake';
const langou_tools_issues_url = 'https://github.com/louis-tru/langou/issues';
const examples_source = 'https://github.com/louis-tru/langou.git';
const documents = 'http://langou.org/';

// registerFont

function handle_go_to(evt) {
	var url = evt.sender.url;
	if ( url ) {
		langou.app.openUrl(url);
	}
}

function handle_bug_feedback() {
	langou.app.sendEmail('louistru@hotmail.com', 'bug feedback');
}

class DefaultToolbar extends Toolbar {
	render() {
		return super.render(
			<Hybrid textAlign="center" width="full" height="full">
				<Button onClick=review_code>
					<Text class="toolbar_btn" value="\ue9ab" />
				</Button>
			</Hybrid>
		);
	}
}

const langou_tools_vx = ()=>(
	<Mynavpage title="Langou Tools" source=resolve(__filename)>
		<Div width="full">
			<Hybrid class="category_title">
`1. You can use nodejs <T textBackgroundColor="#ddd" value="npm install -g lmake" />.
2. Or get the node modules from Github.`
			</Hybrid>
			<Button class="long_btn rm_margin_top" onClick=handle_go_to url=langou_tools>Go Github</Button>
		</Div>
	</Mynavpage>
)

const examples_source_vx = ()=>(
	<Mynavpage title="Examples Source" source=resolve(__filename)>
		<Div width="full">
			<Text class="category_title" value="You can get the full examples source code from Github" />
			<Button class="long_btn rm_margin_top" onClick=handle_go_to url=examples_source>Go Github</Button>
		</Div>
	</Mynavpage>
)

const documents_vx = ()=>(
	<Mynavpage title="Documents" source=resolve(__filename)>
		<Div width="full">
			<Hybrid class="category_title">Now go to <T textColor="#0079ff" value="langou.org" /> to view the document?</Hybrid>
			<Button class="long_btn rm_margin_top" onClick=handle_go_to url=documents>Go Documents</Button>
		</Div>
	</Mynavpage>
)

const bug_feedback_vx = ()=>(
	<Mynavpage title="Bug Feedback" source=resolve(__filename)>
		<Div width="full">
			<Hybrid class="category_title">Now go to Github issues list?</Hybrid>
			<Button class="long_btn rm_margin_top" onClick=handle_go_to url=langou_tools_issues_url>Go Github Issues</Button>
			<Hybrid class="category_title">Or you can send me email, too.</Hybrid>
			<Button class="long_btn rm_margin_top" onClick=handle_bug_feedback>Send email</Button>
		</Div>
	</Mynavpage>
)

var app = new GUIApplication({
	multisample: 4,
	width: 420,
	height: 800,
	fullScreen: util.options.full_screen || 0,
	enableTouch: 1,
	background: 0xffffff,
	title: 'Langou Examples',
}).start(
	<Root>

		<NavpageCollection id="npc" defaultToolbar=DefaultToolbar>
			<Mynavpage title="Home" source=resolve(__filename)>

				<Scroll width="full" height="full" bounceLock=0>

					<Text class="hello" value="Hello." />
					<Div class="category" borderBottom=`${px} #c8c7cc`>
						<Hybrid class="codepre">`<T class="keywork" value="import"/> \{ <T class="identifier" value="GUIApplication" />, <T class="identifier" value="Root" /> \} <T class="keywork" value="from" /> <T class="str" value="'langou'" />
<T class="keywork" value="new"/> <T class="identifier" value="GUIApplication"/>()<T class="keywork" value="."/><T class="identifier" value="start"/>(
	\<<T class="tag_name" value="Root" />\>hello world!\</<T class="tag_name" value="Root" />\>
)`
						</Hybrid>
					</Div>

					<Text class="category_title" />
					<Clip class="category">
						<Navbutton next=examples.vx>Examples</Navbutton>
						<Navbutton next=examples_source_vx>Examples Source</Navbutton>
						<Navbutton next=langou_tools_vx>Langou Tools</Navbutton>
					</Clip>

					<Text class="category_title" />
					<Clip class="category">
						<Navbutton next=about_vx>About</Navbutton>
						<Navbutton next=documents_vx>Documents</Navbutton>
						<Navbutton next=bug_feedback_vx>Bug Feedback</Navbutton>
					</Clip>

					<Div height=32 width="full" />

				</Scroll>

			</Mynavpage>
		</NavpageCollection>
	</Root>
)

// register font icomoon-ultimate
font.registerFont( reader.readFileSync(resolve('./icomoon.ttf')) );

// console.log(app.displayPort.phyWidth)

var lock = Number(util.options.lock);
if (lock) {
	app.displayPort.lockSize(lock);
}
