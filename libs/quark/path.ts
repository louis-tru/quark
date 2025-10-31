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

import {Vec2,Rect,Vec3,Mat,Range,BorderRadius} from './types';

const _path = __binding__('_path');
Object.assign(exports, _path);

/**
 * Path verb types
*/
export enum PathVerb {
	kMove,  //!< move
	kLine,  //!< straight line
	kQuad,  //!< quadratic bezier
	kCubic, //!< cubic bezier
	kClose, //!< close
}

/**
 * Line cap style
*/
export enum Cap {   //!< point style
	kButt,            //!< no stroke extension
	kRound,           //!< adds circle
	kSquare,          //!< adds square
}

/** Line join style */
export enum Join {      //!< stroke style
	kMiter,          //!< extends to miter limit
	kRound,          //!< adds circle
	kBevel,          //!< connects outside edges
}

/**
 * Create an oval (ellipse) path.
 * @param rect The bounding rectangle of the oval.
 * @param ccw Whether to draw counter-clockwise (CCW).
 * @returns A new path representing the oval.
 */
export declare function oval(rect: Rect, ccw: boolean): Path;

/**
 * Create an arc path.
 * @param rect The bounding rectangle of the arc.
 * @param startAngle Start angle in radians.
 * @param sweepAngle Sweep angle in radians (positive = clockwise, negative = counter-clockwise).
 * @param useCenter Whether to connect the center point to form a pie shape.
 * @param close Whether to close the path. Default is false.
 * @returns A new path representing the arc.
 */
export declare function arc(rect: Rect, startAngle: number, sweepAngle: number, useCenter: boolean, close?: boolean): Path;

/**
 * Create a rectangular path.
 * @param rect The rectangle bounds.
 * @param ccw Whether to draw counter-clockwise (default: false).
 * @returns A new path representing the rectangle.
 */
export declare function rect(rect: Rect, ccw?: boolean): Path;

/**
 * Create a circular path.
 * @param center Center of the circle.
 * @param radius Radius of the circle.
 * @param ccw Whether to draw counter-clockwise.
 * @returns A new path representing the circle.
 */
export declare function circle(center: Vec2, radius: number, ccw?: boolean): Path;

/**
 * Create a rounded rectangle path.
 * @param rect The rectangle bounds.
 * @param radius Corner radius values (can define per-corner radii).
 * @returns A new path representing the rounded rectangle.
 */
export declare function rrect(rect: Rect, radius: BorderRadius): Path;

/**
 * Create an outline path between two rounded rectangles (outer and inner).
 * @param outside Outer rectangle.
 * @param inside Inner rectangle.
 * @param radius Corner radius values.
 * @returns A new path representing the rounded rectangle outline.
 */
export declare function rrectOutline(outside: Rect, inside: Rect, radius: BorderRadius): Path;

/**
 * Compute the bounding region from a set of points.
 * Does NOT check if the matrix is an identity matrix.
 * @param pts The point array.
 * @param ptsLen Number of points.
 * @param matrix Optional transform matrix.
 * @returns The computed region bounds.
 */
export declare function getBoundsFromPoints(pts: Vec2[], ptsLen: number, matrix?: Mat): Range;

/**
 * Represents a geometric path consisting of lines, curves, and shapes.
 */
export declare class Path {
	/** Number of points in this path. */
	readonly ptsLen: Uint;

	/** Number of verbs (path commands) in this path. */
	readonly verbsLen: Uint;

	/** Whether the path is normalized (curves converted to line segments). */
	readonly isNormalized: boolean;

	/** Whether the path is sealed (cannot be modified). */
	readonly isSealed: boolean;

	/**
	 * Construct a new path.
	 * @param move Optional initial move-to point.
	 */
	constructor(move?: Vec2);

	/** Move the current point without drawing a line. */
	moveTo(to: Vec2): void;

	/** Draw a straight line from the current point to the specified point. */
	lineTo(to: Vec2): void;

	/** Draw a quadratic Bézier curve. */
	quadTo(control: Vec2, to: Vec2): void;

	/** Draw a cubic Bézier curve. */
	cubicTo(control1: Vec2, control2: Vec2, to: Vec2): void;

	/** Add an oval (ellipse) to the path. */
	ovalTo(rect: Rect, ccw?: boolean): void;

	/** Add a rectangle to the path. */
	rectTo(rect: Rect, ccw?: boolean): void;

	/**
	 * Add an arc defined by a bounding rectangle.
	 * @param rect Bounding rectangle.
	 * @param startAngle Start angle in radians.
	 * @param sweepAngle Sweep angle in radians.
	 * @param useCenter Whether to connect to the center point.
	 */
	arcTo(rect: Rect, startAngle: number, sweepAngle: number, useCenter: boolean): void;

	/**
	 * Add an arc defined by center and radius.
	 * @param center Center of the arc.
	 * @param radius Radius or ellipse radii (Vec2 for elliptical arcs).
	 * @param startAngle Start angle in radians.
	 * @param sweepAngle Sweep angle in radians.
	 * @param useCenter Whether to connect to the center point.
	 */
	arc(center: Vec2, radius: Vec2, startAngle: number, sweepAngle: number, useCenter: boolean): void;

	/** Close the current contour (connect last point to first point). */
	close(): void;

	/** Append another path to the end of this one. */
	concat(path: Path): void;

	/** Get the point at the specified index. */
	ptsAt(index: Uint): Vec2;

	/** Get the verb (command) at the specified index. */
	verbsAt(index: Uint): PathVerb;

	/**
	 * Convert curves into line segments and return all edge points.
	 * @param epsilon Approximation tolerance (smaller = higher precision, default = 1.0).
	 * @returns An array of Vec2 representing the edges.
	 */
	getEdgeLines(epsilon?: number): Vec2[];

	/**
	 * Triangulate the filled region of the path.
	 * @param epsilon Approximation tolerance (default = 1.0).
	 * @returns An array of Vec3 vertices (each triple = one triangle).
	 */
	getTriangles(epsilon?: number): Vec3[];

	/**
	 * Generate anti-aliased fuzzy stroke triangles.
	 * @param width Stroke width.
	 * @param epsilon Approximation tolerance (default = 1.0).
	 * @returns Array of Vec3 triangles for fuzzy stroke rendering.
	 */
	getAAFuzzStrokeTriangle(width: number, epsilon?: number): Vec3[];

	/**
	 * Convert the path into a dashed path.
	 * @param stage An array of segment lengths (e.g. [dash, gap, dash, gap, ...]).
	 *              Each value represents a **distance along the path** in the same coordinate units.
	 *              Even indices (0, 2, 4, …) are drawn segments; odd indices are skipped segments.
	 * @param offset Optional phase offset along the path (default = 0).
	 * @returns A new path containing the dashed version.
	 */
	dashPath(stage: number[], offset?: number): Path;

	/**
	 * Generate a stroke path from the current shape.
	 * @param width Stroke width.
	 * @param cap Line cap style (butt, round, square).
	 * @param join Line join style (miter, round, bevel).
	 * @param miterLimit Maximum miter ratio.
	 * @returns A new path representing the stroked outline.
	 */
	strokePath(width: number, cap?: Cap, join?: Join, miterLimit?: number): Path;

	/**
	 * Normalize the path (convert curves to linear segments).
	 * @param epsilon Approximation tolerance (default = 1.0).
	 * @returns The normalized path (this).
	 */
	normalizedPath(epsilon?: number): this;

	/**
	 * Apply a transformation matrix to the path.
	 * @param matrix Transformation matrix.
	 */
	transform(matrix: Mat): void;

	/**
	 * Scale the path by a given factor.
	 * @param scale Scale vector (x, y).
	 */
	scale(scale: Vec2): void;

	/**
	 * @method seal() seal path, after sealed, path data can not be modified
	*/
	seal(): void;

	/**
	 * Get the path's bounding box.
	 * @param matrix Optional transform matrix (fast path if identity).
	 * @returns The bounding region.
	 */
	getBounds(matrix?: Mat): Range;

	/**
	 * Create a copy of this path.
	 * @returns A new Path instance that is a copy of this path.
	 */
	copy(): Path;

	/**
	 * Generate a hash code for the path.
	 * @returns The hash code as an integer.
	*/
	hashCode(): Int;
}
