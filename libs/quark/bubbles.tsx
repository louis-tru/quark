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

import {_CVD,link,View,Box,Matrix,VDom,Window,ViewType} from './index';
import { Navigation } from './nav';
import { UIEvent } from './event';
import * as types from './types';

const arrowSize = { width: 30, height: 12 };
const borderArrow = 0;
const border = 5;

/**
 * @enum Priority
*/
export enum Priority {
	Auto, //!<
	Top, //!<
	Right, //!<
	Bottom, //!<
	Left, //!<
};

/**
 * @template P,S
 * @class Bubbles
 * @extends Navigation
 * 
 * The tips bubble UI component
 */
export class Bubbles<P={},S={}> extends Navigation<{
	frail?: boolean;
	backgroundColor?: types.ColorStrIn;
	priority?: Priority,
	borderRadius?: number;
}&P,S> {
	private _isActivate = false;
	private _posX = 0;
	private _posY = 0;
	private _offsetX = 0;
	private _offsetY = 0;

	/**
	 * @getset frail:boolean By default clicking anywhere on the screen will disappear
	 */
	@link frail = true;

	/**
	 * @getset priority:Priority display location
	 */
	@link priority = Priority.Bottom;

	/**
	 * @getset backgroundColor:types.ColorStrIn
	*/
	@link backgroundColor: types.ColorStrIn = '#fff';

	/**
	 * @getset borderRadius:number
	*/
	@link borderRadius = 8;

	private getLeft(x: number, offset_x: number) {
		let self = this;
		let main = self.refs.main as Box;
		x -= border; // Keep 10px margin
		let screen_width = this.window.size.x - border * 2;
		let width = main.clientSize.x;

		if (screen_width < width) {
			return (screen_width - width) / 2 + border;
		} else {
			let left = x + offset_x / 2 - width / 2;
			if (left < 0) {
				left = 0;
			}
			else if(left + width > screen_width){
				left = screen_width - width;
			}
			return left + border;
		}
	}

	private getTop(y: number, offset_y: number) {
		let self = this;
		let main = self.refs.main as Box;
		y -= border; // Keep 10px margin
		let screen_height = this.window.size.y - border * 2;
		let height = main.clientSize.y;

		if (screen_height < height) {
			return (screen_height - height) / 2 + border;
		} else {
			let top = y + offset_y / 2 - height / 2;
			if (top < 0) {
				top = 0;
			}
			else if (top + height > screen_height) {
				top = screen_height - height;
			}
			return top + border;
		}
	}

	private getArrowTop(top: number, y: number, offset_y: number) {
		let self = this;
		let main = self.refs.main as Box;
		let height = main.clientSize.y;
		y += offset_y / 2;
		let min = borderArrow + arrowSize.width / 2;
		let max = height - borderArrow - arrowSize.width / 2;
		if (min > max) {
			return height / 2;
		}
		return Math.min(Math.max(min, y - top), max);
	}

	private getArrowLeft(left: number, x: number, offset_x: number) {
		let self = this;
		let main = self.refs.main as Box;
		let width = main.clientSize.x;
		x += offset_x / 2;
		let min = borderArrow + arrowSize.width / 2;
		let max = width - borderArrow - arrowSize.width / 2;
		if (min > max) {
			return width / 2;
		}
		return Math.min(Math.max(min, x - left), max);
	}

	/*
	 * Attempt to display at the top of the target
	 */
	private attemptTop(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		let self = this;
		let main = self.refs.main as Box;
		let arrow = self.refs.arrow as Matrix;
		let height = main.clientSize.y;
		let top = y - height - arrowSize.height;
		if (top - border > 0 || force) {
			let left = self.getLeft(x, offset_x);
			let arrow_left = self.getArrowLeft(left, x, offset_x) - arrowSize.width / 2;
			main.style = { y: top, x: left };
			arrow.style = { 
				align: 'leftBottom',
				y: arrowSize.height,// + 0.5, 
				x: arrow_left,
				rotateZ: 180,
			};
			return true;
		}
		return false;
	}

	/*
	 * Attempt to display at the right of the target
	 */
	private attemptRight(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		let self = this;
		let main = self.refs.main as Box;
		let arrow = self.refs.arrow as Matrix;
		let width = main.clientSize.x;
		let left = x + offset_x + arrowSize.height;
		if (left + width + border <= this.window.size.x || force) {
			let top = self.getTop(y, offset_y);
			let arrow_top = self.getArrowTop(top, y, offset_y) - arrowSize.height / 2;
			main.style = { y: top, x: left };
			arrow.style = { 
				align: 'leftTop',
				x: -(arrowSize.width / 2 + arrowSize.height / 2),
				y: arrow_top, 
				rotateZ: -90,
			};
			return true;
		}
		return false;
	}

	/*
	 * Attempt to display at the bottom of the target
	 */
	private attemptBottom(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		let self = this;
		let main = self.refs.main as Box;
		let arrow = self.refs.arrow as Matrix;
		let height = main.clientSize.y;
		let top = y + offset_y + arrowSize.height;
		if (top + height + border <= this.window.size.y || force) {
			let left = self.getLeft(x, offset_x);
			let arrow_left = self.getArrowLeft(left, x, offset_x) - arrowSize.width / 2;
			main.style = { y: top, x: left };
			arrow.style = {
				align: 'leftTop',
				x: arrow_left,
				y: -arrowSize.height,
				rotateZ: 0,
			};
			return true;
		}
		return false;
	}

	/*
	 * Attempt to display at the left of the target
	 */
	private attemptLeft(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		let self = this;
		let main = self.refs.main as Box;
		let arrow = self.refs.arrow as Matrix;
		let width = main.clientSize.x;
		let left = x - width - arrowSize.height;
		if (left - border > 0 || force) {
			let top = self.getTop(y, offset_y);
			let arrow_top = self.getArrowTop(top, y, offset_y) - arrowSize.height / 2;
			main.style = { y: top, x: left };
			arrow.style = {
				align: 'rightTop',
				x: arrowSize.width / 2 + arrowSize.height / 2,
				y: arrow_top,
				rotateZ: 90,
			};
			return true;
		}
		return false;
	}

	private _showBubbles(x: number, y: number, offset_x: number, offset_y: number) {
		let self = this;
		switch (self.priority) {
			case Priority.Top:
				self.attemptTop(x, y, offset_x, offset_y) ||
				self.attemptBottom(x, y, offset_x, offset_y) ||
				self.attemptRight(x, y, offset_x, offset_y) ||
				self.attemptLeft(x, y, offset_x, offset_y) ||
				self.attemptTop(x, y, offset_x, offset_y, true);
				break;
			case Priority.Right:
				self.attemptRight(x, y, offset_x, offset_y) ||
				self.attemptLeft(x, y, offset_x, offset_y) ||
				self.attemptBottom(x, y, offset_x, offset_y) ||
				self.attemptTop(x, y, offset_x, offset_y) ||
				self.attemptRight(x, y, offset_x, offset_y, true);
				break;
			case Priority.Bottom:
				self.attemptBottom(x, y, offset_x, offset_y) ||
				self.attemptTop(x, y, offset_x, offset_y) ||
				self.attemptRight(x, y, offset_x, offset_y) ||
				self.attemptLeft(x, y, offset_x, offset_y) ||
				self.attemptBottom(x, y, offset_x, offset_y, true);
				break;
			case Priority.Left:
				self.attemptLeft(x, y, offset_x, offset_y) ||
				self.attemptRight(x, y, offset_x, offset_y) ||
				self.attemptBottom(x, y, offset_x, offset_y) ||
				self.attemptTop(x, y, offset_x, offset_y) ||
				self.attemptLeft(x, y, offset_x, offset_y, true);
				break;
			default: break;
		}
	}

	private _fadeOut = (e: UIEvent)=>{
		if (e.isDefault) {
			this.fadeOut();
		}
	};

	protected render() {
		return (
			<free
				width="100%"
				height="100%"
				backgroundColor="#0001"
				opacity={0}
				visible={false}
				onTouchStart={this._fadeOut}
				onMouseDown={this._fadeOut}
			>
				<matrix ref="main">
					<free>
						<matrix
							ref="arrow"
							width={arrowSize.width}
							height={arrowSize.height}
						>
							<text
								marginTop={-10}
								marginLeft={-3}
								textFamily='iconfont'
								textLineHeight={36}
								textSize={36}
								textColor={this.backgroundColor}
								value={"\uedcb"} />
						</matrix>
						<flex
							backgroundColor={this.backgroundColor}
							borderRadius={this.borderRadius}
							direction="column"
							crossAlign="both"
							clip={this.borderRadius ? true: false}
						>{this.children}</flex>
					</free>
				</matrix>
			</free>
		);
	}

	protected triggerMounted() {
		this.window.onChange.on(this.destroy, this);
		this.asRef('main').onClick.on(()=>{
			if (this.frail)
				this.fadeOut();
		});
		this.asRef('main').onTouchStart.on(e=>e.cancelDefault());
		this.asRef('main').onMouseDown.on(e=>e.cancelDefault());
		super.appendTo(this.window.root);
	}

	protected triggerDestroy(): void {
		this.window.onChange.off(this);
		super.triggerDestroy();
	}

	appendTo(): View { throw Error.new('Access forbidden.') }
	afterTo(): View { throw Error.new('Access forbidden.') }

	fadeOut() {
		this.asDom().transition({ opacity: 0, time: 200 }, ()=>this.destroy());
		this.unregisterNavigation(0);
	}

	/**
	 * Showing Bubbles by Target View
	 * 
	 * @param from    Parameters provide the location information to be displayed
	 * @param offset? Displays the offset of the target position
	 */
	showFrom(from: View, offset?: types.Vec2) {
		offset = offset || types.newVec2(0,0);
		let pos = from.position;
		let size = from.viewType > ViewType.Label ? (from as Box).clientSize: types.newVec2(0,0);
		let rect = types.newRect(
			pos.x  + offset.x,     pos.y  + offset.y,
			size.x - offset.x * 2, size.y - offset.y * 2,
		);
		this.show(rect);
	}

	/**
	 * Show by rect
	 */
	show(rect: types.Rect) {
		let self = this;
		let size = this.window.size;
		let _x = Math.max(0, Math.min(size.x, rect.x));
		let _y = Math.max(0, Math.min(size.y, rect.y));
		let _offsetX = rect.width;
		let _offsetY = rect.height;

		self.asDom().visible = true;

		self._posX = _x;
		self._posY = _y;
		self._offsetX = _offsetX;
		self._offsetY = _offsetY;

		this.window.nextTickFrame(function() {
			self._showBubbles(_x, _y, _offsetX, _offsetY);
			self.asDom().transition({ opacity: 1, time: 200 });
		});

		self._isActivate = true;

		this.registerNavigation(0);
	}

	/**
	 * Reset and Reshow
	 */
	reset() {
		if (this._isActivate) {
			this._showBubbles(this._posX, this._posY, this._offsetX, this._offsetY);
		}
	}

	/**
	 * @overwrite
	 */
	navigationBack() {
		this.fadeOut();
		return true;
	}

	/**
	 * @overwrite
	 */
	navigationEnter(focus: View) {
	}

	/**
	 * Creating Bubbles through Vdom
	 * 
	 * @return {Bubbles}
	 */
	static render(win: Window, vdom: VDom<Bubbles>) {
		return vdom.newDom(win.rootCtr);
	}

	/**
	 * Create and show bubbles through Vdom
	 * 
	 * @return {Bubbles}
	 */
	static renderShow(from: View, vdom: VDom<Bubbles>) {
		let dom = vdom.newDom(from.window.rootCtr);
		dom.showFrom(from);
		return dom;
	}
}