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

import { Jsx, Application, Window, createCss, mainScreenScale, ViewController } from 'quark';
import {reader} from 'quark/fs';
import * as types from 'quark/types';
import { NavPageCollection } from 'quark/nav';
import { NavButton, Page } from './tool';
import examples from './examples';
import about_vx from './about';

const app = new Application();
const px = 1 / mainScreenScale();
const resolve = require.resolve;

// register font icomoon-ultimate
app.fontPool.addFontFamily( reader.readFileSync(resolve('./icomoon.ttf')) );

createCss({
	'.category_title': {
		width: 'match',
		textLineHeight: 30,
		textColor: '#6d6d72',
		textSize: 14,
		textWhiteSpace: 'preWrap',
		margin: 16,
	},

	'.rm_margin_top': {
		marginTop: 0,
	},

	'.text_mark': {
	},

	'.hello': {
		width: 'match',
		textSize:46, 
		textAlign:"center",
		textColor:"#000",
		margin: 16,
		marginTop: 18,
		marginBottom: 18,
	},

	'.category': {
		width: 'match',
		borderTop: `${px} #c8c7cc`,
		// borderBottom: `${px} #c8c7cc`,
		backgroundColor: '#fff',
		// borderBottomWidth: 0,
	},

	'.toolbar_btn': {
		padding: 8,
		textFamily: 'icomoon-ultimate',
		textSize: 24,
	},

	'.codepre': {
		width: 'match',
		margin: 10,
		textWhiteSpace: 'preWrap'
	},
	'.codepre:normal': {
		textColor: '#000'
	},
	'.codepre:hover': {
		//textColor: '#f0f'
	},
	'.codepre:active': {
		//textColor: '#f00',
	},
	'.codepre .tag_name': {
		textColor: '#005cc5'
	},
	'.codepre .keywork': {
		textColor: '#d73a49'
	},
	'.codepre .identifier': {
		textColor: '#6f42c1'
	},
	'.codepre .str': {
		textColor: '#007526'
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

const win = new Window({
	title: 'Examples',
	frame: types.newRect(0,0,375,700),
	// msaa: 4,
	// matchScreen: !!util.options.match_screen,
	// enableTouch: true,
	backgroundColor: types.newColor(255,255,255,255),
}).activate();

const quark_tools = 'https://www.npmjs.com/package/qkmake';
const quark_tools_issues_url = 'https://github.com/louis-tru/quark/issues';
const examples_source = 'https://github.com/louis-tru/quark.git';
const documents = 'http://quarks.cc/';

class Button extends ViewController<{url?: string, class?: string, onClick?: ()=>void}> {

	handle_go_to = ()=>{
		const url = this.props.url;
		if ( url ) {
			app.openURL(url);
		}
	}
	render() {
		return <button
			class={this.props.class}
			onClick={this.props.onClick || this.handle_go_to}
		>{this.children}</button>
	}
}

function handle_bug_feedback() {
	app.sendEmail('louistru@hotmail.com', 'bug feedback');
}

const quark_tools_vx = (self: Page)=>{
	self.title = 'Quark Tools';
	self.source = resolve(__filename);
	return (
		<box width="match">
			<text class="category_title">
1. You can use nodejs <label textBackgroundColor="#ddd" value={"npm install -g qkmake\n"} />
2. Or get the node modules from Github.
			</text>
			<Button class="long_btn rm_margin_top" url={quark_tools}>Go Github</Button>
		</box>
	)
}

const examples_source_vx = (self: Page)=>{
	self.title = 'Examples Source';
	self.source = resolve(__filename);
	return (
		<box width="match">
			<text class="category_title" value="You can get the match examples source code from Github" />
			<Button class="long_btn rm_margin_top" url={examples_source}>Go Github</Button>
		</box>
	)
}

const examples_source_vx_test = (self: Page)=>{
	self.title = 'Examples Source';
	self.source = resolve(__filename);
	return (
		<box width="match">
			<text class="category_title" value="You can get the match examples source code from Github" />
			<Button class="long_btn rm_margin_top" url={examples_source}>Go Github</Button>
		</box>
	)
}

const documents_vx = (self: Page)=>{
	self.title = 'Documents';
	self.source = resolve(__filename);
	return (
		<box width="match">
			<text class="category_title">Now go to <label textColor="#0079ff" value="quarks.cc" /> to view the document?</text>
			<Button class="long_btn rm_margin_top" url={documents}>Go Documents</Button>
		</box>
	)
}

const bug_feedback_vx = (self: Page)=>{
	self.title = 'Bug Feedback';
	self.source = resolve(__filename);
	return (
		<box width="match">
			<text class="category_title">Now go to Github issues list?</text>
			<Button class="long_btn rm_margin_top" url={quark_tools_issues_url}>Go Github Issues</Button>
			<text class="category_title">Or you can send me email, too.</text>
			<Button class="long_btn rm_margin_top" onClick={handle_bug_feedback}>Send email</Button>
		</box>
	)
}

win.render(
	<NavPageCollection ref="npc">
		<Page title="Home" source={resolve(__filename)}>
			<scroll width="match" height="match" bounceLock={false}>
				<text class="hello" value="Hello." />
				<box class="category" borderBottom={`${px} #c8c7cc`}>
					<text class="codepre">
						<label class="keywork" value="import"/> {"{"} <label class="identifier" value="Application" />, <label class="identifier" value="Root" /> {"}"} <label class="keywork" value="from" /> <label class="str" value="'quark'" />
							<label class="keywork" value={'\nnew'}/> <label class="identifier" value="Application"/>()
							<label class="keywork" value={'\nnew'}/> <label class="identifier" value="Window"/>()<label class="keywork" value="."/><label class="identifier" value="render"/>
							{"("}
								{"\n    <"}<label class="tag_name" value="text" />{">"}hello world!{"</"}<label class="tag_name" value="text" />{">"}
							{"\n)"}
					</text>
				</box>

				<text class="category_title" />
				<box class="category">
					<NavButton next={examples}>Examples</NavButton>
					<NavButton next={examples_source_vx}>Examples Source</NavButton>
					<NavButton next={quark_tools_vx}>Quark Tools</NavButton>
				</box>

				<text class="category_title" />
				<box class="category">
					<NavButton next={about_vx}>About</NavButton>
					<NavButton next={documents_vx}>Documents</NavButton>
					<NavButton next={bug_feedback_vx}>Bug Feedback</NavButton>
				</box>

				<box height={32} width="match" />
			</scroll>
		</Page>
	</NavPageCollection>
)
