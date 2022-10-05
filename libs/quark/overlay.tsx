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

import quark, {
	View, Div, Text, Clip, Indep, _CVD
} from './index';
import { Navigation } from './nav';
import { prop } from './ctr';

var arrow_size = { width: 30, height: 12 };
var border = 5;
var border_arrow = 0;

/**
 * @class Overlay
 */
export class Overlay extends Navigation {
	
	private m_is_activate = false;
	
	private m_pos_x = 0;
	private m_pos_y = 0;
	private m_offset_x = 0;
	private m_offset_y = 0;

	/**
	 * 默认为点击屏幕任何位置都会消失
	 */
	frail = true;

	@prop backgroundColor: string = '#fff';
	
	/**
	 * 获取left
	 */
	private _get_left(x: number, offset_x: number) {
		var self = this;
		
		x -= border; // 留出10像素边距
		var screen_width = quark.displayPort.width - border * 2;
		var width = self.find<Indep>('inl').clientWidth;
		
		if (screen_width < width) {
			return (screen_width - width) / 2 + border;
		}
		else {
			var left = x + offset_x / 2 - width / 2;
			if (left < 0) {
				left = 0;
			}
			else if(left + width > screen_width){
				left = screen_width - width;
			}
			return left + border;
		}
	}

	/**
	 * 获取top
	 */
	private _get_top(y: number, offset_y: number) {
		var self = this;
		y -= border; // 留出10像素边距
		var screen_height = quark.displayPort.height - border * 2;
		var height = self.find<Indep>('inl').clientHeight;
		
		if (screen_height < height) {
			return (screen_height - height) / 2 + border;
		}
		else{
			var top = y + offset_y / 2 - height / 2;
			if (top < 0) {
				top = 0;
			}
			else if (top + height > screen_height) {
				top = screen_height - height;
			}
			return top + border;
		}
	}

	/**
	 * 获取arrowtop
	 */
	private _get_arrow_top(top: number, y: number, offset_y: number) {
		var self = this;
		var height = self.find<Indep>('inl').clientHeight;
		y += offset_y / 2;
		var min = border_arrow + arrow_size.width / 2;
		var max = height - border_arrow - arrow_size.width / 2;
		if (min > max) {
			return height / 2;
		}
		return Math.min(Math.max(min, y - top), max);
	}

	/**
	 * 获取arrowleft
	 */
	private _get_arrow_left(left: number, x: number, offset_x: number) {
		var self = this;
		var width = self.find<Indep>('inl').clientWidth;
		x += offset_x / 2;
		var min = border_arrow + arrow_size.width / 2;
		var max = width - border_arrow - arrow_size.width / 2;
		if (min > max) {
			return width / 2;
		}
		return Math.min(Math.max(min, x - left), max);
	}

	/**
	 * 尝试在目标的top显示
	 */
	private _attempt_top(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		var self = this;

		var height = self.find<Indep>('inl').clientHeight;
		var top = y - height - arrow_size.height;
		
		if (top - border > 0 || force) {
			var left = self._get_left(x, offset_x);
			var arrow_left = self._get_arrow_left(left, x, offset_x) - arrow_size.width / 2;
			self.IDs.inl.style = { y: top, x: left };
			self.IDs.arrow.style = { 
				alignX: 'left',
				alignY: 'bottom',
				y: arrow_size.height,// + 0.5, 
				x: arrow_left,
				rotateZ: 180,
			};
			return true;
		}
		return false;
	}

	/**
	 * 尝试在目标的right显示
	 */
	private _attempt_right(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		var self = this;
		
		var width = self.find<Indep>('inl').clientWidth;
		
		var left = x + offset_x + arrow_size.height;
		
		if (left + width + border <= quark.displayPort.width || force) {
			var top = self._get_top(y, offset_y);
			var arrow_top = self._get_arrow_top(top, y, offset_y) - arrow_size.height / 2;
			self.IDs.inl.style = { y: top, x: left };
			self.IDs.arrow.style = { 
				alignX: 'left',
				alignY: 'top',
				x: -(arrow_size.width / 2 + arrow_size.height / 2),
				y: arrow_top, 
				rotateZ: -90,
			};
			return true;
		}
		return false;
	}

	/**
	 * 尝试在目标的bottom显示
	 */
	private _attempt_bottom(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		var self = this;
		
		var height = self.find<Indep>('inl').clientHeight;
		
		var top = y + offset_y + arrow_size.height;
		
		if (top + height + border <= quark.displayPort.height || force) {
			var left = self._get_left(x, offset_x);
			var arrow_left = self._get_arrow_left(left, x, offset_x) - arrow_size.width / 2;
			self.IDs.inl.style = { y: top, x: left };
			self.IDs.arrow.style = {
				alignX: 'left',
				alignY: 'top',
				x: arrow_left,
				y: -arrow_size.height,
				rotateZ: 0,
			};
			return true;
		}
		return false;
	}

	/**
	 * 尝试在目标的left显示
	 */
	private _attempt_left(x: number, y: number, offset_x: number, offset_y: number, force?: boolean) {
		var self = this;
		
		var width = self.find<Indep>('inl').clientWidth;
		var left = x - width - arrow_size.height;
		
		if (left - border > 0 || force) {
			
			var top = self._get_top(y, offset_y);
			var arrow_top = self._get_arrow_top(top, y, offset_y) - arrow_size.height / 2;
			self.IDs.inl.style = { y: top, x: left };
			self.IDs.arrow.style = {
				alignX: 'right',
				alignY: 'top',
				x: arrow_size.width / 2 + arrow_size.height / 2,
				y: arrow_top,
				rotateZ: 90,
			};
			return true;
		}
		return false;
	}

	private _showOverlay(x: number, y: number, offset_x: number, offset_y: number) {
		var self = this;

		switch (self.priority) {
			case 'top':
				self._attempt_top(x, y, offset_x, offset_y) ||
				self._attempt_bottom(x, y, offset_x, offset_y) ||
				self._attempt_right(x, y, offset_x, offset_y) ||
				self._attempt_left(x, y, offset_x, offset_y) ||
				self._attempt_top(x, y, offset_x, offset_y, true);
				break;
			case 'right':
				self._attempt_right(x, y, offset_x, offset_y) ||
				self._attempt_left(x, y, offset_x, offset_y) ||
				self._attempt_bottom(x, y, offset_x, offset_y) ||
				self._attempt_top(x, y, offset_x, offset_y) ||
				self._attempt_right(x, y, offset_x, offset_y, true);
				break;
			case 'bottom':
				self._attempt_bottom(x, y, offset_x, offset_y) ||
				self._attempt_top(x, y, offset_x, offset_y) ||
				self._attempt_right(x, y, offset_x, offset_y) ||
				self._attempt_left(x, y, offset_x, offset_y) ||
				self._attempt_bottom(x, y, offset_x, offset_y, true);
				break;
			default:
				self._attempt_left(x, y, offset_x, offset_y) ||
				self._attempt_right(x, y, offset_x, offset_y) ||
				self._attempt_bottom(x, y, offset_x, offset_y) ||
				self._attempt_top(x, y, offset_x, offset_y) ||
				self._attempt_left(x, y, offset_x, offset_y, true);
				break;
		}
	}
		
	/**
	 * 优先显示的位置
	 */
	priority = 'bottom'; // top | right | bottom | left
	
	/**
	 * @overwrite
	 */
	render(...vdoms: any[]) {
		return (
			<Indep visible={false} width="full" height="full" backgroundColor="#0003" opacity={0}>
				<Div width="full" height="full" onTouchStart={()=>this.fadeOut()} onMouseDown={()=>this.fadeOut()} id="mask" />
				<Indep id="inl">

					<Indep id="arrow" 
						width={arrow_size.width }
						height={arrow_size.height} 
						originX={arrow_size.width/2} originY={arrow_size.height/2}>
						<Text id="arrow_text" 
							y={-10} x={-3}
							textFamily='iconfont' 
							textLineHeight={36}
							textSize={36} textColor={this.backgroundColor} value={"\uedcb"} />
					</Indep>

					<Clip id="content" backgroundColor={this.backgroundColor} borderRadius={8}>{vdoms}</Clip>

				</Indep>
			</Indep>
		);
	}

	triggerMounted() {
		//quark.displayPort.onChange.on(self.remove, self);

		this.find<Indep>('inl').onClick.on(()=>{
			if ( this.frail ) {
				this.fadeOut();
			}
		});
		this.appendTo(quark.root);

		return super.triggerMounted();
	}
	
	fadeOut() {
		this.domAs().transition({ opacity: 0, time: 200 }, ()=>{
			this.remove();
		});
		this.unregisterNavigation(0);
	}
	
	/**
	 * @fun showOverlayFromView(target_view[,offset_x[,offset_y]])  通过目标视图显示 Overlay
	 * @arg target_view {View} # 参数可提供要显示的位置信息
	 * @arg [offset] {Object} # 显示目标位置的偏移
	 */
	showOverlayFromView(target_view: View, offset_x?: number, offset_y?: number) {
		offset_x = offset_x || 0;
		offset_y = offset_y || 0;
		var rect = target_view.screenRect();
		this.showOverlay(
			rect.x + offset_x, rect.y + offset_y, 
			rect.width - offset_x * 2, rect.height - offset_y * 2);
	}
	
	/**
	 * showOverlay(pos_x,pos_y[,offset_x[,offset_y]]) 通过位置显示
	 */
	showOverlay(pos_x: number, pos_y: number, offset_x?: number, offset_y?: number) {
		
		var self = this;
		
		var _x = Math.max(0, Math.min(quark.displayPort.width, pos_x));
		var _y = Math.max(0, Math.min(quark.displayPort.height, pos_y));
		var _offset_x = offset_x || 0;
		var _offset_y = offset_y || 0;
		
		self.domAs().show();
		
		self.m_pos_x = _x;
		self.m_pos_y = _y;
		self.m_offset_x = _offset_x;
		self.m_offset_y = _offset_y;
		
		quark.nextFrame(function() {
			self._showOverlay(_x, _y, _offset_x, _offset_y);
			self.domAs().transition({ opacity: 1, time: 200 });
		});
		
		self.m_is_activate = true;

		this.registerNavigation(0);
	}
	
	/**
	 * reset() 重新设置位置
	 */
	reset() {
		if (this.m_is_activate) {
			this._showOverlay(this.m_pos_x, this.m_pos_y, this.m_offset_x, this.m_offset_y);
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

}