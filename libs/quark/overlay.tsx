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

import {_CVD,link,View,Box,Matrix,VDom,Window} from './index';
import { Navigation } from './nav';
import * as types from './types';

const arrowSize = { width: 30, height: 12 };
const borderArrow = 0;
const border = 5;

export enum Priority {
	Auto,
	Top,
	Right,
	Bottom,
	Left,
};

/**
 * @class Overlay
 */
export class Overlay<P={},S={}> extends Navigation<{
	frail?: boolean;
	backgroundColor?: types.ColorStrIn;
	priority?: Priority,
	borderRadius?: number;
}&P,S> {
	private _is_activate = false;
	private _pos_x = 0;
	private _pos_y = 0;
	private _offset_x = 0;
	private _offset_y = 0;

	/**
	 * @prop frail, By default clicking anywhere on the screen will disappear
	 */
	@link frail = true;

	/**
	 * @prop priority display location
	 */
	@link priority = Priority.Bottom;

	/**
	 * @prop backgroundColor
	*/
	@link backgroundColor: types.ColorStrIn = '#fff';

	/**
	 * @prop borderRadius
	*/
	@link borderRadius = 8;

	private _get_left(x: number, offset_x: number) {
		let self = this;
		let inl = self.refs.inl as Box;
		x -= border; // 留出10像素边距
		let screen_width = this.window.size.x - border * 2;
		let width = inl.clientSize.x;

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
	private _get_top(y: number, offset_y: number) {
		let self = this;
		let inl = self.refs.inl as Box;
		y -= border; // 留出10像素边距
		let screen_height = this.window.size.y - border * 2;
		let height = inl.clientSize.y;

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
	private _get_arrow_top(top: number, y: number, offset_y: number) {
		let self = this;
		let inl = self.refs.inl as Box;
		let height = inl.clientSize.y;
		y += offset_y / 2;
		let min = borderArrow + arrowSize.width / 2;
		let max = height - borderArrow - arrowSize.width / 2;
		if (min > max) {
			return height / 2;
		}
		return Math.min(Math.max(min, y - top), max);
	}
	private _get_arrow_left(left: number, x: number, offset_x: number) {
		let self = this;
		let inl = self.refs.inl as Box;
		let width = inl.clientSize.x;
		x += offset_x / 2;
		let min = borderArrow + arrowSize.width / 2;
		let max = width - borderArrow - arrowSize.width / 2;
		if (min > max) {
			return width / 2;
		}
		return Math.min(Math.max(min, x - left), max);
	}

	/**
	 * Attempt to display at the top of the target
	 */
	private _attempt_top(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		let self = this;
		let inl = self.refs.inl as Box;
		let arrow = self.refs.arrow as Matrix;
		let height = inl.clientSize.y;
		let top = y - height - arrowSize.height;
		if (top - border > 0 || force) {
			let left = self._get_left(x, offset_x);
			let arrow_left = self._get_arrow_left(left, x, offset_x) - arrowSize.width / 2;
			inl.style = { y: top, x: left };
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

	/**
	 * Attempt to display at the right of the target
	 */
	private _attempt_right(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		let self = this;
		let inl = self.refs.inl as Box;
		let arrow = self.refs.arrow as Matrix;
		let width = inl.clientSize.x;
		let left = x + offset_x + arrowSize.height;
		if (left + width + border <= this.window.size.x || force) {
			let top = self._get_top(y, offset_y);
			let arrow_top = self._get_arrow_top(top, y, offset_y) - arrowSize.height / 2;
			inl.style = { y: top, x: left };
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

	/**
	 * Attempt to display at the bottom of the target
	 */
	private _attempt_bottom(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		let self = this;
		let inl = self.refs.inl as Box;
		let arrow = self.refs.arrow as Matrix;
		let height = inl.clientSize.y;
		let top = y + offset_y + arrowSize.height;
		if (top + height + border <= this.window.size.y || force) {
			let left = self._get_left(x, offset_x);
			let arrow_left = self._get_arrow_left(left, x, offset_x) - arrowSize.width / 2;
			inl.style = { y: top, x: left };
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

	/**
	 * Attempt to display at the left of the target
	 */
	private _attempt_left(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		let self = this;
		let inl = self.refs.inl as Box;
		let arrow = self.refs.arrow as Matrix;
		let width = inl.clientSize.x;
		let left = x - width - arrowSize.height;
		if (left - border > 0 || force) {
			let top = self._get_top(y, offset_y);
			let arrow_top = self._get_arrow_top(top, y, offset_y) - arrowSize.height / 2;
			inl.style = { y: top, x: left };
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

	private _showOverlay(x: number, y: number, offset_x: number, offset_y: number) {
		let self = this;
		switch (self.priority) {
			case Priority.Top:
				self._attempt_top(x, y, offset_x, offset_y) ||
				self._attempt_bottom(x, y, offset_x, offset_y) ||
				self._attempt_right(x, y, offset_x, offset_y) ||
				self._attempt_left(x, y, offset_x, offset_y) ||
				self._attempt_top(x, y, offset_x, offset_y, true);
				break;
			case Priority.Right:
				self._attempt_right(x, y, offset_x, offset_y) ||
				self._attempt_left(x, y, offset_x, offset_y) ||
				self._attempt_bottom(x, y, offset_x, offset_y) ||
				self._attempt_top(x, y, offset_x, offset_y) ||
				self._attempt_right(x, y, offset_x, offset_y, true);
				break;
			case Priority.Bottom:
				self._attempt_bottom(x, y, offset_x, offset_y) ||
				self._attempt_top(x, y, offset_x, offset_y) ||
				self._attempt_right(x, y, offset_x, offset_y) ||
				self._attempt_left(x, y, offset_x, offset_y) ||
				self._attempt_bottom(x, y, offset_x, offset_y, true);
				break;
			case Priority.Left:
				self._attempt_left(x, y, offset_x, offset_y) ||
				self._attempt_right(x, y, offset_x, offset_y) ||
				self._attempt_bottom(x, y, offset_x, offset_y) ||
				self._attempt_top(x, y, offset_x, offset_y) ||
				self._attempt_left(x, y, offset_x, offset_y, true);
				break;
			default: break;
		}
	}

	private _fadeOut = ()=>{
		this.fadeOut();
	};

	protected render() {
		return (
			<free
				width="100%"
				height="100%"
				backgroundColor="#0003"
				opacity={0}
				visible={false}
				onTouchStart={this._fadeOut}
				onMouseDown={this._fadeOut}
			>
				<matrix ref="inl">
					<matrix
						ref="arrow"
						width={arrowSize.width}
						height={arrowSize.height}
						originX={arrowSize.width/2} originY={arrowSize.height/0.5}
					>
						<text
							marginTop={-10}
							marginLeft={-3}
							textFamily='iconfont'
							textLineHeight={36}
							textSize={36}
							textColor={this.backgroundColor}
							value="\uedcb" />
					</matrix>
					<free backgroundColor={this.backgroundColor} borderRadius={this.borderRadius}>{this.children}</free>
				</matrix>
			</free>
		);
	}

	protected triggerMounted() {
		this.window.onChange.on(this.destroy, this);
		(this.refs.inl as Box).onClick.on(()=>{
			if (this.frail) {
				this.fadeOut();
			}
		});
		super.appendTo(this.window.root);
	}

	protected triggerDestroy(): void {
		this.window.onChange.off(this);
		super.triggerDestroy();
	}

	appendTo(): View { throw Error.new('Access forbidden.') }
	afterTo(): View { throw Error.new('Access forbidden.') }

	fadeOut() {
		this.asDom().transition({ opacity: 0, time: 200 }, ()=>{
			this.destroy();
		});
		this.unregisterNavigation(0);
	}

	/**
	 * @method showOverlayFrom(from[,offset_x[,offset_y]])  通过目标视图显示 Overlay
	 * @param from {Box} # 参数可提供要显示的位置信息
	 * @param [offset] {Object} # 显示目标位置的偏移
	 */
	showOverlayFrom(from: Box, offset?: types.Vec2) {
		offset = offset || types.newVec2(0,0);
		let {position,contentSize} = from;
		let rect = types.newRect(
			position.x    + offset.x,     position.y    + offset.y,
			contentSize.x - offset.x * 2, contentSize.y - offset.y * 2,
		);
		this.showOverlay(rect);
	}

	/**
	 * showOverlay(rect) 通过rect显示
	 */
	showOverlay(rect: types.Rect) {
		let self = this;
		let size = this.window.size;
		let _x = Math.max(0, Math.min(size.x, rect.x));
		let _y = Math.max(0, Math.min(size.y, rect.y));
		let _offset_x = rect.width;
		let _offset_y = rect.height;

		self.asDom().visible = true;

		self._pos_x = _x;
		self._pos_y = _y;
		self._offset_x = _offset_x;
		self._offset_y = _offset_y;

		this.window.nextFrame(function() {
			self._showOverlay(_x, _y, _offset_x, _offset_y);
			self.asDom().transition({ opacity: 1, time: 200 });
		});
		
		self._is_activate = true;

		this.registerNavigation(0);
	}

	/**
	 * reset() 重新设置位置
	 */
	reset() {
		if (this._is_activate) {
			this._showOverlay(this._pos_x, this._pos_y, this._offset_x, this._offset_y);
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
	 * @method render()
	*/
	static render(vdom: VDom<Overlay>, win: Window) {
		return vdom.newDom(win.rootCtr);
	}
}