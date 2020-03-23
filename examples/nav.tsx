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

import { 
	Div, Indep, Button, Text, Hybrid, _CVD,
} from 'ngui';
import { Mynavpage, Page } from './public';
import { Navbar, Toolbar } from 'ngui/nav';
import review_vx from './review';
import { GUIClickEvent } from 'ngui/event';

const resolve = require.resolve;

function hide_show_navbar(evt: GUIClickEvent) {
	var navbar = evt.sender.ownerAs<Page>().navbar;
	var hidden = !navbar.hidden
	navbar.setHidden(hidden, true);
	(evt.sender.prev as Div).transition({ height: hidden ? 20 : 0, time: 400 });
}

function hide_show_toolbar(evt: GUIClickEvent) {
	var toolbar = evt.sender.ownerAs<Page>().toolbar;
	toolbar.setHidden(!toolbar.hidden, true);
}

function nav_pop(evt: GUIClickEvent) {
	evt.sender.ownerAs<Page>().collection.pop(true);
}

function view_code(evt: GUIClickEvent) {
	evt.sender.ownerAs<Page>().collection.push(review_vx(), true);
}

const navbar_vx = ()=>(
	<Navbar backgroundColor="#333" backTextColor="#fff" titleTextColor="#fff">
		<Indep alignX="right" alignY="center" x={-10}>
			<Button textFamily="icomoon-ultimate" textColor="#fff" textSize={20}>{"\ued63"}</Button>
		</Indep>
	</Navbar>
)

const toolbar_vx = ()=>(
	<Toolbar backgroundColor="#333">
		<Hybrid textAlign="center" width="full" height="full">
			<Button onClick={view_code}>
				<Text class="toolbar_btn" textColor="#fff" value={"\ue9ab"} />
			</Button>
		</Hybrid>
	</Toolbar>
)

export default ()=>(
	<Mynavpage 
		title="Nav" source={resolve(__filename)} 
		backgroundColor="#333" navbar={navbar_vx()} toolbar={toolbar_vx()}>
		<Div width="full">
			<Div width="full" height={0} />
			<Button class="long_btn2" onClick={hide_show_navbar}>Hide/Show Navbar</Button>
			<Button class="long_btn2" onClick={hide_show_toolbar}>Hide/Show Toolbar</Button>
			<Button class="long_btn2" onClick={nav_pop}>Nav pop</Button>
		</Div>
	</Mynavpage>
)