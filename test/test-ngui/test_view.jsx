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

import { P, M, LOG, VM, VP } from './test'
import { Vec2, value } from 'ngui/value';
import KeyframeAction from 'ngui/action';
import {
  GUIApplication,
  CSS,
  View,
  Root, 
  Div, 
  Indep,
  Limit,
  LimitIndep,
  Image, 
  Scroll,
  Sprite,
  Hybrid,
  TextNode, 
  Span, 
  Label, 
  Panel, 
  Button, 
  Text, 
  Clip,
  Input,
  Textarea,
  ngui: gui,
} from 'ngui';
import { Video } from 'ngui/media';

CSS({
  '.test': {
    x: 0,
  },
})

// start gui application
new GUIApplication({ _multisample: 4 }).start(
  <Root>
  </Root>
).onload = function() {

  LOG('\nTest View:\n')

  var v = new View();

  M(v, 'prepend', [new View()]);
  M(v, 'append', [new View()]);
  M(v, 'appendText', ['ABCD']);
  M(v, 'appendTo', [this.root]);
  M(v, 'before', [new View()]);
  M(v, 'after', [new View]);
  M(v, 'moveToBefore');
  M(v, 'moveToAfter');
  M(v, 'moveToFirst');
  M(v, 'moveToLast');
  M(v, 'removeAllChild');
  VP(v, 'childrenCount', 0)
  M(v, 'remove');
  VP(v, 'parent', null)
  M(v, 'appendTo', [this.root])
  M(v, 'focus');
  M(v, 'blur');
  M(v, 'layoutOffset');
  M(v, 'layoutOffsetFrom', [this.root]);
  M(v, 'children', [0]);
  M(v, 'setAction', [new KeyframeAction()]);
  M(v, 'getAction');
  M(v, 'setAction', [null]);
  M(v, 'screenRect');
  M(v, 'finalMatrix');
  M(v, 'finalOpacity');
  M(v, 'position');
  M(v, 'overlapTest', [new Vec2(0,0)]);
  M(v, 'addClass', ['test']);
  M(v, 'removeClass', ['test']);
  M(v, 'toggleClass', ['test']);
  // property
  P(v, 'innerText');
  P(v, 'innerText', 'KKKKK');
  P(v, 'childrenCount');
  P(v, 'id');
  P(v, 'id', 'view_01');
  P(v, 'id');
  P(v, 'controller');
  P(v, 'ctr');
  P(v, 'top');
  P(v, 'topCtr');
  P(v, 'parent');
  P(v, 'prev');
  P(v, 'next');
  P(v, 'first');
  P(v, 'last');
  P(v, 'x');
  P(v, 'x', 0);
  P(v, 'y');
  P(v, 'y', 0);
  P(v, 'scaleX');
  P(v, 'scaleX', 1);
  P(v, 'scaleY');
  P(v, 'scaleY', 1);
  P(v, 'rotateZ');
  P(v, 'rotateZ', 0);
  P(v, 'skewX');
  P(v, 'skewX', 0);
  P(v, 'skewY');
  P(v, 'skewY', 0);
  P(v, 'opacity');
  P(v, 'opacity', 1);
  P(v, 'visible');
  P(v, 'visible', 1);
  P(v, 'finalVisible');
  P(v, 'translate');
  P(v, 'translate', new Vec2(0,20));
  P(v, 'scale');
  P(v, 'scale', new Vec2(1,1));
  P(v, 'skew');
  P(v, 'skew', new Vec2(0,0));
  P(v, 'originX');
  P(v, 'originX', 0);
  P(v, 'originY');
  P(v, 'originY', 0);
  P(v, 'origin');
  P(v, 'origin', new Vec2(0,0));
  P(v, 'matrix');
  P(v, 'level');
  P(v, 'needDraw');
  P(v, 'needDraw', v.need_draw);
  P(v, 'receive');
  P(v, 'receive', v.receive);
  P(v, 'isFocus');
  P(v, 'viewType');
  P(v, 'style');
  P(v, 'style', {x:0});
  P(v, 'class');
  P(v, 'class', 'test');
  P(v, 'y', 140);

  LOG('\nTest Box Div:\n')

  var d = new Div();

  M(d, 'appendTo', [this.root])

  P(d, 'width');
  P(d, 'width', 'auto');
  P(d, 'width', '50%');
  P(d, 'width', '100!');
  P(d, 'width', 'full');
  P(d, 'width', 100);

  P(d, 'height', 'auto');
  P(d, 'height', '50%');
  P(d, 'height', '100!');
  P(d, 'height', 'full');
  P(d, 'height', 100);

  P(d, 'margin', 10);
  P(d, 'marginLeft');
  P(d, 'marginTop');
  P(d, 'marginRight');
  P(d, 'marginBottom');
  P(d, 'marginLeft', 'auto');
  P(d, 'marginTop', 15);
  P(d, 'marginRight', 'auto');
  P(d, 'marginBottom', 0);

  P(d, 'border', '1 #f00');
  P(d, 'borderLeft');
  P(d, 'borderTop');
  P(d, 'borderRight');
  P(d, 'borderBottom');
  P(d, 'borderLeft', '2 #f00');
  P(d, 'borderTop', '2 #f0f');
  P(d, 'borderRight', '2 #ff0');
  P(d, 'borderBottom', '2 #00f');

  P(d, 'borderWidth', 3);
  P(d, 'borderLeftWidth');
  P(d, 'borderTopWidth');
  P(d, 'borderRightWidth');
  P(d, 'borderBottomWidth');
  P(d, 'borderLeftWidth', 4);
  P(d, 'borderTopWidth', 4);
  P(d, 'borderRightWidth', 4);
  P(d, 'borderBottomWidth', 4);

  P(d, 'borderColor', '#f00');
  P(d, 'borderLeftColor');
  P(d, 'borderTopColor');
  P(d, 'borderRightColor');
  P(d, 'borderBottomColor');
  P(d, 'borderLeftColor', '#f00');
  P(d, 'borderTopColor', '#f0f');
  P(d, 'borderRightColor', '#ff0');
  P(d, 'borderBottomColor', '#00f');

  P(d, 'borderRadius', 10);
  P(d, 'borderRadiusLeftTop');
  P(d, 'borderRadiusRightTop');
  P(d, 'borderRadiusRightBottom');
  P(d, 'borderRadiusLeftBottom');
  P(d, 'borderRadiusLeftTop', 20);
  P(d, 'borderRadiusRightTop', 10);
  P(d, 'borderRadiusRightBottom', 20);
  P(d, 'borderRadiusLeftBottom', 10);

  P(d, 'backgroundColor');
  P(d, 'backgroundColor', '#aaa4');

  P(d, 'newline');
  P(d, 'newline', true);

  P(d, 'finalWidth');
  P(d, 'finalHeight');
  P(d, 'finalMarginLeft');
  P(d, 'finalMarginTop');
  P(d, 'finalMarginRight');
  P(d, 'finalMarginBottom');

  P(d, 'contentAlign')
  P(d, 'contentAlign', 'right')

  LOG('\nTEST Indep:\n')
  var indep = new Indep()
  M(indep, 'appendTo', [this.root])
  P(indep, 'alignX')
  P(indep, 'alignY')
  P(indep, 'alignX', 'left')
  P(indep, 'alignY', 'center')
  //P(indep, 'align', 'center')

  LOG('\nTest Limit:\n');
  var limit = new Limit();
  M(limit, 'appendTo', [this.root])
  P(limit, 'minWidth');
  P(limit, 'minHeight');
  P(limit, 'maxWidth');
  P(limit, 'maxHeight');
  P(limit, 'minWidth', 10);
  P(limit, 'minHeight', 10);
  P(limit, 'maxWidth', 10);
  P(limit, 'maxHeight', 10);

  LOG('\nTEST LimitIndep:\n')
  var li = new LimitIndep();
  M(li, 'appendTo', [this.root])
  P(li, 'minWidth');
  P(li, 'minHeight');
  P(li, 'maxWidth');
  P(li, 'maxHeight');
  P(li, 'minWidth', 10);
  P(li, 'minHeight', 10);
  P(li, 'maxWidth', 10);
  P(li, 'maxHeight', 10);

  LOG('\nTEST Image:\n')
  var img = new Image();
  M(img, 'appendTo', [this.root])
  P(img, 'src');
  P(img, 'src', resolve('res/10440501.jpg'));
  P(img, 'marginLeft', 'auto');
  P(img, 'marginRight', 'auto');
  P(img, 'sourceWidth');
  P(img, 'sourceHeight');
  P(img, 'backgroundImage');
  P(img, 'backgroundImage', resolve('res/cc.jpg'));
  P(img, 'height', 200)

  LOG('\nTest Scroll:\n')
  var sc = new Scroll();
  M(sc, 'appendTo', [this.root])
  M(sc, 'scrollTo', [new Vec2(0,0)]);
  M(sc, 'terminate');
  P(sc, 'scroll');
  P(sc, 'scroll', new Vec2(0,0));
  P(sc, 'scrollX');
  P(sc, 'scrollX', 0);
  P(sc, 'scrollY');
  P(sc, 'scrollY', 0);
  P(sc, 'scrollWidth');
  P(sc, 'scrollHeight');
  P(sc, 'scrollbar');
  P(sc, 'scrollbar', true);
  P(sc, 'resistance');
  P(sc, 'resistance', 1);
  P(sc, 'bounce');
  P(sc, 'bounce', true);
  P(sc, 'bounceLock');
  P(sc, 'bounceLock', false);
  P(sc, 'lockDirection');
  P(sc, 'lockDirection', false);
  P(sc, 'catchPositionX');
  P(sc, 'catchPositionX', 1);
  P(sc, 'catchPositionY');
  P(sc, 'catchPositionY', 1);
  P(sc, 'scrollbarColor');
  P(sc, 'scrollbarColor', '#000');
  P(sc, 'hScrollbar');
  P(sc, 'vScrollbar');
  P(sc, 'scrollbarWidth');
  P(sc, 'scrollbarWidth', 2);
  P(sc, 'scrollbarMargin');
  P(sc, 'scrollbarMargin', 2);
  P(sc, 'defaultScrollDuration');
  P(sc, 'defaultScrollDuration', 4000)
  P(sc, 'defaultScrollCurve');
  P(sc, 'defaultScrollCurve', new value.Curve(0,0,1,1));
  // TODO ..
  P(sc, 'momentum');
  P(sc, 'momentum', true);
  // Scroll
  P(sc, 'switchMarginLeft');
  P(sc, 'switchMarginLeft', 10);
  P(sc, 'switchMarginRight');
  P(sc, 'switchMarginRight', 10);
  P(sc, 'switchMarginTop');
  P(sc, 'switchMarginTop', 10);
  P(sc, 'switchMarginBottom');
  P(sc, 'switchMarginBottom', 10);
  P(sc, 'switchAlignX');
  P(sc, 'switchAlignX', 'center');
  P(sc, 'switchAlignY');
  P(sc, 'switchAlignY', 'bottom');
  P(sc, 'enableSwitchScroll');
  P(sc, 'enableSwitchScroll', true);
  P(sc, 'enableFixedScrollSize');
  P(sc, 'enableFixedScrollSize', new Vec2());

  LOG('\nTest Video:\n')
  var video = new Video();
  P(Video, 'MEDIA_TYPE_AUDIO');
  P(Video, 'MEDIA_TYPE_VIDEO')
  P(Video, 'PLAYER_STATUS_STOP')
  P(Video, 'PLAYER_STATUS_START')
  P(Video, 'PLAYER_STATUS_PLAYING')
  P(Video, 'PLAYER_STATUS_PAUSED')
  P(Video, 'SOURCE_STATUS_UNINITIALIZED')
  P(Video, 'SOURCE_STATUS_READYING')
  P(Video, 'SOURCE_STATUS_READY')
  P(Video, 'SOURCE_STATUS_WAIT')
  P(Video, 'SOURCE_STATUS_FAULT')
  P(Video, 'SOURCE_STATUS_EOFF')
  P(Video, 'VIDEO_COLOR_FORMAT_INVALID')
  P(Video, 'VIDEO_COLOR_FORMAT_YUV420P')
  P(Video, 'VIDEO_COLOR_FORMAT_YUV420SP')
  P(Video, 'VIDEO_COLOR_FORMAT_YUV411P')
  P(Video, 'VIDEO_COLOR_FORMAT_YUV411SP')
  //
  P(video, 'autoPlay');
  P(video, 'autoPlay', true);
  P(video, 'sourceStatus');
  P(video, 'status');
  P(video, 'mute');
  P(video, 'mute', false);
  P(video, 'volume');
  P(video, 'volume', 10);
  P(video, 'time');
  P(video, 'duration');
  P(video, 'audioTrackIndex');
  P(video, 'audioTrackCount');
  P(video, 'disableWaitBuffer');
  P(video, 'disableWaitBuffer', false);
  P(video, 'videoWidth');
  P(video, 'videoHeight');
  M(video, 'selectAudioTrack', [0]);
  M(video, 'audioTrack');
  M(video, 'videoTrack');
  M(video, 'start');
  M(video, 'seek', [0]);
  M(video, 'pause');
  M(video, 'resume');
  M(video, 'stop');

  LOG('\nTest Sprite:\n')
  var s = new Sprite()
  M(s, 'appendTo', [this.root])
  P(s, 'src');
  P(s, 'src', resolve('res/cc.jpg'));
  P(s, 'texture');
  P(s, 'startX');
  P(s, 'startX', 20);
  P(s, 'startY');
  P(s, 'startY', 20);
  P(s, 'width');
  P(s, 'width', 100);
  P(s, 'height');
  P(s, 'height', 100);
  P(s, 'start');
  P(s, 'start', new Vec2(20,20));
  P(s, 'ratioX');
  P(s, 'ratioX', 1);
  P(s, 'ratioY');
  P(s, 'ratioY', 1);
  P(s, 'ratio');
  P(s, 'ratio', new Vec2(1,1));
  P(s, 'repeat');
  P(s, 'repeat', 'repeat');
  P(s, 'y', 160);

  LOG('\nTEST Hybrid:\n')
  var h = new Hybrid();
  M(h, 'appendTo', [this.root])
  M(h, 'simpleLayoutWidth', ['ABCD']);
  P(h, 'textBackgroundColor');
  P(h, 'textBackgroundColor', '#00f');
  P(h, 'textColor');
  P(h, 'textColor', '#000');
  P(h, 'textSize');
  P(h, 'textSize', 14);
  P(h, 'textStyle');
  P(h, 'textStyle', 'bold');
  P(h, 'textFamily');
  P(h, 'textFamily', 'AA');
  P(h, 'textShadow');
  P(h, 'textShadow', '10 10 2 #f00');
  P(h, 'textLineHeight');
  P(h, 'textLineHeight', 'auto');
  P(h, 'textDecoration');
  P(h, 'textDecoration', 'underline');
  P(h, 'textOverflow');
  P(h, 'textOverflow', 'ellipsis');
  P(h, 'textWhiteSpace');
  P(h, 'textWhiteSpace', 'no_wrap');
  P(h, 'textAlign');
  P(h, 'textAlign', 'right');

  LOG('\nTEST TextNode:\n')
  var t = new TextNode();
  M(t, 'appendTo', [this.root])
  P(t, 'value');
  P(t, 'value', 'AAAA');
  P(t, 'length');
  P(t, 'textHoriBearing');
  P(t, 'textHeight');
  P(t, 'textColor', '#f00');
  P(t, 'y', 100);

  LOG('\nTest Span:\n')
  var s = new Span();
  M(s, 'appendTo', [this.root])
  P(s, 'innerText', 'TTTTTTTTTTTTTTTTTTTTT Span')
  P(s, 'y', 120);

  LOG('\nTest Label:\n')
  var l = new Span();
  M(l, 'appendTo', [this.root])
  P(l, 'innerText', 'TTTTTTTTTTTTTTTTTTTTT Label')
  P(l, 'y', 80);

  LOG('\nTest Panel:\n')
  var p = new Panel();
  M(p, 'appendTo', [this.root])
  P(p, 'innerText', 'TTTTTTTTTTTTTTTTTTTTT Panel')
  M(p, 'firstButton');
  P(p, 'allowLeave');
  P(p, 'allowLeave', true);
  P(p, 'switchTime');
  P(p, 'switchTime', 100);
  P(p, 'enableSwitch');
  P(p, 'enableSwitch', true);
  P(p, 'isActivity');
  P(p, 'parentPanel');
  P(p, 'backgroundColor', '#ccc')

  LOG('\nTest Button:\n')
  var b = new Button();
  M(b, 'appendTo', [this.root])
  P(b, 'innerText', 'Button')
  P(b, 'backgroundColor', '#eee')
  M(b.onclick, 'on', [(e)=>{ }])
  M(b, 'findNextButton', ['right']);
  P(b, 'panel');

  LOG('\nTest Text:\n')
  var t = new Text();
  M(t, 'appendTo', [this.root])
  P(t, 'value', 'Text')
  P(t, 'backgroundColor', '#ddd')

  LOG('\nTest Clip:\n')
  var c = new Clip();
  M(c, 'appendTo', [this.root])
  P(c, 'innerText', 'Clip')
  P(c, 'backgroundColor', '#ccc')

  LOG('\nTest Input:\n')
  var i = new Input();
  M(i, 'appendTo', [this.root])
  P(i, 'margin', 2)
  P(i, 'width', 'full')
  P(i, 'height', 20)
  P(i, 'background_color', '#aaa')
  P(i, 'type');
  P(i, 'type', 'ascii');
  P(i, 'returnType');
  P(i, 'returnType');
  P(i, 'placeholder');
  P(i, 'placeholder', 'AAAAAA...');
  P(i, 'placeholderColor');
  P(i, 'placeholderColor', '#f00');
  P(i, 'security');
  P(i, 'security', false);
  P(i, 'textMargin');
  P(i, 'textMargin', 10);
  P(i, 'y', -340)

  LOG('\nTest Textarea:\n')
  var i = new Textarea();
  M(i, 'appendTo', [this.root])
  P(i, 'margin', 2)
  P(i, 'marginTop', 0)
  P(i, 'width', 'full')
  P(i, 'backgroundColor', '#aaa')
  P(i, 'placeholder');
  P(i, 'placeholder', 'Textarea...');
  P(i, 'placeholderColor');
  P(i, 'placeholderColor', '#f00');
  P(i, 'security');
  P(i, 'security', false);
  P(i, 'textMargin');
  P(i, 'textMargin', 10);
  P(i, 'y', -340)
    
};
