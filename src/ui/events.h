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

#ifndef __quark__ui__events__
#define __quark__ui__events__

// All events of UI / Name, Flag
#define Qk_UI_Events(F) \
/* can bubble event */ \
F(Click, Click, kBubble_UIEventFlags) \
F(Back, Click, kBubble_UIEventFlags) \
F(KeyDown, Keyboard, kBubble_UIEventFlags) \
F(KeyPress, Keyboard, kBubble_UIEventFlags) \
F(KeyUp, Keyboard, kBubble_UIEventFlags) \
F(KeyEnter, Keyboard, kBubble_UIEventFlags) \
F(TouchStart, Touch, kBubble_UIEventFlags) \
F(TouchMove, Touch, kBubble_UIEventFlags) \
F(TouchEnd, Touch, kBubble_UIEventFlags) \
F(TouchCancel, Touch, kBubble_UIEventFlags) \
F(MouseOver, Mouse, kBubble_UIEventFlags) \
F(MouseOut, Mouse, kBubble_UIEventFlags) \
F(MouseLeave, Mouse, kBubble_UIEventFlags) \
F(MouseEnter, Mouse, kBubble_UIEventFlags) \
F(MouseMove, Mouse, kBubble_UIEventFlags) \
F(MouseDown, Mouse, kBubble_UIEventFlags) \
F(MouseUp, Mouse, kBubble_UIEventFlags) \
F(MouseWheel, Mouse, kBubble_UIEventFlags) \
F(Focus, Default, kBubble_UIEventFlags) \
F(Blur, Default, kBubble_UIEventFlags) \
F(Highlighted, Highlighted, kBubble_UIEventFlags) /* normal / hover / down */ \
F(Error, Default, kError_UIEventFlags) \
/* canno bubble event */ \
F(ActionKeyframe, Action, kNone_UIEventFlags) \
F(ActionLoop, Action, kNone_UIEventFlags) \
F(Scroll, Default, kNone_UIEventFlags) /*ScrollView*/\
F(Change, Default, kNone_UIEventFlags) /*Input*/ \
F(Load, Default, kNone_UIEventFlags) /* Image */ \
/* Player */ \
F(Stop, Player, kNone_UIEventFlags) \
F(Buffering, Player, kFloat32_UIEventFlags) \
/* Spine */ \
F(SpineStart, Spine, kNone_UIEventFlags) \
F(SpineInterrupt, Spine, kNone_UIEventFlags) \
F(SpineEnd, Spine, kNone_UIEventFlags) \
F(SpineDispose, Spine, kNone_UIEventFlags) \
F(SpineComplete, Spine, kNone_UIEventFlags) \
F(SpineEvent, Spine, kNone_UIEventFlags) \
/* Agent */ \
F(ReachWaypoint, Agent, kNone_UIEventFlags) \
F(DiscoveryAgent, Agent, kNone_UIEventFlags) \
F(AgentMovement, Agent, kNone_UIEventFlags) \
F(AgentHeadingChange, Agent, kNone_UIEventFlags) \

#endif