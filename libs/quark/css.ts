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

import * as types from './types';
import type {Uint,Float} from './defs';

const _css = __binding__('_css');

exports.CStyleSheetsClass = _css.CStyleSheetsClass;

/**
 * Qk Style System
 *
 * Qk provides a lightweight, CSS-inspired style system designed
 * specifically for hierarchical GUI view trees, not for browser DOMs.
 *
 * The system intentionally supports only a constrained and explicit
 * subset of CSS concepts that map cleanly onto Qk’s view architecture
 * and rendering pipeline.
 *
 * Supported selector features:
 * - Class selectors (e.g. `.a`, `.a.b`)
 * - Hierarchical selectors (e.g. `.a .b`, `.a > .b`)
 * - Limited pseudo states (`:normal`, `:hover`, `:active`)
 *
 * Non-goals:
 * - Full CSS selector compatibility
 * - Layout-driven or cascade-heavy semantics
 * - Browser-specific behaviors
 *
 * The design prioritizes:
 * - Explicit structure over implicit cascading
 * - Predictable selector resolution
 * - Efficient propagation and incremental updates
 * - Stable performance on deep or dynamic view trees
 */

/**
 * @type CSSNameExp:'.className'
 *
 * Represents a class-based selector expression.
 *
 * IMPORTANT:
 * - Only class selectors are supported.
 * - This does NOT represent a full CSS selector grammar.
 *
 * Example:
 *   '.button'
 *   '.button.primary'
 */
export type CSSNameExp = `.${string}`;

/**
 * @class StyleSheets
 *
 * Represents a collection of style properties applied to a View.
 *
 * This class does NOT describe selector logic.
 * Selector matching and propagation are handled separately by
 * `CStyleSheetsClass`.
 *
 * Properties defined here are inspired by CSS,
 * but semantics are adapted for Qk's GUI layout and rendering model.
 */
export declare abstract class StyleSheets {
	time?: Uint; //!< keyframe time or css transition time in milliseconds
	curve?: types.CurveIn; //!< keyframe curve or css transition curve
	// Meta attribute
	color?: types.ColorIn; //!< view color
	cascadeColor?: types.CascadeColorIn; //!< cascade color
	cursor?: types.CursorStyleIn; //!<
	zIndex?: Uint; //!< z index in global view tree
	opacity?: Float; //!< opacity 0.0 ~ 1.0, color.a alias, opacity = color.a / 255.0
	visible?: boolean; //!< view visible
	receive?: boolean; //!< view receive events
	aa?: boolean; //!< view anti-aliasing
	clip?: boolean; //!< view clip children
	layout?: types.LayoutTypeIn; //!< view layout type
	align?: types.AlignIn; //!<
	width?: types.BoxSizeIn; //!<
	height?: types.BoxSizeIn; //!<
	minWidth?: types.BoxSizeIn; //!<
	minHeight?: types.BoxSizeIn; //!<
	maxWidth?: types.BoxSizeIn; //!<
	maxHeight?: types.BoxSizeIn; //!<
	margin?: number[] | number; //!<
	marginTop?: number; //!<
	marginRight?: number; //!<
	marginBottom?: number; //!<
	marginLeft?: number; //!<
	padding?: number[] | number; //!<
	paddingTop?: number; //!<
	paddingRight?: number; //!<
	paddingBottom?: number; //!<
	paddingLeft?: number; //!<
	borderRadius?: number[] | number; //!<
	borderTopLeftRadius?: number; //!<
	borderTopRightRadius?: number; //!<
	borderBottomRightRadius?: number; //!<
	borderBottomLeftRadius?: number; //!<
	border?: types.BorderIn[] | types.BorderIn; //!< border
	borderTop?: types.BorderIn; //!<
	borderRight?: types.BorderIn; //!<
	borderBottom?: types.BorderIn; //!<
	borderLeft?: types.BorderIn; //!<
	borderWidth?: number[] | number; //!<
	borderColor?: types.ColorIn[] | types.ColorIn; //!<
	borderTopWidth?: number; //!< border width
	borderRightWidth?: number; //!<
	borderBottomWidth?: number; //!<
	borderLeftWidth?: number; //!<
	borderTopColor?: types.ColorIn; //!< border color
	borderRightColor?: types.ColorIn; //!<
	borderBottomColor?: types.ColorIn; //!<
	borderLeftColor?: types.ColorIn; //!<
	backgroundColor?: types.ColorIn; //!<
	background?: types.BoxFilterIn; //!<
	boxShadow?: types.BoxShadowIn; //!<
	weight?: number[] | number; //!<
	direction?: types.DirectionIn; //!<
	itemsAlign?: types.ItemsAlignIn; //!<
	crossAlign?: types.CrossAlignIn; //!<
	wrap?: types.WrapIn; //!<
	wrapAlign?: types.WrapAlignIn; //!<
	src?: string; //!<
	textAlign?: types.TextAlignIn; //!<
	fontWeight?: types.FontWeightIn; //!<
	fontSlant?: types.FontSlantIn; //!<
	textDecoration?: types.TextDecorationIn; //!<
	textOverflow?: types.TextOverflowIn; //!<
	whiteSpace?: types.WhiteSpaceIn; //!<
	wordBreak?: types.WordBreakIn; //!<
	fontSize?: types.FontSizeIn; //!<
	textBackgroundColor?: types.TextColorIn; //!<
	textStroke?: types.TextStrokeIn; //!<
	textColor?: types.TextColorIn; //!<
	lineHeight?: types.FontSizeIn; //!<
	textShadow?: types.TextShadowIn; //!<
	fontFamily?: types.FontFamilyIn; //!<
	security?: boolean; //!<
	readonly?: boolean; //!<
	keyboardType?: types.KeyboardTypeIn; //!<
	returnType?: types.KeyboardReturnTypeIn; //!<
	placeholderColor?: types.ColorIn; //!<
	cursorColor?: types.ColorIn; //!<
	maxLength?: number; //!<
	placeholder?: string; //!<
	scrollbarColor?: types.ColorIn; //!<
	scrollbarWidth?: number; //!<
	scrollbarMargin?: number; //!<
	translate?: types.Vec2In; //!<
	scale?: types.Vec2In; //!<
	skew?: types.Vec2In; //!<
	origin?: types.BoxOriginIn[] | types.BoxOriginIn //!<
	originX?: types.BoxOriginIn; //!<
	originY?: types.BoxOriginIn; //!<
	x?: number; //!<
	y?: number; //!<
	scaleX?: number; //!<
	scaleY?: number; //!<
	skewX?: number; //!<
	skewY?: number; //!<
	rotateZ?: number; //!<
}

/**
 * @class CStyleSheetsClass
 *
 * Runtime stylesheet class collection bound to a View.
 *
 * This object:
 * - Holds the set of class names applied to a View
 * - Resolves which style rules apply at runtime
 * - Manages propagation of class-based selectors to descendant Views
 *
 * IMPORTANT:
 * - Only class names participate in selector matching.
 * - No attribute, tag, or structural selectors are involved.
 */
export declare class CStyleSheetsClass {
	/**
	 * Set Style Collection
	*/
	set(name: string[] | string): void;
	/**
	 * Add a stylesheet class selector name
	*/
	add(name: string): void;
	/**
	 * Delete a stylesheet class selector name
	*/
	remove(name: string): void;
	/**
	 * Toggle between Add and Delete
	*/
	toggle(name: string): boolean;

	/**
	 * Clear all stylesheet class selector names
	*/
	clear(): void;

	/**
	 * Check if a stylesheet class selector name exists
	*/
	has(name: string): boolean;

	/**
	 * get all stylesheet class selector names
	*/
	readonly names: Set<string>;
}
/** @end */

/**
 *
 * @method createCss(sheets)
 *
 * Creates style rules using a CSS-inspired, class-driven syntax.
 *
 * IMPORTANT:
 * This API intentionally supports ONLY a constrained subset of CSS.
 * It is designed for GUI view styling in Qk, not for full web CSS compatibility.
 *
 * Supported:
 * - Class selectors only (e.g. `.a`, `.a.b`, `.a .b`, `.a > .b`)
 * - Multi-level hierarchical selectors
 * - Limited pseudo states: `:normal`, `:hover`, `:active`
 * - Style properties inspired by CSS, adapted for Qk view rendering
 *
 * Unsupported:
 * - ID selectors, attribute selectors, and tag selectors
 * - Complex combinators (e.g. sibling selectors)
 * - Arbitrary or custom pseudo-classes and pseudo-elements
 * - Media queries, keyframes, and CSS animations
 * - Browser-specific or layout-driven CSS features
 *
 * Pseudo state mapping:
 * - `:normal` → [`HighlightedStatus.Normal`]
 * - `:hover`  → [`HighlightedStatus.Hover`]
 * - `:active` → [`HighlightedStatus.Active`]
 *
 * Notes:
 * - Selector semantics are resolved at creation time, not at runtime.
 * - This method mutates global style state.
 * - All rendering threads are automatically paused and resumed internally
 *   while stylesheets are being created or modified.
 *
 * @param sheets:object Style rule definitions keyed by class selectors
 * @param apply?:boolean Whether to apply the created styles to all views, **default false**
 * 
 * @example
 * 
 *	```ts
 *	createCss({
 *		'.test': {
 *			width: '50%',
 *			height: '50%',
 *			backgroundColor: '#00f',
 *		},
 *		'.test .a': {
 *			width: 50,
 *			height: 50,
 *		},
 *		'.test > .c': {
 *			width: 100,
 *		},
 *		'.test .a.b': { // This selector will have a higher priority
 *			height: 60,
 *		},
 *		// Apply these pseudo-classes to the target.
 *		// To make them effective on the target, the target view must be able to receive events.
 *		'.test:normal .a': {
 *			backgroundColor: '#0000',
 *			time: 1000, // Use transition time
 *		},
 *		'.test:hover': {
 *			backgroundColor: '#f0f',
 *		},
 *		'.test:hover .a': {
 *			backgroundColor: '#f00',
 *			time: 1000,  // Use transition time
 *		},
 *	})
 *	```
 */
export const createCss = _css.create as ((sheets: { [key: `.${string}`]: StyleSheets }, apply?: boolean)=>void);
export default createCss;
