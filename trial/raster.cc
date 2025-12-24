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

#include "./raster.h"
#include <math.h>

namespace qk {

	XLineScaner::XLineScaner(const Path& path, Rect clipRect, float scale, bool is_convex_polygon)
		: _activeEdges{0, 0, Int32::limit_min, 0, 0},
		_start_y(Int32::limit_max), _end_y(Int32::limit_min), _is_convex_polygon(is_convex_polygon)
	{
		scale *= 65536;

		Array<iVec2> edges;
		Path path2(path);
		path2.scale(Vec2(scale));

		//int i = 0;
		for (auto edge: path2.getEdgeLines()) {
			edges.push(iVec2(edge.x(), edge.y()));
			//if (i++ % 2 == 0)
			//Qk_DEBUG("Edge, %f, %f", edge.x() / 65536, edge.y() / 65536);
		}

		// clip paths
		if (clipRect.size.x() > 0 && clipRect.size.y() > 0) {
			if (scale != 1.0) {
				clipRect.origin *= Vec2(scale);
				clipRect.size *= Vec2(scale);
			}
			clip(edges, clipRect);
		}
		
		for (int i = 0; i < edges.length(); i+=2) {
			iVec2 p1 = edges[i], p2 = edges[i + 1];
			int y1 = p1.y() >> 16, y2 = p2.y() >> 16;
			//int y1 = roundf(p1.y() / 65536.0f), y2 = roundf(p2.y() / 65536.0f);
			int d = p1.y() - p2.y();
			if (y1 > y2) {
				int64_t incr = (int64_t(p1.x() - p2.x()) << 16) / (d);
				int32_t fix = 0x8000 - (p2.y() & 0xffff);
				_edges.push({ y2, y1, p2.x() + int32_t((fix * incr) >> 16), int(incr), nullptr });
			} else if (y1 < y2) {
				int64_t incr = (int64_t(p2.x() - p1.x()) << 16) / (-d);
				int32_t fix = 0x8000 - (p1.y() & 0xffff);
				_edges.push({ y1, y2, p1.x() + int32_t((fix * incr) >> 16), int(incr), nullptr });
			} else {
				continue; // discard
			}
			int minY = _edges.back().min_y;
			int maxY = _edges.back().max_y;
			if (minY < _start_y) {
				_start_y = minY;
			}
			if (maxY > _end_y) {
				_end_y = maxY;
			}
		}
		
		_newEdges.extend(_end_y - _start_y);
		memset(_newEdges.val(), 0, _newEdges.size());

		// make new edges and edges
		for (Edge& newEdge: _edges) {
			int y = newEdge.min_y - _start_y;

			Edge* prev = nullptr;
			Edge* edge = _newEdges[y];
			
			if (_is_convex_polygon) {
				if (y) {
					newEdge.x += newEdge.incr_x;
				}
			}

			// sort new edges
			do {
				if ( !edge || newEdge.x < edge->x || (newEdge.x == edge->x && newEdge.incr_x < edge->incr_x) ) {
					if (prev) {
						prev->next = &newEdge;
					} else {
						_newEdges[y] = &newEdge;
					}
					newEdge.next = edge;
					break;
				}
				prev = edge;
				edge = edge->next;
			} while(1);
		}

		_firstLineEdges = _newEdges[0];

		_newEdges[0] = nullptr;
	}

	void XLineScaner::scan(ScanCb cb, void* ctx) {
		if (_is_convex_polygon) {
			scan_convex_polygon(cb, ctx);
		} else {
			scan_polygon(cb, ctx);
		}
	}

	void XLineScaner::scan_polygon(ScanCb cb, void* ctx) {
		int i = 0;
		int y = _start_y;
		int e = _end_y;// + 1;
		/*
		auto getLeft = [](Edge* prev, Edge* left, int32_t y) {
			if (prev->x == left->x) {
				if ((prev->max_y > y && y > left->min_y) || (prev->min_y < y && y < left->max_y)) { // 1 points
					return left->next;
				} // else 0、2 points
			}
			return left;
		};*/

		_activeEdges.next = _firstLineEdges; // first line edges

		while (y < e) {
			Edge *prev, *left, *right;
			prev = &_activeEdges;

			//while ((left = getLeft(right, prev->next, y)) && (right = left->next)) {
			while ((left = prev->next) && (right = left->next)) {
				//if (y == 319) {
				//	Qk_DEBUG("%d", y);
				//}

				cb(left->x/* >> 16*/, right->x/* >> 16*/, y, ctx);

				// check delete active edges
				if (y < left->max_y) {
					left->x += left->incr_x;
					if (y < right->max_y) {
						right->x += right->incr_x;
						prev = right;
					} else { // delete right
						left->next = right->next;
						prev = left;
						if (!prev)
							break;
					}
				} else if (y < right->max_y) { // delete left
					right->x += right->incr_x;
					prev->next = right;
					prev = right;
				} else { // delete left/right
					prev->next = right->next;
					prev = prev->next;
					if (!prev)
						break;
				}
			}

			check_new_edges(i); // end check new edge

			y++, i++;
		}
	}

	void XLineScaner::scan_convex_polygon(ScanCb cb, void* ctx) {
		int i = 0;
		int y = _start_y;
		int e = _end_y;// + 1;

		_activeEdges.next = _firstLineEdges; // first line edges

		while (y < e) {
			Edge *left = _activeEdges.next;
			Qk_ASSERT(left, "left Edge cannot be empty");
			Edge *right = left->next;
			Qk_ASSERT(right, "right Edge cannot be empty");

			//if (left->x > right->x) {
			//	cb(right->x, left->x, y, ctx);
			//} else {
				cb(left->x, right->x, y, ctx);
			//}

			// check delete active edges
			if (y < left->max_y) {
				left->x += left->incr_x;
				if (y < right->max_y) {
					right->x += right->incr_x;
				} else { // delete right
					left->next = right->next;
				}
			} else if (y < right->max_y) { // delete left
				right->x += right->incr_x;
				_activeEdges.next = right;
			} else { // delete left/right
				_activeEdges.next = nullptr;
			}

			check_new_edges(i); // end check new edge

			y++, i++;
		}
	}

	void XLineScaner::check_new_edges(int y) {
		Edge *newEdge = _newEdges[y];
		if (!newEdge) return;

		// check new edges
		Edge *prev = &_activeEdges;
		Edge *edge = prev->next; // edge
		// Qk_DEBUG("%d", newEdge->x);
		if (!edge) {
			prev->next = newEdge; // end
			//newEdge->x += newEdge->incr_x; // incr
		} else {
			do { // sort
				start:
				if (newEdge->x < edge->x || (newEdge->x == edge->x && newEdge->incr_x < edge->incr_x)) {
					Edge* tmp = newEdge->next;
					prev->next = newEdge;
					newEdge->next = edge;
					newEdge = tmp;
				} else {
					prev = edge;
					edge = edge->next;
					if (edge) {
						goto start;
					} else {
						prev->next = newEdge; // end
						break;
					}
				}
			} while(newEdge);
		}
		// end check new edge
	}

	void XLineScaner::clip(Array<iVec2>& edges, Rect rect) {
		iVec2 clipOrigin = iVec2(rect.origin.x(), rect.origin.y());
		iVec2 clipEnd = clipOrigin + iVec2(rect.size.x(), rect.size.y());
		// TODO ...
	}

}