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

import util from 'quark/util';
import * as reader from 'quark/reader';
import * as font from 'quark/font';
import { Application } from 'quark/app';

var app = new Application({
	multisample: 4,
	width: 420,
	height: 800,
	fullScreen: !!util.options.full_screen,
	enableTouch: true,
	background: 0xffffff,
	title: 'Quark Examples',
});

import {
	Root, Scroll, Div, Hybrid, Clip, Text, Button, TextNode as T, default as quark, _CVD
} from 'quark';
import { NavPageCollection, Toolbar } from 'quark/nav';
import { Navbutton, Mynavpage, Page } from './public';
import examples from './examples';
import about_vx from './about';
import review_vx from './review';
import {ClickEvent} from 'quark/event';

const resolve = require.resolve;
const px = quark.atomPixel;

quark.css({

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
		'width': 'full',
		'margin': 10,
		// 'textColor': '#000',
		':normal': {
			textColor: '#000'
		},
		':hover': {
			textColor: '#ff0'
		},
		':down': {
			textColor: '#f00',
		},
		'.tag_name': {
			textColor: '#005cc5'
		},
		'.keywork': {
			textColor: '#d73a49'
		},
		'.identifier': {
			textColor: '#6f42c1'
		},
		'.str': {
			textColor: '#007526'
		},
	},

	'.codepre.a': {
	},

	'.codepre.a.b': {
	},

	// '.codepre .tag_name': { textColor: '#005cc5' },
	// '.codepre .keywork': { textColor: '#d73a49' },
	// '.codepre .identifier': { textColor: '#6f42c1' },
	// '.codepre .str': { textColor: '#007526' },

	'.keywork': { textColor: '#c73a45' },
})

function review_code(evt: ClickEvent) {
	evt.sender.ownerAs<Page>().collection.push(review_vx(), true);
}

const quark_tools = 'https://www.npmjs.com/package/noproj';
const quark_tools_issues_url = 'https://github.com/louis-tru/quark/issues';
const examples_source = 'https://github.com/louis-tru/quark.git';
const documents = 'http://quarks.cc/';

// registerFont

function handle_go_to(evt: ClickEvent) {
	var url = (evt.sender as any).url;
	if ( url ) {
		quark.app.openUrl(url);
	}
}

function handle_bug_feedback() {
	quark.app.sendEmail('louistru@hotmail.com', 'bug feedback');
}

class DefaultToolbar extends Toolbar {
	render() {
		return super.render(
			<Hybrid textAlign="center" width="full" height="full">
				<Button onClick={review_code}>
					<Text class="toolbar_btn" value={"\ue9ab"} />
				</Button>
			</Hybrid>
		);
	}
}

const quark_tools_vx = ()=>(
	<Mynavpage title="Quark Tools" source={resolve(__filename)}>
		<Div width="full">
			<Hybrid class="category_title">
1. You can use nodejs <T textBackgroundColor="#ddd" value={"npm install -g noproj\n"} />.
2. Or get the node modules from Github.
			</Hybrid>
			<Button class="long_btn rm_margin_top" onClick={handle_go_to} url={quark_tools}>Go Github</Button>
		</Div>
	</Mynavpage>
)

const examples_source_vx = ()=>(
	<Mynavpage title="Examples Source" source={resolve(__filename)}>
		<Div width="full">
			<Text class="category_title" value="You can get the full examples source code from Github" />
			<Button class="long_btn rm_margin_top" onClick={handle_go_to} url={examples_source}>Go Github</Button>
		</Div>
	</Mynavpage>
)

const examples_source_vx_test = ()=>(
	<Mynavpage title="Examples Source" source={resolve(__filename)}>
		{/* :normal w400 tcf01 :hover w500 :down w550 */}
		<Div style="wfull h100 b1#fffsold tcf00 bfff s'http://baidu.com/logo.png'">
			<Text class="category_title" value="You can get the full examples source code from Github" />
			<Button class="long_btn rm_margin_top" onClick={handle_go_to} url={examples_source}>Go Github</Button>
		</Div>
	</Mynavpage>
)

const documents_vx = ()=>(
	<Mynavpage title="Documents" source={resolve(__filename)}>
		<Div width="full">
			<Hybrid class="category_title">Now go to <T textColor="#0079ff" value="quarks.cc" /> to view the document?</Hybrid>
			<Button class="long_btn rm_margin_top" onClick={handle_go_to} url={documents}>Go Documents</Button>
		</Div>
	</Mynavpage>
)

const bug_feedback_vx = ()=>(
	<Mynavpage title="Bug Feedback" source={resolve(__filename)}>
		<Div width="full">
			<Hybrid class="category_title">Now go to Github issues list?</Hybrid>
			<Button class="long_btn rm_margin_top" onClick={handle_go_to} url={quark_tools_issues_url}>Go Github Issues</Button>
			<Hybrid class="category_title">Or you can send me email, too.</Hybrid>
			<Button class="long_btn rm_margin_top" onClick={handle_bug_feedback}>Send email</Button>
		</Div>
	</Mynavpage>
)

// register font icomoon-ultimate
font.registerFont( reader.readFileSync(resolve('./icomoon.ttf')) );

// console.log(app.displayPort.phyWidth)

app.start(
	<Root>

		<NavPageCollection id="npc" defaultToolbar={<DefaultToolbar />}>
			<Mynavpage title="Home" source={resolve(__filename)}>

				<Scroll width="full" height="full" bounceLock={0}>

					<Text class="hello" value="Hello." />
					<Div class="category" borderBottom={`${px} #c8c7cc`}>
						<Hybrid class="codepre">
							<T class="keywork" value="import"/> {"{"} <T class="identifier" value="Application" />, <T class="identifier" value="Root" /> {"}"} <T class="keywork" value="from" /> <T class="str" value="'quark'" />
								<T class="keywork" value={'\nnew'}/> <T class="identifier" value="Application"/>()<T class="keywork" value="."/><T class="identifier" value="start"/>
								(
									{"<"}<T class="tag_name" value="Root" />{">"}hello world!{"</"}<T class="tag_name" value="Root" />{">"}
								)
						</Hybrid>
					</Div>

					<Text class="category_title" />
					<Clip class="category">
						<Navbutton next={examples}>Examples</Navbutton>
						<Navbutton next={examples_source_vx}>Examples Source</Navbutton>
						<Navbutton next={quark_tools_vx}>Quark Tools</Navbutton>
					</Clip>

					<Text class="category_title" />
					<Clip class="category">
						<Navbutton next={about_vx}>About</Navbutton>
						<Navbutton next={documents_vx}>Documents</Navbutton>
						<Navbutton next={bug_feedback_vx}>Bug Feedback</Navbutton>
					</Clip>

					<Div height={32} width="full" />

				</Scroll>

			</Mynavpage>
		</NavPageCollection>
	</Root>
)

var lock = Number(util.options.lock);
if (lock) {
	app.displayPort.lockSize(lock);
}
