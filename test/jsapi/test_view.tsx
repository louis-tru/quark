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

import { LOG, Pv, Mv } from './tool'
import * as types from 'quark/types'
import {BoxSizeKind,BoxOriginKind} from 'quark/types'
import {KeyframeAction,Action} from 'quark/action'
import {Window,createCss,CStyleSheetsClass,ViewType} from 'quark'
import {
	View,
	Box,
	Flex,
	Flow,
	Free,
	Matrix,
	Image,
	Scroll,
	Label,
	Button,
	Text,
	Input,
	Textarea,
} from 'quark'

export default async function(win: Window) {
	const resolve = require.resolve;
	const root = win.root;

	root.style.backgroundColor = '#aa0';

	createCss({
		'.test': { x: 0 },
	});

	LOG('\nTest View:\n')
	const v = new View(win);
	const before = new View(win);
	const after = new View(win);
	Mv(v, 'prepend', [new View(win)]);
	Mv(v, 'append', [new View(win)]);
	Mv(v, 'appendTo', [root]);
	Mv(v, 'before', [before]);
	Mv(v, 'after', [after]);
	Mv(v, 'removeAllChild', []);
	Mv(v, 'prepend', [new View(win)]);
	Mv(v, 'remove', []);
	Mv(v, 'appendTo', [root]);
	Pv(v, 'parent', root);
	Mv(v, 'focus', []);
	Mv(v, 'blur', []);
	Pv(v, 'layoutOffset', e=>e.x==0&&e.y==0); // {0,0}
	Pv(v, 'action', null);
	Pv(v, 'action', e=>e instanceof Action, e=>e.action=new KeyframeAction(win));
	Pv(v, 'position', e=>e.x==0&&e.y==0);
	Mv(v, 'overlapTest', [types.newVec2(0,0)], false);
	Pv(v, 'cssclass', e=>e instanceof CStyleSheetsClass);
	Mv(v.cssclass, 'add', ['test']);
	Mv(v.cssclass, 'remove', ['test']);
	Mv(v.cssclass, 'set', ['test']);
	Mv(v.cssclass, 'toggle', ['test']);
	// property
	Pv(v, 'parent', root);
	Pv(v, 'prev', after);
	Pv(v, 'next', null);
	Pv(v, 'first', e=>e instanceof View);
	Pv(v, 'last', e=>e instanceof View);
	Pv(v, 'window', win);
	Pv(v, 'opacity', 0, e=>e.opacity=0);
	Pv(v, 'opacity', 1, e=>e.opacity=1);
	Pv(v, 'visible', true);
	// Pv(v, 'level', 2);
	Pv(v, 'matrixView', root);
	Pv(v, 'receive', false, e=>e.receive=false);
	Pv(v, 'isFocus', false);
	Pv(v, 'viewType', ViewType.View);
	Pv(v, 'style', v);
	Pv(v, 'class', e=>e.length==0, e=>e.class=['test']);
	Pv(v, 'layoutWeight',  e=>e.x==0&&e.y==0);
	Pv(v, 'layoutAlign', types.Align.Normal);
	Pv(v, 'isClip', false);
	Pv(v, 'center', e=>e.x==0&&e.y==0);
	Pv(v, 'metaView', v);
	//Pv(v, 'visibleRegion', true);
	Pv(v, 'ref', '');
	Pv(v, 'cursor', types.CursorStyle.Arrow);
	Mv(v, 'hide', [], ()=>v.visible===false);
	Mv(v, 'show', [], ()=>v.visible===true);
	Mv(v, 'isSelfChild', [v.first!], true);
	Mv(v, 'hashCode', [], e=>e==v.hashCode());
	//(v as any).ttt = 'ABCDEFG';
	//console.log('(v as any).ttt----------------------', (v as any).ttt);
	Mv(v, 'transition', [{time:1e3,opacity: 0.3}], e=>e.duration==1e3);

	LOG('\nTest Box:\n')
	const d = new Box(win);
	// background: types.BoxFilter | null;
	// boxShadow: types.BoxShadow | null;
	Mv(d, 'appendTo', [root]);
	Pv(d, 'width', e=>e.kind==BoxSizeKind.Auto);
	//d.style.width='100%'
	//console.log('d.width---', d.width.kind, d.width.value);
	Pv(d, 'width', e=>e.kind==BoxSizeKind.Ratio&&e.value==1, e=>e.style.width='100%');
	Pv(d, 'height', e=>e.kind==BoxSizeKind.Auto);
	Pv(d, 'height', e=>e.kind===BoxSizeKind.Ratio&&e.value==1, e=>e.style.height='100%');
	Pv(d, 'minWidth', e=>e.kind==BoxSizeKind.Auto, e=>e.style.minWidth='auto');
	Pv(d, 'minHeight', e=>e.kind==BoxSizeKind.Match, e=>e.style.minHeight='match');
	Pv(d, 'maxWidth', e=>e.kind===BoxSizeKind.Ratio&&e.value==1, e=>e.style.maxWidth='100%');
	Pv(d, 'maxHeight', e=>e.kind==BoxSizeKind.Minus&&e.value==0, e=>e.style.maxHeight='0!');
	Pv(d, 'margin', e=>e.every(e=>e==10), e=>e.style.margin=10);
	Pv(d, 'marginLeft', 10);
	Pv(d, 'marginTop', 10);
	Pv(d, 'marginRight', 10);
	Pv(d, 'marginBottom', 10);
	Pv(d, 'marginLeft', 1, e=>e.marginLeft=1);
	Pv(d, 'padding', e=>e.every(e=>e==10), e=>e.style.padding=10);
	Pv(d, 'paddingLeft', 10);
	Pv(d, 'paddingTop', 10);
	Pv(d, 'paddingRight', 10);
	Pv(d, 'paddingBottom', 10);
	Pv(d, 'paddingLeft', 1, e=>e.paddingLeft=1);
	Pv(d, 'border', e=>e.every(e=>e.width==1&&e.color.toString()=='#ff0000'), e=>e.style.border='1 #f00');
	Pv(d, 'borderLeft', e=>e.width==1&&e.color.toString()=='#ff0000');
	Pv(d, 'borderTop', e=>e.width==1&&e.color.toString()=='#ff0000');
	Pv(d, 'borderRight', e=>e.width==1&&e.color.toString()=='#ff0000');
	Pv(d, 'borderBottom', e=>e.width==1&&e.color.toString()=='#ff0000');
	Pv(d, 'borderLeft', e=>e.width==2&&e.color.toString()=='#ffff00', e=>e.style.borderLeft='2 #ff0');
	Pv(d, 'borderTop', e=>e.width==3&&e.color.toString()=='#ff00ff', e=>e.style.borderTop='3 #f0f');
	Pv(d, 'borderRight', e=>e.width==4&&e.color.toString()=='#ffff00', e=>e.style.borderRight='4 #ff0');
	Pv(d, 'borderBottom', e=>e.width==2&&e.color.toString()=='#0000ff', e=>e.style.borderBottom='2 #00f');
	Pv(d, 'borderWidthLeft', 3, e=>e.style.borderWidth=3);
	Pv(d, 'borderWidthTop', 3);
	Pv(d, 'borderWidthRight', 3);
	Pv(d, 'borderWidthBottom', 3);
	Pv(d, 'borderWidthLeft', 1, e=>e.borderWidthLeft=1);
	Pv(d, 'borderWidthTop', 2, e=>e.borderWidthTop=2);
	Pv(d, 'borderWidthRight', 3, e=>e.borderWidthRight=3);
	Pv(d, 'borderWidthBottom', 4, e=>e.borderWidthBottom=4);
	Pv(d, 'borderColorLeft', e=>e.toString()=='#ff0000', e=>e.style.borderColor='#f00');
	Pv(d, 'borderColorTop', e=>e.toString()=='#ff0000');
	Pv(d, 'borderColorRight', e=>e.toString()=='#ff0000');
	Pv(d, 'borderColorBottom', e=>e.toString()=='#ff0000');
	Pv(d, 'borderColorLeft', e=>e.toString()=='#ff0000', e=>e.style.borderColorLeft='#f00');
	Pv(d, 'borderColorTop', e=>e.toString()=='#ff00ff', e=>e.style.borderColorTop='#f0f');
	Pv(d, 'borderColorRight', e=>e.toString()=='#ffff00', e=>e.style.borderColorRight='#ff0');
	Pv(d, 'borderColorBottom', e=>e.toString()=='#0000ff', e=>e.style.borderColorBottom='#00f');
	Pv(d, 'borderRadiusLeftTop', 10, e=>e.style.borderRadius=10);
	Pv(d, 'borderRadiusRightTop', 10);
	Pv(d, 'borderRadiusRightBottom', 10);
	Pv(d, 'borderRadiusLeftBottom', 10);
	Pv(d, 'borderRadiusLeftTop', 20, e=>e.borderRadiusLeftTop=20);
	Pv(d, 'borderRadiusRightTop', 10, e=>e.borderRadiusRightTop=10);
	Pv(d, 'borderRadiusRightBottom', 5, e=>e.borderRadiusRightBottom=5);
	Pv(d, 'borderRadiusLeftBottom', 1, e=>e.borderRadiusLeftBottom=1);
	Pv(d, 'backgroundColor', e=>e.a==0);
	Pv(d, 'backgroundColor', e=>e.toHex32String()=='#aaaaaa44', e=>e.style.backgroundColor='#aaa4');
	//Pv(d, 'clientSize', e=>e.x==0&&e.y==0);
	//Pv(d, 'contentSize', e=>e.x==0&&e.y==0);
	Pv(d, 'align', types.Align.Normal);
	Pv(d, 'align', types.Align.Start, e=>e.style.align='start');
	Pv(d, 'clip', false);
	//console.log('d.wrapX', d.wrapX);
	//Pv(d, 'wrapX', true);
	//Pv(d, 'wrapY', true);
	Pv(d, 'weight', e=>e.x==0&&e.y==0);

	LOG('\nTest Flex:\n')
	const f = new Flex(win);
	Mv(f, 'appendTo', [root])
	Pv(f, 'direction', types.Direction.Column, e=>e.style.direction='column');
	Pv(f, 'itemsAlign', types.ItemsAlign.End, e=>e.style.itemsAlign='end');
	Pv(f, 'crossAlign', types.CrossAlign.Center, e=>e.style.crossAlign='center');

	LOG('\nTest Flow:\n')
	const flow = new Flow(win);
	Mv(flow, 'appendTo', [root])
	Pv(flow, 'wrap', types.Wrap.Wrap, e=>e.style.wrap='wrap')
	Pv(flow, 'wrapAlign', types.WrapAlign.Center, e=>e.style.wrapAlign='center')

	LOG('\nTest Free:\n')
	const fr = new Free(win);
	Mv(fr, 'appendTo', [root])
	Pv(flow, 'viewType', ViewType.Flow)

	LOG('\nTEST Image:\n')
	const img = new Image(win);
	Mv(img, 'appendTo', [root]);
	Pv(img, 'src', '');
	Pv(img, 'src', resolve('./res/0.png'), e=>e.src=resolve('./res/0.png'));
	Pv(img, 'marginLeft', 0);
	Pv(img, 'marginRight', 0);
	Pv(img, 'height', e=>e.kind==BoxSizeKind.Value&&e.value==200, e=>e.style.height=200);

	LOG('\nTest Scroll:\n')
	const sc = new Scroll(win);
	Mv(sc, 'appendTo', [root])
	Mv(sc, 'scrollTo', [types.newVec2(0,0)]);
	Mv(sc, 'terminate', []);
	console.log('sc.scroll', sc.scroll, sc.scroll.x, sc.scroll.y)
	Pv(sc, 'scroll', e=>e.x==0&&e.y==0);
	Pv(sc, 'scroll', e=>e.x==0&&e.y==0, e=>e.scroll=types.newVec2(0,0));
	Pv(sc, 'scrollX', 0);
	Pv(sc, 'scrollY', 0);
	Pv(sc, 'scrollSize', e=>e.x==0&&e.y==0);
	Pv(sc, 'scrollbar', true);
	Pv(sc, 'scrollbar', false, e=>e.scrollbar=false);
	Pv(sc, 'resistance', 1);
	Pv(sc, 'bounce', true);
	Pv(sc, 'bounceLock', true);
	Pv(sc, 'lockDirection', false);
	Pv(sc, 'catchPositionX', 1);
	Pv(sc, 'catchPositionY', 1);
	Pv(sc, 'scrollbarColor', e=>e.toHex32String()=='#8c8c8cc8');
	Pv(sc, 'scrollbarWidth', 2);
	Pv(sc, 'scrollbarMargin', 2);
	Pv(sc, 'scrollDuration', 0);
	Pv(sc, 'scrollDuration', 4000, e=>e.scrollDuration=4000);
	Pv(sc, 'defaultCurve', e=>e.toString().indexOf('curve(0,0,0.5799')==0); // 5799999833106995
	Pv(sc, 'momentum', true);
	Pv(sc, 'momentum', false, e=>e.momentum=false);
	Pv(sc, 'scrollbarH', false);
	Pv(sc, 'scrollbarV', false);

	LOG('\nTest Label:\n')
	const l = new Label(win);
	Mv(l, 'appendTo', [root])
	Pv(l, 'value', 'Test Label', e=>e.value='Test Label')

	LOG('\nTest Button:\n')
	const b = new Button(win);
	Mv(b, 'appendTo', [root])
	Pv(b, 'backgroundColor', e=>e.toString()=='#eeeeee', e=>e.style.backgroundColor='#eee')

	LOG('\nTest Text:\n')
	const t = new Text(win);
	Mv(t, 'appendTo', [root])
	Pv(t, 'value', 'Text', e=>e.value='Text')
	Pv(t, 'backgroundColor', e=>e.toString()=='#dddddd', e=>e.style.backgroundColor='#ddd')

	LOG('\nTest Input:\n')
	const i = new Input(win);
	i.style = {width: 'match', height: 20, border: '1 #666'};
	Mv(i, 'appendTo', [root])
	Pv(i, 'type', types.KeyboardType.Ascii, e=>e.style.type='ascii');
	Pv(i, 'returnType', types.KeyboardReturnType.Done, e=>e.style.returnType='done');
	Pv(i, 'placeholder', 'AAAAAA...', e=>e.placeholder='AAAAAA...');
	Pv(i, 'placeholderColor', e=>e.toString()=='#ff0000', e=>e.style.placeholderColor='#f006');
	Pv(i, 'marginLeft', 2, e=>e.margin=[2]);
	Pv(i, 'paddingLeft', 2, e=>e.padding=[2]);
	Pv(i, 'security', true, e=>e.security=true);

	LOG('\nTest Textarea:\n')
	const i1 = new Textarea(win);
	i1.style = {width: 'match', height: 150, border: '1 #666'};
	Mv(i1, 'appendTo', [root])
	Pv(i1, 'marginLeft', 2, e=>e.margin=[2]);
	Pv(i1, 'paddingLeft', 2, e=>e.padding=[2]);
	Pv(i1, 'width', e=>e.kind==BoxSizeKind.Match, e=>e.style.width='match')
	Pv(i1, 'security', false)

	LOG('\nTest Matrix:\n')
	const m = new Matrix(win)
	Pv(m, 'x', 0)
	Pv(m, 'y', 140, e=>e.y=140)
	Pv(m, 'scaleX', 1)
	Pv(m, 'scaleY', 1)
	Pv(m, 'rotateZ', 0)
	Pv(m, 'skewX', 0)
	Pv(m, 'skewY', 0)
	Pv(m, 'translate', e=>(e+'')==`vec2(0,140)`)
	Pv(m, 'scale', e=>(e+'')==`vec2(1,1)`)
	Pv(m, 'skew', e=>(e+'')==`vec2(0,0)`)
	Pv(m, 'origin', e=>e[0].kind==BoxOriginKind.Auto&&e[1].kind==BoxOriginKind.Auto)
	Pv(m, 'originX', e=>e.kind==BoxOriginKind.Ratio&&e.value==0.5, e=>e.style.originX='50%')
	Pv(m, 'originY', e=>e.kind==BoxOriginKind.Auto)
}