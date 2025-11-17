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

import util from './util';
import event, {
	Listen, NativeNotification, Notification, EventNoticer,
	UIEvent, HighlightedEvent, KeyEvent,
	ClickEvent, TouchEvent, MouseEvent, ActionEvent, GestureEvent,
	GestureTouchPoint,
	GestureStage, GestureType,
	TouchPoint,
	ReachWaypointEvent,
	DiscoveryAgentEvent,
	AgentStateEvent,
	SpineEvent,
	SpineExtEvent,
	AgentMovementEvent,} from './event';
import * as types from './types';
import {RemoveReadonly, Vec2, Vec2In} from './types';
import { StyleSheets, CStyleSheetsClass } from './css';
import { Window } from './window';
import { Action, createAction,KeyframeIn,TransitionResult } from './action';
import * as action from './action';
import {ViewController} from './ctr';
import {Player,MediaType,MediaSourceStatus,Stream} from './media';
import { Path } from './path';
/*───────────────────────────────────────────
  Chapter 1 — Core View & DOM
───────────────────────────────────────────*/

/**
 * Enumeration of all runtime view types in the UI/world tree.
 * Used for RTTI-style checks and optimized branching.
 *
 * @enum ViewType
 */
export enum ViewType {
	View, //!<
	Entity, //!<
	Sprite, //!<
	Spine, //!<
	Label, //!<
	Box, //!<
	Flex, //!<
	Flow, //!<
	Free, //!<
	Image, //!<
	Video, //!<
	Input, //!<
	Textarea, //!<
	Scroll, //!<
	Text, //!<
	Button, //!<
	Morph, //!<
	World, //!<
	Root, //!<
	Enum_Counts, //!<
}

/**
 * Lightweight DOM-like interface for runtime view attachment and ownership.
 * A DOM can be appended into the view tree, moved, and destroyed from its owner.
 *
 * @interface DOM
 */
export interface DOM {
	/** Reference name for this node (for lookup / debugging). */
	readonly ref: string;

	/** The "meta" view (mount point) associated with this DOM node. */
	readonly metaView: View;

	/**
	 * The owner ViewController that manages this DOM node.
	*/
	readonly owner: ViewController;

	/**
	 * Append this node as a child of the given parent.
	 * @param parent Target parent view.
	 * @returns The same view for chaining.
	 */
	appendTo(parent: View): View;

	/**
	 * Insert this node after an existing sibling.
	 * @param prev The sibling after which this node will be inserted.
	 * @returns The same view for chaining.
	 */
	afterTo(prev: View): View;

	/**
	 * Destroy this node from its logical owner (e.g. a ViewController).
	 * @param owner The controller / owner that manages this DOM node.
	 */
	destroy(owner: ViewController): void;
}

/* Internal JSX child node handle (may be null for holes/conditional children). */
type ChildDOM = DOM | null;

/**
 * Base class for all visual nodes.
 *
 * A `View` participates in:
 * - The scene tree (parent/child, prev/next sibling, etc.).
 * - Layout (size, alignment, margins, etc.).
 * - Style (color, border, background, etc.).
 * - Input and interaction events (mouse, touch, keys, gestures).
 * - Rendering and visibility.
 *
 * `View` is also an event emitter (`Notification<UIEvent>`) and implements a DOM-like API.
 *
 * @class View
 * @extends Notification<UIEvent>
 * @implements DOM
 */
export declare class View extends Notification<UIEvent> implements DOM {
	/** Internal JSX child DOM nodes (virtual children for VDOM/JSX diffing). */
	readonly childDoms: ChildDOM[];

	/** The owner ViewController that manages this view. */
	readonly owner: ViewController;

	/** @event Fired on pointer/touch "click"-like activation. */
	readonly onClick: EventNoticer<ClickEvent>;
	/** @event Fired on back navigation intent (e.g. hardware back). */
	readonly onBack: EventNoticer<ClickEvent>;

	/** @event Fired on key down. */
	readonly onKeyDown: EventNoticer<KeyEvent>;
	/** @event Fired on key press / text input. */
	readonly onKeyPress: EventNoticer<KeyEvent>;
	/** @event Fired on key up. */
	readonly onKeyUp: EventNoticer<KeyEvent>;
	/** @event Convenience event for Enter/Return. */
	readonly onKeyEnter: EventNoticer<KeyEvent>;

	/** @event Touch/multitouch start. */
	readonly onTouchStart: EventNoticer<TouchEvent>;
	/** @event Touch/multitouch move. */
	readonly onTouchMove: EventNoticer<TouchEvent>;
	/** @event Touch/multitouch end. */
	readonly onTouchEnd: EventNoticer<TouchEvent>;
	/** @event Touch canceled (gesture aborted). */
	readonly onTouchCancel: EventNoticer<TouchEvent>;

	/** @event Pointer enters this view's region. */
	readonly onMouseOver: EventNoticer<MouseEvent>;
	/** @event Pointer leaves toward another sibling/child. */
	readonly onMouseOut: EventNoticer<MouseEvent>;
	/** @event Pointer fully leaves (no longer within subtree). */
	readonly onMouseLeave: EventNoticer<MouseEvent>;
	/** @event Pointer first enters (no previous containment). */
	readonly onMouseEnter: EventNoticer<MouseEvent>;
	/** @event Pointer moves within this view. */
	readonly onMouseMove: EventNoticer<MouseEvent>;
	/** @event Mouse/pointer button down. */
	readonly onMouseDown: EventNoticer<MouseEvent>;
	/** @event Mouse/pointer button up. */
	readonly onMouseUp: EventNoticer<MouseEvent>;
	/** @event Mouse wheel / scroll wheel. */
	readonly onMouseWheel: EventNoticer<MouseEvent>;

	/** @event View received focus. */
	readonly onFocus: EventNoticer<UIEvent>;
	/** @event View lost focus. */
	readonly onBlur: EventNoticer<UIEvent>;
	/** @event View highlight state changed (hover, selection, etc.). */
	readonly onHighlighted: EventNoticer<HighlightedEvent>;

	/** @event Animation keyframe reached. */
	readonly onActionKeyframe: EventNoticer<ActionEvent>;
	/** @event Animation loop completed. */
	readonly onActionLoop: EventNoticer<ActionEvent>;

	/** @event Generic gesture event. */
	readonly onGesture: EventNoticer<GestureEvent>;
	/** @event Pan gesture recognized. */
	readonly onPanGesture: EventNoticer<GestureEvent>;
	/** @event Swipe gesture recognized. */
	readonly onSwipeGesture: EventNoticer<GestureEvent>;
	/** @event Pinch (scale) gesture recognized. */
	readonly onPinchGesture: EventNoticer<GestureEvent>;
	/** @event Rotation gesture recognized. */
	readonly onRotateGesture: EventNoticer<GestureEvent>;
	/** @event Three-finger gesture (platform dependent). */
	readonly onThreeFingerGesture: EventNoticer<GestureEvent>;
	/** @event Four-finger gesture (platform dependent). */
	readonly onFourFingerGesture: EventNoticer<GestureEvent>;

	/** Computed CSS class set for this view. */
	readonly cssclass: CStyleSheetsClass;

	/** Parent view in the scene graph (or null if root). */
	readonly parent: View | null;
	/** Previous sibling view (or null). */
	readonly prev: View | null;
	/** Next sibling view (or null). */
	readonly next: View | null;
	/** First child view (or null). */
	readonly first: View | null;
	/** Last child view (or null). */
	readonly last: View | null;

	/** Associated Window / rendering context. */
	readonly window: Window;

	/**
	 * The closest ancestor `MorphView` (a transformable node), or null.
	 * This is useful for walking up to a transform root.
	 */
	readonly morphView: MorphView | null;

	/** Depth level in the scene graph (0 at root, increasing downward). */
	readonly level: number;

	/** Layout weight (used by layout containers like Flex/Flow). */
	readonly layoutWeight: Vec2;

	/** Layout alignment rules for this view inside its parent. */
	readonly layoutAlign: types.Align;

	/** Whether this view clips its children to its bounds. */
	readonly isClip: boolean;

	/** View runtime type (for optimized branching and RTTI). */
	readonly viewType: ViewType;

	/**
	 * Final resolved position in layout coordinates.  
	 * 
	 * ⚠️ **Runtime-synced (render-thread updated):**  
	 * This value is updated from the rendering thread.  
	 * If accessed during active rendering, `x` and `y` may come from slightly  
	 * different frame moments.  
	 * 
	 * For most use cases, this value is sufficiently accurate,  
	 * but precision-critical code should not assume atomic consistency.  
	 * 
	 * @safe rt — updated by the render thread; may not always be atomically consistent.
	 */
	readonly position: Vec2;

	/**
	 * Layout offset from parent content origin.
	 * @safe rt — updated by the render thread; may not always be atomically consistent.
	 */
	readonly layoutOffset: Vec2;

	/**
	 * Final resolved layout size.
	 * For Box-like views: border + padding + content + margin.
	 * @safe rt — updated by the render thread; may not always be atomically consistent.
	 */
	readonly layoutSize: Vec2;

	/**
	 * Client (inner) size in local coordinates.
	 * For Box: offset, border + padding + content (no margin).
	 * @safe rt — updated by the render thread; may not always be atomically consistent.
	 */
	readonly clientSize: Vec2;

	/**
	 * Client region (used for precise hit testing).
	 * @safe rt — updated by the render thread; may not always be atomically consistent.
	 */
	readonly clientRegion: types.Region;

	/**
	 * Meta view for controller mounting.
	 * In many cases this is the same `View`, but controller systems may wrap.
	 */
	readonly metaView: View;

	/** Whether this view is currently in the visible area. */
	readonly visibleArea: boolean;

	/** Reference in owner view controller */
	readonly ref: string;

	/** Style sheet block attached to this view. */
	style: StyleSheets;

	/** Running animation or tween controller. */
	action: Action | null;

	/**
	 * Assigned class names.
	 * Setting this updates `cssclass` internally.
	 */
	class: string[];

	/**
	 * Local color tint applied to this view.
	 * The final rendered color is a combination of this color and parent color (depending on cascade_color).
	 */
	color: types.Color;

	/**
	 * Color inheritance mode from the parent view.
	 * Determines how this view's color combines with its parent's final color.
	 *
	 * final_color = parent.final_color ⨉ self.color   // depending on cascade_color
	 *
	 * Default: CascadeColor::Both
	 */
	cascadeColor: types.CascadeColor;

	/** Mouse cursor style when hovering this view. */
	cursor: types.CursorStyle;

	/** Z index in global view tree. */
	zIndex: Uint;

	/**
	 * Visual opacity.
	 * 0.0 ~ 1.0. Often mirrors color.a / 255.0.
	 */
	opacity: Float;

	/** Whether the view is visible for rendering & hit testing. */
	visible: boolean;

	/** Whether the view is currently interactive / receiving events. */
	receive: boolean;

	/** Enable anti-aliasing for drawing (if supported), default true. */
	aa: boolean;

	/** Whether the view currently has keyboard focus. */
	isFocus: boolean;

	/**
	 * Request focus for this view (if focusable).
	 * @returns true if focus was granted.
	 */
	focus(): boolean;

	/** Drop focus from this view if it currently has focus. */
	blur(): boolean;

	/** Convenience: set `visible = true`. */
	show(): void;

	/** Convenience: set `visible = false`. */
	hide(): void;

	/**
	 * Returns true if the given view is contained in this view's subtree.
	 * @param child The view to test.
	 */
	isSelfChild(child: View): boolean;

	/**
	 * Insert this view before another sibling in the same parent.
	 * @param view Sibling to insert before.
	 */
	before(view: View): void;

	/**
	 * Insert this view after another sibling in the same parent.
	 * @param view Sibling to insert after.
	 */
	after(view: View): void;

	/**
	 * Insert a view as the first child.
	 * @param view Child to prepend.
	 */
	prepend(view: View): void;

	/**
	 * Append a view as the last child.
	 * @param view Child to append.
	 */
	append(view: View): void;

	/**
	 * Remove this view from its parent.
	 */
	remove(): void;

	/**
	 * Remove all children from this view.
	 */
	removeAllChild(): void;

	/**
	 * Hit test in world or parent space.
	 * @param point World-space or compatible test point.
	 * @returns true if the point overlaps this view's hit region.
	 */
	overlapTest(point: Vec2): boolean;

	/**
	 * Returns a hash code / unique runtime identifier.
	 */
	hashCode(): Int;

	/**
	 * DOM: append this view to a parent view.
	 * @param parent Parent to append into.
	 * @returns this view.
	 */
	appendTo(parent: View): this;

	/**
	 * DOM: insert this view after a sibling.
	 * @param prev Sibling to insert after.
	 * @returns this view.
	 */
	afterTo(prev: View): this;

	/**
	 * Destroy this view in the context of its owning controller.
	 * @param owner The ViewController managing this view.
	 */
	destroy(owner: ViewController): void;

	/**
	 * Run a visual transition (animation) from an initial keyframe to a target keyframe.
	 * @param to   Target keyframe(s)
	 * @param from Optional starting keyframe(s)
	 */
	transition(to: KeyframeIn, from?: KeyframeIn): TransitionResult;

	/**
	 * Cast helper: return this view as a MorphView if it is one, otherwise null.
	 */
	asMorphView(): MorphView | null;

	/**
	 * Cast helper: return this view as an Entity if it is one, otherwise null.
	 */
	asEntity(): Entity | null;

	/**
	 * Cast helper: return this view as an Agent if it is one, otherwise null.
	 */
	asAgent(): Agent | null;

	/**
	 * Create a new View instance bound to a given Window.
	 * @param win Rendering / event window context.
	 */
	constructor(win: Window);

	/** Marker used by JSX/runtime to detect controllers. */
	static readonly isViewController: boolean;
}

/**
 * Box is a rectangular layout container.
 *
 * It introduces margin, padding, border, background, radius,
 * and shadow styling, plus "weight" and computed content size.
 *
 * @class Box
 * @extends View
 */
export declare class Box extends View {
	/** Clip children to this box's bounds. */
	clip: boolean;

	/**
	 * Whether to use **free layout mode** for child positioning.  
	 * 
	 * - If `true`, children are positioned freely (absolute mode).  
	 * - If `false`, the container uses **float layout** for its children (default).  
	 * 
	 * Default: `false`
	 */
	free: boolean;

	/** Alignment of this box inside its parent. */
	align: types.Align;

	/** Declared width (can be absolute, auto, percent, etc.). */
	width: types.BoxSize;
	/** Declared height. */
	height: types.BoxSize;

	/** Minimum width constraint. */
	minWidth: types.BoxSize;
	/** Minimum height constraint. */
	minHeight: types.BoxSize;
	/** Maximum width constraint. */
	maxWidth: types.BoxSize;
	/** Maximum height constraint. */
	maxHeight: types.BoxSize;

	/** Margin (top,right,bottom,left). */
	margin: number[];
	/** Margin top. */
	marginTop: number;
	/** Margin right. */
	marginRight: number;
	/** Margin bottom. */
	marginBottom: number;
	/** Margin left. */
	marginLeft: number;

	/** Padding (top,right,bottom,left). */
	padding: number[];
	/** Padding top. */
	paddingTop: number;
	/** Padding right. */
	paddingRight: number;
	/** Padding bottom. */
	paddingBottom: number;
	/** Padding left. */
	paddingLeft: number;

	/** Corner radii (tl,tr,br,bl). */
	borderRadius: number[];
	/** Corner radius top-left. */
	borderRadiusLeftTop: number;
	/** Corner radius top-right. */
	borderRadiusRightTop: number;
	/** Corner radius bottom-right. */
	borderRadiusRightBottom: number;
	/** Corner radius bottom-left. */
	borderRadiusLeftBottom: number;

	/** Border descriptors for each edge. */
	border: types.Border[];
	/** Border descriptor for the top edge. */
	borderTop: types.Border;
	/** Border descriptor for the right edge. */
	borderRight: types.Border;
	/** Border descriptor for the bottom edge. */
	borderBottom: types.Border;
	/** Border descriptor for the left edge. */
	borderLeft: types.Border;

	/** Border widths per edge (top,right,bottom,left). */
	borderWidth: number[];
	/** Border width top. */
	borderWidthTop: number;
	/** Border width right. */
	borderWidthRight: number;
	/** Border width bottom. */
	borderWidthBottom: number;
	/** Border width left. */
	borderWidthLeft: number;

	/** Border colors per edge (top,right,bottom,left). */
	borderColor: types.Color[];
	/** Border color top. */
	borderColorTop: types.Color;
	/** Border color right. */
	borderColorRight: types.Color;
	/** Border color bottom. */
	borderColorBottom: types.Color;
	/** Border color left. */
	borderColorLeft: types.Color;

	/** Solid background color. */
	backgroundColor: types.Color;

	/** Advanced background or filter effect. */
	background: types.BoxFilter | null;

	/** Drop shadow definition. */
	boxShadow: types.BoxShadow | null;

	/**
	 * Layout weight (e.g. Flex ratio).
	 * Often interpreted by parent layout containers.
	 */
	weight: Vec2;

	/**
	 * Final inner content size (width,height),
	 * not including this view's padding.
	 */
	readonly contentSize: Vec2;
}

/**
 * Flex is a box-level container that arranges children in a single line
 * (row or column) with alignment rules similar to flexbox concepts.
 *
 * @class Flex
 * @extends Box
 */
export declare class Flex extends Box {
	/** Main axis direction (row, column, etc.). */
	direction: types.Direction;

	/** Alignment of items along the main axis. */
	itemsAlign: types.ItemsAlign;

	/** Alignment of items along the cross axis. */
	crossAlign: types.CrossAlign;
}

/**
 * Flow is a flex-like container that supports wrapping,
 * similar to a multi-line flexbox layout.
 *
 * @class Flow
 * @extends Flex
 */
export declare class Flow extends Flex {
	/** Whether and how children wrap to the next line/column. */
	wrap: types.Wrap;

	/** Alignment of wrapped lines relative to the container. */
	wrapAlign: types.WrapAlign;
}

/**
 * Free is a box-style view without additional layout rules.
 * Useful for absolute/overlay-style positioning inside custom layouts.
 *
 * @class Free
 * @extends Box
 */
export declare class Free extends Box {
}

/**
 * Image is a drawable rectangular view that displays a texture or bitmap.
 *
 * @class Image
 * @extends Box
 */
export declare class Image extends Box {
	/** @event Fired when the image has successfully loaded. */
	readonly onLoad: EventNoticer<UIEvent>;

	/** @event Fired when the image failed to load. */
	readonly onError: EventNoticer<UIEvent>;

	/** Image source URL or resource identifier. */
	src: string;
}

/**
 * MorphView is a transform-capable view interface.
 * Anything that implements MorphView can be translated,
 * scaled, skewed, rotated, and exposes a transform matrix.
 *
 * @interface MorphView
 * @extends View
 */
export interface MorphView extends View {
	/** Translation in local space. */
	translate: Vec2;
	/** Scale factors along X and Y axes. */
	scale: Vec2;
	/** Skew factors for X and Y axes. */
	skew: Vec2;
	/** Anchor point(s) used for transformation origin. */
	origin: types.BoxOrigin[];
	/** X-axis anchor (e.g., left, center, right). */
	originX: types.BoxOrigin;
	/** Y-axis anchor (e.g., top, center, bottom). */
	originY: types.BoxOrigin;
	/** Local X position. */
	x: number;
	/** Local Y position. */
	y: number;
	/** X scale factor. */
	scaleX: number;
	/** Y scale factor. */
	scaleY: number;
	/** X-axis skew in degrees. */
	skewX: number;
	/** Y-axis skew in degrees. */
	skewY: number;
	/** Z-axis rotation in degrees. */
	rotateZ: number;
	/** Resolved origin offset values as `[x, y]`. */
	readonly originValue: number[];
	/** Combined transformation matrix (local-to-world). */
	readonly matrix: types.Mat;
}

/**
 * Morph is a Box that supports geometric transform (translate, scale, skew,
 * rotation, and origin). It is typically used as a transform root / group.
 *
 * @class Morph
 * @extends Box
 * @implements MorphView
 */
export declare class Morph extends Box implements MorphView {
	translate: Vec2;
	scale: Vec2;
	skew: Vec2;
	origin: types.BoxOrigin[];
	originX: types.BoxOrigin;
	originY: types.BoxOrigin;
	x: number;
	y: number;
	scaleX: number;
	scaleY: number;
	skewX: number;
	skewY: number;
	rotateZ: number;
	readonly originValue: number[];
	readonly matrix: types.Mat;
}


/*───────────────────────────────────────────
  Chapter 2 — Entity / Agent / World
───────────────────────────────────────────*/

/**
 * Entity is the base class for all drawable / interactive 2D world objects.
 *
 * An Entity:
 * - Participates in rendering and hit testing.
 * - Has world-space transform (via MorphView).
 * - Maintains geometric bounds (polygon / circle / etc.).
 * - Can exist inside a `World`.
 *
 * @class Entity
 * @extends View
 * @implements MorphView
 */
export declare class Entity extends View implements MorphView {
	translate: Vec2;
	scale: Vec2;
	skew: Vec2;
	origin: types.BoxOrigin[];
	originX: types.BoxOrigin;
	originY: types.BoxOrigin;
	x: number;
	y: number;
	scaleX: number;
	scaleY: number;
	skewX: number;
	skewY: number;
	rotateZ: number;
	readonly originValue: number[];
	readonly matrix: types.Mat;
	/**
	 * Geometric bounds for collision / hit testing.
	 * Examples: circle radius, polygon points, etc.
	 */
	bounds: types.Bounds;

	/**
	 * Whether this entity participates in the world as a detectable/collidable object.
	 * 
	 * If false, other agents will ignore this entity (no collision, no detection),
	 * but this entity may still actively collide or detect others if it is an Agent.
	 *
	 * Default: true
	 */
	participate: boolean;
}

/**
 * Agent is a moving Entity with basic navigation / AI behavior.
 *
 * An Agent:
 * - Can move toward a target position.
 * - Can follow a waypoint path.
 * - Can follow another Agent with distance constraints.
 * - Emits movement- and AI-related events.
 *
 * @class Agent
 * @extends Entity
 */
export declare abstract class Agent extends Entity {
	/**
	 * Fired when the agent reaches the next waypoint.
	 * @event
	 */
	readonly onReachWaypoint: EventNoticer<ReachWaypointEvent>;

	/**
	 * Fired when the agent arrives at its final destination.
	 * @event
	 */
	readonly onAgentMovement: EventNoticer<AgentMovementEvent>;

	/**
	 * Fired when an agent is discovered (enters range) or lost (leaves range).
	 * @event
	 */
	readonly onDiscoveryAgent: EventNoticer<DiscoveryAgentEvent>;

	/**
	 * Fired when the agent's heading direction changes.
	 * @event
	*/
	readonly onAgentHeadingChange: EventNoticer<AgentStateEvent>;

	/**
	* Whether the agent is currently active (moving or processing behavior). 
	* Default: true
	*/
	active: boolean;

	/**
	 * Whether the agent is currently moving toward a target or along waypoints or following another agent.
	 */
	readonly moving: boolean;

	/**
	 * Indicates if the agent’s standing position is floating (soft).
	 * 
	 * When enabled, the agent does not defend its current spot after arrival
	 * and can be displaced by nearby agents—useful for group formations or
	 * crowding around large targets.
	 * Default: false
	 */
	floatingStation: boolean;

	/** Waypoints path for the agent to navigate. */
	waypoints: Path | null;

	/** Direct movement destination in world coordinates. */
	readonly target: Vec2;

	/** Current avoidance velocity steering vector in world coordinates. */
	readonly velocitySteer: Vec2;

	/** Current real velocity vector in world coordinates. */
	readonly velocity: Vec2;

	/**
	 * Behavior heading direction (normalized).
	 * The agent's intended movement direction based on path/follow logic.
	 * Not necessarily equal to velocity direction and does not jitter.
	 */
	readonly heading: Vec2;

	/** Maximum allowed movement speed. */
	velocityMax: Float;

	/** Index of the current waypoint along the path. */
	readonly currentWaypoint: Uint;

	/**
	 * Discovery radii for proximity checks.
	 * Each element is a threshold band for detection.
	 */
	discoveryDistances: Float[];

	/**
	 * Safety buffer distance for local avoidance.
	 * Higher = keep more distance from obstacles/agents.
	 */
	safetyBuffer: Float;

	/**
	 * Avoidance strength multiplier during collision resolution.
	 * Default is 1.0f, range [0.0, 10.0].
	 */
	avoidanceFactor: Float;

	/**
	 * Maximum avoidance velocity applied when steering away from obstacles.
	 * Default is 0.8f, range [0.0, 3.0], recommended range [0.0, 1.0].
	 */
	avoidanceVelocityFactor: Float;

	/**
	 * Distance min range maintained while following a target agent.
	 */
	followMinDistance: Float;

	/**
	 * Distance max range maintained while following a target agent.
	 */
	followMaxDistance: Float;

	/**
	 * The agent currently being followed.
	 * If null, no follow behavior is active.
	 */
	followTarget: Agent | null;

	/**
	 * Move toward a specific position.
	 * @param target Destination position (world coords).
	 * @param immediately If true, teleport or snap immediately.
	 */
	moveTo(target: Vec2, immediately?: boolean): void;

	/**
	 * Assign a list of waypoints for navigation.
	 * @param waypoints Path object representing the navigation route.
	 * @param immediately If true, begin from the nearest waypoint now.
	 */
	setWaypoints(waypoints: Path, immediately?: boolean): void;

	/**
	 * Rejoin the assigned waypoint path from the closest segment.
	 * @param immediately If true, snap directly to that segment.
	 */
	returnToWaypoints(immediately?: boolean): void;

	/**
	 * clear current movement (waypoints or follow target).
	 * and set target to current position.
	 */
	stop(): void;
}

/**
 * Sprite is a renderable Agent that draws from a sprite sheet / atlas.
 *
 * Features:
 * - Frame-based animation (rows/cols).
 * - Playback control (play / stop).
 * - Directional facing.
 *
 * @class Sprite
 * @extends Agent
 */
export declare class Sprite extends Agent {
	/** @event Fired when sprite asset / texture is loaded. */
	readonly onLoad: EventNoticer<UIEvent>;
	/** @event Fired if sprite asset fails to load. */
	readonly onError: EventNoticer<UIEvent>;

	/** Sprite source (image / atlas). */
	src: string;

	/** Rendered width of the sprite (px / world units). */
	width: Float;

	/** Rendered height of the sprite. */
	height: Float;

	/** Current frame index. */
	frame: Uint16;

	/** Total frame count in the animation grid. */
	frames: Uint16;

	/** The current set of the sprite animation, default 0. */
	set: Uint16;

	/** The number of sets in the sprite animation, default 1. */
	sets: Uint16;

	/**
	 * Spacing between frames in the sprite sheet (px).
	*/
	spacing: Uint8;

	/** Animation playback frequency (frames per second). */
	frequency: Uint8;

	/** Facing or motion direction. */
	direction: types.Direction;

	/** Whether the sprite is currently playing an animation. */
	playing: boolean;

	/** Begin playback of the active animation. */
	play(): void;

	/** Stop playback and hold the current frame. */
	stop(): void;
}

/**
 * Spine is an animated skeletal Agent driven by Spine runtime data.
 *
 * Features:
 * - Multiple animation tracks.
 * - Mixing, events, callbacks.
 * - Runtime skin changes.
 *
 * @class Spine
 * @extends Agent
 */
export declare class Spine extends Agent {
	/** @event Spine animation track started. */
	readonly onSpineStart: EventNoticer<SpineEvent>;
	/** @event Spine animation interrupted (cut by another). */
	readonly onSpineInterrupt: EventNoticer<SpineEvent>;
	/** @event Spine animation track ended. */
	readonly onSpineEnd: EventNoticer<SpineEvent>;
	/** @event Spine track disposed/cleaned. */
	readonly onSpineDispose: EventNoticer<SpineEvent>;
	/** @event Spine animation loop completed. */
	readonly onSpineComplete: EventNoticer<SpineEvent>;
	/** @event Spine user event fired (custom markers). */
	readonly onSpineEvent: EventNoticer<SpineExtEvent>;

	/** Spine skeleton data (bones, slots, attachments). */
	skel: types.SkeletonData | null;

	/** Current active skin name. */
	skin: string;

	/** Global playback speed scale. */
	speed: Float;

	/** Default crossfade/mix duration between animations. */
	defaultMix: Float;

	/**
	 * Get/Set current animation name for track 0 (the base track).
	 */
	animation: string;

	/** Reset full skeleton pose to setup state. */
	setToSetupPose(): void;

	/** Reset bones to setup pose. */
	setBonesToSetupPose(): void;

	/** Reset slots (attachments/visuals) to setup pose. */
	setSlotsToSetupPose(): void;

	/**
	 * Attach a specific attachment into a slot.
	 * @param slotName Target slot.
	 * @param attachmentName Attachment to bind.
	 */
	setAttachment(slotName: string, attachmentName: string): void;

	/**
	 * Define mix duration when transitioning from one animation to another.
	 * @param fromName Source animation.
	 * @param toName Target animation.
	 * @param duration Crossfade time.
	 */
	setMix(fromName: string, toName: string, duration: Float): void;

	/**
	 * Set an animation on a given track.
	 * @param trackIndex Track number.
	 * @param name Animation name.
	 * @param loop Whether to loop.
	 */
	setAnimation(trackIndex: Uint, name: string, loop?: boolean): void;

	/**
	 * Queue another animation after the current one.
	 * @param trackIndex Track number.
	 * @param name Animation name.
	 * @param loop Whether to loop.
	 * @param delay Delay before it starts.
	 */
	addAnimation(trackIndex: Uint, name: string, loop?: boolean, delay?: Float): void;

	/**
	 * Set an "empty" animation (used to smoothly fade out pose).
	 * @param trackIndex Track number.
	 * @param mixDuration Fade duration.
	 */
	setEmptyAnimation(trackIndex: Uint, mixDuration: Float): void;

	/**
	 * Apply "empty" animation to all tracks with default mix.
	 * Used to smoothly clear pose.
	 * @param mixDuration Fade duration.
	 */
	setEmptyAnimations(mixDuration: Float): void;

	/**
	 * Queue an empty animation after current.
	 * @param trackIndex Track number.
	 * @param mixDuration Fade duration.
	 * @param delay Delay before it starts.
	 */
	addEmptyAnimation(trackIndex: Uint, mixDuration: Float, delay?: Float): void;

	/** Clear all animation tracks. */
	clearTracks(): void;

	/**
	 * Clear a specific animation track.
	 * @param trackIndex Track number to clear. If omitted, may clear default.
	 */
	clearTrack(trackIndex?: Uint): void;
}

/**
 * World is a transformable container (`Morph`) that simulates a 2D world
 * for `Entity` and `Agent` objects.
 *
 * World responsibilities:
 * - Run per-frame / sub-step updates.
 * - Drive agent movement, avoidance and discovery logic.
 * - Act as the spatial root / coordinate space for gameplay logic.
 *
 * Pausing `World` (playing = false) freezes simulation but keeps visuals.
 *
 * @class World
 * @extends Morph
 */
export declare class World extends Morph {
	/**
	 * Whether the world is actively simulating.
	 * If false, agents remain visible but do not update or move.
	 */
	playing: boolean;

	/**
	 * Number of physics / navigation sub-steps per frame.
	 * Higher values improve stability at high speeds.
	 * Typical range: 1–5.
	 */
	subSteps: Uint;

	/**
	 * Global time scaling factor.
	 * 1.0 = realtime, <1.0 = slow motion, >1.0 = fast-forward.
	 */
	timeScale: Float;

	/**
	 * Prediction horizon (in seconds) used for avoidance.
	 * Agents steer based on projected future positions in this time window.
	 */
	predictionTime: Float;

	/**
	 * Buffer distance added to discovery thresholds to prevent flicker.
	 * Helps avoid repeated enter/leave spam when agents hover near edge.
	 */
	discoveryThresholdBuffer: Float;

	/**
	 * Radius (in world units) around waypoints that counts as "reached".	
	 * When an agent comes within this distance of a waypoint, it will proceed to the next one.
	 * Default: 0.0f
	 */
	waypointRadius: Float;
}

/**
 * Root is a top-level Morph typically used as the root of a scene or UI tree.
 * It often serves as the mount point for a `Window` or `ViewController`.
 *
 * @class Root
 * @extends Morph
 */
export declare class Root extends Morph {
}


/*───────────────────────────────────────────
  Chapter 3 — Text / Input / Scroll / Media
───────────────────────────────────────────*/

/**
 * TextOptions describes common text styling and measurement APIs
 * shared by text-capable views (Text, Label, Input, etc.).
 *
 * @interface TextOptions
 */
export interface TextOptions {
	/** Font style bitmask / ID (implementation-defined). */
	readonly fontStyle: number;

	/** Horizontal text alignment. */
	textAlign: types.TextAlign;

	/** Font weight. */
	textWeight: types.TextWeight;

	/** Font slant / italic style. */
	textSlant: types.TextSlant;

	/** Text decoration (underline, strike, etc.). */
	textDecoration: types.TextDecoration;

	/** Overflow handling (clip, ellipsis, etc.). */
	textOverflow: types.TextOverflow;

	/** Whitespace handling. */
	textWhiteSpace: types.TextWhiteSpace;

	/** Word-break rule. */
	textWordBreak: types.TextWordBreak;

	/** Font size. */
	textSize: types.TextSize;

	/** Background color for text glyphs. */
	textBackgroundColor: types.TextColor;

	/** Stroke/outline style for text. */
	textStroke: types.TextStroke;

	/** Primary text color. */
	textColor: types.TextColor;

	/** Line height. */
	textLineHeight: types.TextSize;

	/** Shadow styling for text. */
	textShadow: types.TextShadow;

	/** Font family / fallback list. */
	textFamily: types.TextFamily;

	/**
	 * Measure the rendered layout size of a text string,
	 * using this object's current font / style settings.
	 * @param text Text to measure.
	 * @returns Size in px/world units (w,h).
	 */
	computeLayoutSize(text: string): Vec2;
}

/**
 * Text is a non-editable text view with rich styling support.
 * It can measure, wrap, truncate, etc.
 *
 * @class Text
 * @extends Box
 * @implements TextOptions
 */
export declare class Text extends Box implements TextOptions {
	readonly fontStyle: number;
	textAlign: types.TextAlign;
	textWeight: types.TextWeight;
	textSlant: types.TextSlant;
	textDecoration: types.TextDecoration;
	textOverflow: types.TextOverflow;
	textWhiteSpace: types.TextWhiteSpace;
	textWordBreak: types.TextWordBreak;
	textSize: types.TextSize;
	textBackgroundColor: types.TextColor;
	textStroke: types.TextStroke;
	textColor: types.TextColor;
	textLineHeight: types.TextSize;
	textShadow: types.TextShadow;
	textFamily: types.TextFamily;

	/** Actual displayed string content. */
	value: string;

	/** Measure size for the given text using current style. */
	computeLayoutSize(text: string): Vec2;
}

/**
 * Button is an interactive Text view that can be navigated,
 * focused, and "clicked". It can also expose directional
 * navigation helpers for gamepad/keyboard UIs.
 *
 * @class Button
 * @extends Text
 */
export declare class Button extends Text {
	/**
	 * Query the next logical button in a given direction.
	 * Useful for D-pad / keyboard navigation grids.
	 * @param dir Direction to search.
	 * @returns The neighbor Button or null.
	 */
	nextButton(dir: types.Direction): Button | null;
}

/**
 * Label is a lightweight text-bearing View.
 * It behaves similarly to Text, but does not inherit Box layout
 * (so it's cheaper and may integrate differently with layout).
 *
 * @class Label
 * @extends View
 * @implements TextOptions
 */
export declare class Label extends View implements TextOptions {
	readonly fontStyle: number;
	textAlign: types.TextAlign;
	textWeight: types.TextWeight;
	textSlant: types.TextSlant;
	textDecoration: types.TextDecoration;
	textOverflow: types.TextOverflow;
	textWhiteSpace: types.TextWhiteSpace;
	textWordBreak: types.TextWordBreak;
	textSize: types.TextSize;
	textBackgroundColor: types.TextColor;
	textStroke: types.TextStroke;
	textColor: types.TextColor;
	textLineHeight: types.TextSize;
	textShadow: types.TextShadow;
	textFamily: types.TextFamily;

	/** Text content for display. */
	value: string;

	/** Measure size for the given text using current style. */
	computeLayoutSize(text: string): Vec2;
}

/**
 * Input is a single-line editable text box.
 *
 * Features:
 * - Cursor, max length, secure entry.
 * - Software keyboard hints (type, return key).
 * - Inline styling consistent with TextOptions.
 *
 * @class Input
 * @extends Box
 * @implements TextOptions
 */
export declare class Input extends Box implements TextOptions {
	/** @event Fired when the value changes (user edit). */
	readonly onChange: EventNoticer<UIEvent>;

	readonly fontStyle: number;
	textAlign: types.TextAlign;
	textWeight: types.TextWeight;
	textSlant: types.TextSlant;
	textDecoration: types.TextDecoration;
	textOverflow: types.TextOverflow;
	textWhiteSpace: types.TextWhiteSpace;
	textWordBreak: types.TextWordBreak;
	textSize: types.TextSize;
	textBackgroundColor: types.TextColor;
	textColor: types.TextColor;
	textLineHeight: types.TextSize;
	textShadow: types.TextShadow;
	textFamily: types.TextFamily;
	textStroke: types.TextStroke;

	/** If true, mask input (password-style). */
	security: boolean;

	/** Whether the field is read-only. */
	readonly: boolean;

	/** Keyboard type hint for virtual keyboards. */
	type: types.KeyboardType;

	/** Return/enter key style hint. */
	returnType: types.KeyboardReturnType;

	/** Placeholder text color. */
	placeholderColor: types.Color;

	/** Caret (cursor) color. */
	cursorColor: types.Color;

	/** Maximum allowed text length. */
	maxLength: number;

	/** Current text value. */
	value: string;

	/** Placeholder text when empty. */
	placeholder: string;

	/** Current text length (may differ from value.length due to encoding). */
	readonly textLength: number;

	/** Measure size for the given text using current style. */
	computeLayoutSize(text: string): Vec2;
}

/**
 * ScrollView describes scrollable behavior and parameters.
 * It's implemented by Scroll and Textarea.
 *
 * @interface ScrollView
 * @extends Box
 */
export interface ScrollView extends Box {
	/** Whether scrollbars are shown. */
	scrollbar: boolean;

	/** Whether content can bounce (rubber-band). */
	bounce: boolean;

	/**
	 * If true, bouncing locks when edge is reached
	 * to reduce jitter / overscroll chaining.
	 */
	bounceLock: boolean;

	/** Whether inertial/momentum scrolling is enabled. */
	momentum: boolean;

	/** If true, lock scroll to the first detected direction. */
	lockDirection: boolean;

	/** Current scroll X offset. */
	scrollX: number;

	/** Current scroll Y offset. */
	scrollY: number;

	/** Current scroll offset as Vec2. */
	scroll: Vec2;

	/** Scroll resistance factor. */
	resistance: number;

	/** Sticky "catch" position X (e.g. snap zones). */
	catchPositionX: number;

	/** Sticky "catch" position Y. */
	catchPositionY: number;

	/** Scrollbar color. */
	scrollbarColor: types.Color;

	/** Scrollbar thickness. */
	scrollbarWidth: number;

	/** Scrollbar margin from the edge. */
	scrollbarMargin: number;

	/** Scroll animation duration for programmatic scrolls (ms). */
	scrollDuration: number;

	/** Default easing curve for programmatic scrolls. */
	defaultCurve: types.Curve;

	/** Whether horizontal scrollbar is active. */
	readonly scrollbarH: boolean;

	/** Whether vertical scrollbar is active. */
	readonly scrollbarV: boolean;

	/** Total scrollable content size. */
	readonly scrollSize: Vec2;

	/**
	 * Smooth-scroll to a given offset.
	 * @param val Target scroll offset.
	 * @param duration Optional animation duration.
	 * @param curve Optional easing curve.
	 */
	scrollTo(val: Vec2, duration?: number, curve?: types.Curve): void;

	/**
	 * Abort any running scroll animation / momentum.
	 */
	terminate(): void;
}

/**
 * Textarea is a multi-line text input field with its own scrollable
 * viewport. It merges text editing and ScrollView behaviors.
 *
 * @class Textarea
 * @extends Input
 * @implements ScrollView
 */
export declare class Textarea extends Input implements ScrollView {
	/** @event Fired as the scroll offset changes. */
	readonly onScroll: EventNoticer<UIEvent>;

	scrollbar: boolean;
	bounce: boolean;
	bounceLock: boolean;
	momentum: boolean;
	lockDirection: boolean;
	scrollX: number;
	scrollY: number;
	scroll: Vec2;
	resistance: number;
	catchPositionX: number;
	catchPositionY: number;
	scrollbarColor: types.Color;
	scrollbarWidth: number;
	scrollbarMargin: number;
	scrollDuration: number;
	defaultCurve: types.Curve;
	readonly scrollbarH: boolean;
	readonly scrollbarV: boolean;
	readonly scrollSize: Vec2;

	scrollTo(val: Vec2, duration?: number, curve?: types.Curve): void;
	terminate(): void;
}

/**
 * Scroll is a generic scrollable container.
 * It can host arbitrary child content and provides ScrollView APIs.
 *
 * @class Scroll
 * @extends Box
 * @implements ScrollView
 */
export declare class Scroll extends Box implements ScrollView {
	/** @event Fired as the scroll offset changes. */
	readonly onScroll: EventNoticer<UIEvent>;

	scrollbar: boolean;
	bounce: boolean;
	bounceLock: boolean;
	momentum: boolean;
	lockDirection: boolean;
	scrollX: number;
	scrollY: number;
	scroll: Vec2;
	resistance: number;
	catchPositionX: number;
	catchPositionY: number;
	scrollbarColor: types.Color;
	scrollbarWidth: number;
	scrollbarMargin: number;
	scrollDuration: number;
	defaultCurve: types.Curve;
	readonly scrollbarH: boolean;
	readonly scrollbarV: boolean;
	readonly scrollSize: Vec2;

	scrollTo(val: Vec2, duration?: number, curve?: types.Curve): void;
	terminate(): void;
}

/**
 * Video is a playable media view that extends Image and implements Player.
 *
 * It exposes playback control (play/pause/stop/seek),
 * audio track switching, mute/volume control, and metadata such as duration.
 *
 * @class Video
 * @extends Image
 * @implements Player
 */
export declare class Video extends Image implements Player {
	readonly pts: number;
	volume: number;
	mute: boolean;
	readonly isPause: boolean;
	readonly type: MediaType;
	readonly duration: number;
	readonly status: MediaSourceStatus;
	readonly video: Stream | null;
	readonly audio: Stream | null;
	readonly audioStreams: number;
	play(): void;
	pause(): void;
	stop(): void;
	seek(timeMs: number): void;
	switchAudio(index: number): void;
	/** @event Fired when playback stops. */
	readonly onStop: EventNoticer<UIEvent>;
	/** @event Fired while buffering / loading. */
	readonly onBuffering: EventNoticer<UIEvent>;
}

/**
 * test overlap point in convex quadrilateral
*/
export declare function testOverlapFromConvexQuadrilateral(quadrilateral: Vec2[], point: Vec2): boolean;

/**
 * Represents the minimum translation vector required to separate two overlapping convex shapes.
*/
export interface MinimumTranslationVector {
	/** The axis along which the minimum translation should occur. */
	axis: Vec2;
	/** The magnitude of the overlap along the axis. */
	overlap: Float;
};

/**
 * alias MinimumTranslationVector
*/
export type MTV = MinimumTranslationVector;

/**
 * Test overlap from convex polygons with SAT and GJK algorithm and get minimum translation vector.
 * @param poly1 The vertices of the first convex polygon.
 * @param poly2 The vertices of the second convex polygon.
 * @param outMTV? Optional output parameter to receive the minimum translation vector to separate the polygons.
 * @param requestSeparationMTV? Optional flag to compute the minimum translation vector even when the polygons are separated.
 * @returns Returns true if the polygons overlap, false otherwise.
*/
export declare function testPolygonVsPolygon(poly1: Vec2[], poly2: Vec2[], 
												outMTV?: Partial<MTV>, requestSeparationMTV?: boolean): boolean;

const _ui = __binding__('_ui');

Object.assign(exports, {
	View: _ui.View,
	Box: _ui.Box,
	Flex: _ui.Flex,
	Flow: _ui.Flow,
	Free: _ui.Free,
	Image: _ui.Image,
	Video: _ui.Video,
	Input: _ui.Input,
	Textarea: _ui.Textarea,
	Label: _ui.Label,
	Scroll: _ui.Scroll,
	Text: _ui.Text,
	Button: _ui.Button,
	Morph: _ui.Morph,
	Entity: _ui.Entity,
	Agent: _ui.Agent,
	Sprite: _ui.Sprite,
	Spine: _ui.Spine,
	World: _ui.World,
	Root: _ui.Root,
	testOverlapFromConvexQuadrilateral: _ui.testOverlapFromConvexQuadrilateral,
	testOverlapFromConvexPolygons: _ui.testOverlapFromConvexPolygons,
});

// JSX IntrinsicElements
// -------------------------------------------------------------------------------
declare global {
	namespace JSX {
		interface ViewJSX {
			onClick?: Listen<ClickEvent, View> | null;
			onBack?: Listen<ClickEvent, View> | null;
			onKeyDown?: Listen<KeyEvent, View> | null;
			onKeyPress?: Listen<KeyEvent, View> | null;
			onKeyUp?: Listen<KeyEvent, View> | null;
			onKeyEnter?: Listen<KeyEvent, View> | null;
			onTouchStart?: Listen<TouchEvent, View> | null;
			onTouchMove?: Listen<TouchEvent, View> | null;
			onTouchEnd?: Listen<TouchEvent, View> | null;
			onTouchCancel?: Listen<TouchEvent, View> | null;
			onMouseOver?: Listen<MouseEvent, View> | null;
			onMouseOut?: Listen<MouseEvent, View> | null;
			onMouseLeave?: Listen<MouseEvent, View> | null;
			onMouseEnter?: Listen<MouseEvent, View> | null;
			onMouseMove?: Listen<MouseEvent, View> | null;
			onMouseDown?: Listen<MouseEvent, View> | null;
			onMouseUp?: Listen<MouseEvent, View> | null;
			onMouseWheel?: Listen<MouseEvent, View> | null;
			onFocus?: Listen<UIEvent, View> | null;
			onBlur?: Listen<UIEvent, View> | null;
			onHighlighted?: Listen<HighlightedEvent, View> | null;
			onActionKeyframe?: Listen<ActionEvent, View> | null;
			onActionLoop?: Listen<ActionEvent, View> | null;
			ref?: string;
			key?: string|number;
			style?: StyleSheets;
			action?: action.ActionIn | null;
			class?: string | string[];
			color?: types.ColorIn;
			cascadeColor?: types.CascadeColorIn;
			cursor?: types.CursorStyleIn;
			zIndex?: Uint;
			opacity?: Float;
			visible?: boolean;
			receive?: boolean;
			aa?: boolean;
			isFocus?: boolean;
		}

		interface BoxJSX extends ViewJSX {
			clip?: boolean;
			free?: boolean;
			align?: types.AlignIn;
			width?: types.BoxSizeIn;
			height?: types.BoxSizeIn;
			minWidth?: types.BoxSizeIn;
			minHeight?: types.BoxSizeIn;
			maxWidth?: types.BoxSizeIn;
			maxHeight?: types.BoxSizeIn;
			margin?: number[] | number;
			marginTop?: number;
			marginRight?: number;
			marginBottom?: number;
			marginLeft?: number;
			padding?: number[] | number;
			paddingTop?: number;
			paddingRight?: number;
			paddingBottom?: number;
			paddingLeft?: number;
			borderRadius?: number[] | number;
			borderRadiusLeftTop?: number;
			borderRadiusRightTop?: number;
			borderRadiusRightBottom?: number;
			borderRadiusLeftBottom?: number;
			border?: types.BorderIn[] | types.BorderIn; // border
			borderTop?: types.BorderIn;
			borderRight?: types.BorderIn;
			borderBottom?: types.BorderIn;
			borderLeft?: types.BorderIn;
			borderWidth?: number[] | number;
			borderColor?: types.ColorIn[] | types.ColorIn;
			borderWidthTop?: number; // border width
			borderWidthRight?: number;
			borderWidthBottom?: number;
			borderWidthLeft?: number;
			borderColorTop?: types.ColorIn; // border color
			borderColorRight?: types.ColorIn;
			borderColorBottom?: types.ColorIn;
			borderColorLeft?: types.ColorIn;
			backgroundColor?: types.ColorIn;
			background?: types.BoxFilterIn;
			boxShadow?: types.BoxShadowIn;
			weight?: Vec2In;
		}

		interface FlexJSX extends BoxJSX {
			direction?: types.DirectionIn;
			itemsAlign?: types.ItemsAlignIn;
			crossAlign?: types.CrossAlignIn;
		}

		interface FlowJSX extends FlexJSX {
			wrap?: types.WrapIn;
			wrapAlign?: types.WrapAlignIn;
		}

		interface FreeJSX extends BoxJSX {
		}

		interface ImageJSX extends BoxJSX {
			onLoad?: Listen<UIEvent, Image> | null;
			onError?: Listen<UIEvent, Image> | null;
			src?: string;
		}

		interface VideoJSX extends ImageJSX {
			onStop?: Listen<UIEvent, Image> | null;
			onLoading?: Listen<UIEvent, Image> | null;
			volume?: Float;
			mute?: boolean;
		}

		interface MorphViewJSX {
			translate?: Vec2In;
			scale?: Vec2In;
			skew?: Vec2In;
			origin?: types.BoxOriginIn[] | types.BoxOriginIn
			originX?: types.BoxOriginIn;
			originY?: types.BoxOriginIn;
			x?: Float;
			y?: Float;
			scaleX?: Float;
			scaleY?: Float;
			skewX?: Float;
			skewY?: Float;
			rotateZ?: Float;
		}

		interface MorphJSX extends BoxJSX, MorphViewJSX {
		}

		interface EntityJSX extends ViewJSX, MorphViewJSX {
			bounds?: types.BoundsIn;
			participate?: boolean;
		}

		interface AgentJSX extends EntityJSX {
			onReachWaypoint?: Listen<ReachWaypointEvent, Agent> | null;
			onDiscoveryAgent?: Listen<DiscoveryAgentEvent, Agent> | null;
			onAgentMovement?: Listen<AgentMovementEvent, Agent> | null;
			onAgentHeadingChange?: Listen<AgentStateEvent, Agent> | null;
			active?: boolean;
			floatingStation?: boolean;
			velocityMax?: number;
			discoveryDistances?: number | number[];
			safetyBuffer?: number;
			avoidanceFactor?: number;
			avoidanceVelocityFactor?: number;
			followMinDistance?: number;
			followMaxDistance?: number;
			followTarget?: Agent | null;
			waypoints?: Path | null;
		}

		interface SpriteJSX extends AgentJSX {
			onLoad?: Listen<UIEvent, Sprite> | null;
			onError?: Listen<UIEvent, Sprite> | null;
			src?: string;
			width?: Float;
			height?: Float;
			frame?: Uint16;
			frames?: Uint16;
			set?: Uint16;
			sets?: Uint16;
			spacing?: Uint8;
			frequency?: Uint8;
			direction?: types.DirectionIn;
			playing?: boolean;
		}

		interface SpineJSX extends AgentJSX {
			onSpineStart?: Listen<SpineEvent, Spine> | null;
			onSpineInterrupt?: Listen<SpineEvent, Spine> | null;
			onSpineEnd?: Listen<SpineEvent, Spine> | null;
			onSpineDispose?: Listen<SpineEvent, Spine> | null;
			onSpineComplete?: Listen<SpineEvent, Spine> | null;
			onSpineEvent?: Listen<SpineExtEvent, Spine> | null;
			skel?: types.SkeletonDataIn;
			skin?: string;
			speed?: Float;
			defaultMix?: Float;
			animation?: string;
		}

		interface WorldJSX extends MorphJSX {
			playing?: boolean;
			subSteps?: Uint;
			timeScale?: Float;
			predictionTime?: Float;
			discoveryThresholdBuffer?: Float;
			waypointRadius?: Float;
		}

		interface TextOptionsJSX {
			textAlign?: types.TextAlignIn;
			textWeight?: types.TextWeightIn;
			textSlant?: types.TextSlantIn;
			textDecoration?: types.TextDecorationIn;
			textOverflow?: types.TextOverflowIn;
			textWhiteSpace?: types.TextWhiteSpaceIn;
			textWordBreak?: types.TextWordBreakIn;
			textSize?: types.TextSizeIn;
			textBackgroundColor?: types.TextColorIn;
			textColor?: types.TextColorIn;
			textLineHeight?: types.TextSizeIn;
			textShadow?: types.TextShadowIn;
			textFamily?: types.TextFamilyIn;
			textStroke?: types.TextStrokeIn;
		}

		interface TextJSX extends BoxJSX, TextOptionsJSX {
			value?: string;
		}

		interface ButtonJSX extends TextJSX {
		}

		interface LabelJSX extends ViewJSX, TextOptionsJSX {
			value?: string;
		}

		interface InputJSX extends BoxJSX, TextOptionsJSX {
			onChange?: Listen<UIEvent, Input> | null;
			security?: boolean;
			readonly?: boolean;
			type?: types.KeyboardTypeIn;
			returnType?: types.KeyboardReturnTypeIn;
			placeholderColor?: types.ColorIn;
			cursorColor?: types.ColorIn;
			maxLength?: number;
			placeholder?: string;
			value?: string;
		}

		interface ScrollViewJSX {
			scrollbar?: boolean;
			bounce?: boolean;
			bounceLock?: boolean;
			momentum?: boolean;
			lockDirection?: boolean;
			scrollX?: number;
			scrollY?: number;
			scroll?: Vec2In;
			resistance?: number;
			catchPositionX?: number;
			catchPositionY?: number;
			scrollbarColor?: types.ColorIn;
			scrollbarWidth?: number;
			scrollbarMargin?: number;
			scrollDuration?: number;
			defaultCurve?: types.CurveIn;
		}

		interface TextareaJSX extends InputJSX, ScrollViewJSX {
			onScroll?: Listen<UIEvent, Textarea> | null;
		}

		interface ScrollJSX extends BoxJSX, ScrollViewJSX {
			onScroll?: Listen<UIEvent, Scroll> | null;
		}

		interface IntrinsicElements {
			view: ViewJSX;
			box: BoxJSX;
			flex: FlexJSX;
			flow: FlowJSX;
			free: FreeJSX;
			image: ImageJSX;
			img: ImageJSX;
			morph: MorphJSX;
			entity: EntityJSX;
			sprite: SpriteJSX;
			spine: SpineJSX;
			text: TextJSX;
			button: ButtonJSX;
			label: LabelJSX;
			input: InputJSX;
			textarea: TextareaJSX;
			scroll: ScrollJSX;
			video: VideoJSX;
			world: WorldJSX;
		}

		type IntrinsicElementsName = keyof IntrinsicElements;
	}
}

// extend view impl
// ----------------------------------------------------------------------------

const NN_getNoticer = NativeNotification.prototype.getNoticer;
// global touch point id set
const gGestureManagerSet: Set<GestureManager> = new Set();
// is listening root touchend/touchcancel event
let isListeningRootTouchEnd: boolean = false;

const _init = __binding__('_init');

const FingerCounts = {
	[GestureType.PanGesture]: 1,
	[GestureType.PinchGesture]: 2,
	[GestureType.RotateGesture]: 2,
	[GestureType.ThreeFingerGesture]: 3,
	[GestureType.FourFingerGesture]: 4,
};

interface GestureEventInl extends RemoveReadonly<GestureEvent> {
	_update(timestamp: Uint, stage: GestureStage): void;
}

class GestureManager {
	private _view: View;
	private _gesture?: EventNoticer<GestureEvent>;
	private _swipe?: EventNoticer<GestureEvent>;
	private _noticers: Map<GestureType, EventNoticer<GestureEvent>> = new Map();
	private _sorts: {type: GestureType, evt?: GestureEventInl}[] = []; // sorted gesture types
	private _eventsFlow: (GestureEventInl|null)[] = []; // gesture events context
	private _touches: Map<Uint, [GestureTouchPoint, GestureEventInl]> = new Map(); // touch.id=>gt,Event

	// delete zombie touch points when global touchend/touchcancel event triggered
	private static deleteZombieTouchPoint(e: TouchEvent) {
		for (const gm of gGestureManagerSet) {
			if (gm._touches.size !== 0) {
				for (const {id} of e.changedTouches) {
					const touch = gm._touches.get(id);
					if (touch) {
						gm._touches.delete(id);
						gm._eventsFlow[touch[1].id] = null; // clear event flow
					}
				}
			}
			if (gm._touches.size === 0) {
				gGestureManagerSet.delete(gm);
			}
		}
	}

	constructor(view: View) {
		this._view = view;
		this._view.onTouchStart.on(this._handleTouchStart, this, '-1');
		this._view.onTouchMove.on(this._handleTouchMove, this, '-1');
		this._view.onTouchEnd.on(this._handleTouchEnd, this, '-1');
		this._view.onTouchCancel.on(this._handleTouchCancel, this, '-1');
		if (!isListeningRootTouchEnd) {
			isListeningRootTouchEnd = true;
			// listen root touchend/touchcancel event to delete zombie touch points.
			// by native event listener to avoid event listeners remove.
			_init.addNativeEventListener(view.window.root, "TouchEnd", GestureManager.deleteZombieTouchPoint, 1);
			_init.addNativeEventListener(view.window.root, "TouchCancel", GestureManager.deleteZombieTouchPoint, 1);
		}
	}

	private getNewEventId() {
		for (let i = 0; i < this._eventsFlow.length; i++) {
			if (!this._eventsFlow[i])
				return i;
		}
		return this._eventsFlow.length;
	}

	private _dispatchStart(evt: GestureEventInl, touchs: TouchPoint[], timestamp: Uint, rejectDiscard: boolean) {
		let i = 0;
		do {
			const touch = touchs[i];
			util.assert(!this._touches.has(touch.id), 'Gesture point already exists');

			if (evt.sealed) {
				touchs.splice(0, touchs.length); // discard all points
				return; // event sealed, next event
			}
			if (evt.length >= evt.expectedFingerCount) {
				return; // next event
			}

			const gt = new GestureTouchPoint(touch);
			evt.touchs.push(gt);
			evt._update(timestamp, GestureStage.Start);

			if (this._gesture) {
				this._gesture.triggerWithEvent(evt as any);
			}

			if (evt.rejected) { // reject by touch point
				evt.rejected = false; // reset flag
				if (rejectDiscard) { // discard point
					touchs.splice(i, 1); // consumption point
				} else {
					evt.touchs.pop();
					i++;
				}
				continue; // next point
			}

			if (evt.isDefault) {
				let mask = 0; // mask is 0: none, 2: pan, 4: pinch, 8: rotate
				for (const sort of this._sorts) {
					// Distribute routing event flow
					if (!sort.evt) { // not bind event
						const noticer = this._noticers.get(sort.type)!;
						const fingerCount = FingerCounts[sort.type as 2|3|4|5|6] || 1; // need points
						mask |= (1 << sort.type); // mark occupy event
						evt.expectedFingerCount = fingerCount;
						if (evt.length === fingerCount) {
							sort.evt = evt;
							noticer.triggerWithEvent(evt as any);
						}
						break; // Exclusive event context
					}
				} // for sorts

				if (!mask) {
					evt.seal(); // No need for more events flow, seal event
				}
			} // isDefault

			evt.isDefault = true; // clear flag

			this._touches.set(touch.id, [gt, evt]); // accept point
			touchs.splice(i, 1); // remove point
		} while (i < touchs.length);
	}

	private _handleTouchStart(event: TouchEvent) {
		// add to global manager set for Delete zombie touchpoints caused by preventing event bubbles
		gGestureManagerSet.add(this);

		const timestamp = event.timestamp;
		const touchs = event.changedTouches.slice();
		for (const evt of this._eventsFlow) { // old event flow
			if (evt) {
				if (!touchs.length)
					return;
				this._dispatchStart(evt, touchs, timestamp, false);
			}
		}

		// New event flow
		while (touchs.length) {
			const id = this.getNewEventId();
			const evt = new GestureEvent(this._view, id, timestamp) as RemoveReadonly<GestureEvent>;
			this._dispatchStart(evt as GestureEventInl, touchs, timestamp, true);
			if (evt.length) {
				this._eventsFlow[evt.id] = evt as GestureEventInl; // add to event flow
			}
		}
	}

	private _handleTouchMove(event: TouchEvent) {
		const timestamp = event.timestamp;
		const changedEvents = new Set<GestureEventInl>(); // changed events

		for (const touch of event.changedTouches) { // find touch point
			const rec = this._touches.get(touch.id);
			if (rec) {
				const [pt, evt] = rec;
				changedEvents.add(evt); // mark event
				(pt as RemoveReadonly<typeof pt>).touch = touch; // update touch point
			}
		}

		for (const evt of changedEvents) {
			evt._update(timestamp, GestureStage.Change); // update event

			if (this._gesture) {
				this._gesture.triggerWithEvent(evt as any);
			}
			if (evt.isDefault) {
				for (const sort of this._sorts) {
					const noticer = this._noticers.get(sort.type)!;
					if (sort.evt === evt) {
						noticer.triggerWithEvent(evt as any);
						break; // Exclusive event context
					}
				}
			}

			evt.isDefault = true; // clear flag
		}
	}

	private _handleTouchEndOrCancel(event: TouchEvent, stage: GestureStage) {
		const timestamp = event.timestamp;
		const changedEvents = new Set<GestureEventInl>(); // changed events

		for (const touch of event.changedTouches) { // find touch point
			const rec = this._touches.get(touch.id);
			if (rec) {
				const [pt, evt] = rec;
				changedEvents.add(evt); // mark event
				evt.touchs.deleteOf(pt); // remove point from event
				this._touches.delete(touch.id); // remove point from manager
			}
		}

		for (const evt of changedEvents) {
			evt._update(timestamp, stage); // Update event state

			if (evt.length === 0) {
				this._eventsFlow[evt.id] = null; // clear event flow
			}

			if (this._gesture) {
				this._gesture.triggerWithEvent(evt as any);
			}
			if (evt.isDefault) {
				for (const sort of this._sorts) {
					const noticer = this._noticers.get(sort.type)!;
					if (sort.evt === evt) {
						noticer.triggerWithEvent(evt as any);
						sort.evt = void 0; // clear event context
						break; // Exclusive event context
					}
				}
				if (evt.length === 0 && stage === GestureStage.End) {
					if (this._swipe && evt.isSwipeTriggered()) {
						this._swipe.triggerWithEvent(evt as any);
					}
				}
			}

			evt.isDefault = true; // clear flag
		}
	}

	private _handleTouchEnd(event: TouchEvent) {
		this._handleTouchEndOrCancel(event, GestureStage.End);
	}

	private _handleTouchCancel(event: TouchEvent) {
		this._handleTouchEndOrCancel(event, GestureStage.Cancel);
	}

	addGesture(type: GestureType, noticer: EventNoticer<GestureEvent>) {
		if (type > GestureType.Gesture) {
			if (type === GestureType.SwipeGesture) {
				this._swipe = noticer;
			} else {
				this._noticers.set(type, noticer);
				this._sorts.push({type});
			}
		} else {
			this._gesture = noticer;
		}
	}
}

class _View extends NativeNotification<UIEvent> {
	@event readonly onClick: EventNoticer<ClickEvent>;
	@event readonly onBack: EventNoticer<ClickEvent>;
	@event readonly onKeyDown: EventNoticer<KeyEvent>;
	@event readonly onKeyPress: EventNoticer<KeyEvent>;
	@event readonly onKeyUp: EventNoticer<KeyEvent>;
	@event readonly onKeyEnter: EventNoticer<KeyEvent>;
	@event readonly onTouchStart: EventNoticer<TouchEvent>;
	@event readonly onTouchMove: EventNoticer<TouchEvent>;
	@event readonly onTouchEnd: EventNoticer<TouchEvent>;
	@event readonly onTouchCancel: EventNoticer<TouchEvent>;
	@event readonly onMouseOver: EventNoticer<MouseEvent>;
	@event readonly onMouseOut: EventNoticer<MouseEvent>;
	@event readonly onMouseLeave: EventNoticer<MouseEvent>;
	@event readonly onMouseEnter: EventNoticer<MouseEvent>;
	@event readonly onMouseMove: EventNoticer<MouseEvent>;
	@event readonly onMouseDown: EventNoticer<MouseEvent>;
	@event readonly onMouseUp: EventNoticer<MouseEvent>;
	@event readonly onMouseWheel: EventNoticer<MouseEvent>;
	@event readonly onFocus: EventNoticer<UIEvent>;
	@event readonly onBlur: EventNoticer<UIEvent>;
	@event readonly onHighlighted: EventNoticer<HighlightedEvent>;
	@event readonly onActionKeyframe: EventNoticer<ActionEvent>;
	@event readonly onActionLoop: EventNoticer<ActionEvent>;
	@event readonly onGesture: EventNoticer<GestureEvent>; // base gesture events
	@event readonly onPanGesture: EventNoticer<GestureEvent>; // 1 finger gesture
	@event readonly onSwipeGesture: EventNoticer<GestureEvent>; // 1 finger gesture
	@event readonly onPinchGesture: EventNoticer<GestureEvent>; // 2 finger gesture
	@event readonly onRotateGesture: EventNoticer<GestureEvent>; // 2 finger gesture
	@event readonly onThreeFingerGesture: EventNoticer<GestureEvent>; // 3 finger gesture
	@event readonly onFourFingerGesture: EventNoticer<GestureEvent>; // 4 finger gesture

	private _gestureManager?: GestureManager; // gesture manager

	getNoticer(name: string): EventNoticer<UIEvent> {
		const onName = '_on' + name;
		const noticer = (this as any)[onName] as EventNoticer<UIEvent>;
		if (!noticer) {
			if (name in GestureType) {
				if (!this._gestureManager)
					this._gestureManager = new GestureManager(this as any);
				const noticer = new EventNoticer<GestureEvent>(name, this as unknown as View);
				(this as any)[onName] = noticer;
				this._gestureManager.addGesture(GestureType[name as keyof typeof GestureType], noticer);
				return noticer;
			} else {
				return NN_getNoticer.call(this, name);
			}
		}
		return noticer;
	}

	readonly childDoms: (DOM|undefined)[]; // jsx children dom
	readonly ref: string;
	get metaView() { return this }
	get style() { return this as StyleSheets }
	set style(value) { Object.assign(this, value) }
	get class() { return [] }
	set class(value: string[]) {
		(this as unknown as View).cssclass.set(value);
	}

	get action() { // get action object
		return (this as any).action_ as Action | null;
	}

	set action(value) { // set action
		if (value)
			(this as any).action_ = createAction((this as unknown as View).window, value);
		else
			(this as any).action_ = null;
	}

	set key(val: any) { /* ignore */ }

	show() {
		(this as unknown as View).visible = true;
	}

	hide() {
		(this as unknown as View).visible = false;
	}

	hashCode() {
		return (this as unknown as View).viewType + 18766898;
	}

	appendTo(parent: View) {
		parent.append(this as unknown as View);
		return this;
	}

	afterTo(prev: View) {
		prev.after(this as unknown as View);
		return this;
	}

	destroy(owner: ViewController): void {
		for (let dom of this.childDoms) {
			if (dom)
				dom.destroy(owner);
		}
		(this as any).childDoms = []; // clear child doms
		let ref = this.ref;
		if (ref) {
			if (owner.refs[ref] === this as unknown as View) {
				delete (owner.refs as Dict<DOM>)[ref];
			}
		}
		(this as unknown as View).remove(); // remove from parent view
	}

	transition(to: KeyframeIn, from?: KeyframeIn) { // transition animate
		return action.transition(this as unknown as View, to, from);
	}

	toStringStyled(indent?: number) {
		let _rv = [] as string[];
		_rv.push('{');
		let kv = Object.entries(this);
		let _indent = (indent || 0) + 2;
		let push_indent = ()=>_rv.push(new Array(_indent + 1).join(' '));

		for (let i = 0; i < kv.length; i++) {
			let [k,v] = kv[i];
			_rv.push(i ? ',\n': '\n'); push_indent();
			_rv.push(k);
			_rv.push(':'); _rv.push(' ');

			if (typeof v == 'object') {
				if ('toStringStyled' in v) {
					_rv.push(v['toStringStyled'](_indent));
				} else if (Array.isArray(v)) {
					_rv.push('[Array]');
				} else {
					_rv.push('[Object]');
				}
			} else {
				_rv.push(v + '');
			}
		}
		_indent -= 2;
		_rv.push('\n'); push_indent();
		_rv.push('}');

		return _rv.join('');
	}

	toString() {
		return '[object view]';
	}
}

class _Image {
	@event readonly onLoad: EventNoticer<UIEvent>;
	@event readonly onError: EventNoticer<UIEvent>;
}

class _Agent {
	@event readonly onReachWaypoint: EventNoticer<ReachWaypointEvent>;
	@event readonly onDiscoveryAgent: EventNoticer<DiscoveryAgentEvent>;
	@event readonly onAgentMovement: EventNoticer<AgentMovementEvent>;
	@event readonly onAgentHeadingChange: EventNoticer<AgentStateEvent>;
}
	
class _Sprite {
	@event readonly onLoad: EventNoticer<UIEvent>;
	@event readonly onError: EventNoticer<UIEvent>;
}

class _Spine {
	@event readonly onSpineStart: EventNoticer<SpineEvent>;
	@event readonly onSpineInterrupt: EventNoticer<SpineEvent>;
	@event readonly onSpineEnd: EventNoticer<SpineEvent>;
	@event readonly onSpineDispose: EventNoticer<SpineEvent>;
	@event readonly onSpineComplete: EventNoticer<SpineEvent>;
	@event readonly onSpineEvent: EventNoticer<SpineExtEvent>;
}

class _Video {
	@event readonly onStop: EventNoticer<UIEvent>;
	@event readonly onBuffering: EventNoticer<UIEvent>;
}

class _Input {
	@event readonly onChange: EventNoticer<UIEvent>;
}

class _Textarea {
	@event readonly onScroll: EventNoticer<UIEvent>;
}

class _Scroll {
	@event readonly onScroll: EventNoticer<UIEvent>;
}

_ui.View.isViewController = false;
_ui.View.prototype.ref = '';
_ui.View.prototype.owner = null;
_ui.View.prototype.childDoms = [];
util.extendClass(_ui.View, _View);
util.extendClass(_ui.Scroll, _Scroll);
util.extendClass(_ui.Image, _Image);
util.extendClass(_ui.Agent, _Agent);
util.extendClass(_ui.Spine, _Spine);
util.extendClass(_ui.Sprite, _Sprite);
util.extendClass(_ui.Video, _Video);
util.extendClass(_ui.Input, _Input);
util.extendClass(_ui.Textarea, _Textarea);