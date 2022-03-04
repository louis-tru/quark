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

#include "./scaner.h"

namespace flare {

	XLineScaner::XLineScaner(const PathLine& path, Rect clip, float scale)
		: _activeEdges{0, 0, Float::limit_min, 0, 0},
		_start_y(Int32::limit_max), _end_y(Int32::limit_min)
	{
		scale *= 65536;

		Array<Vec2i> edges;
		for (auto edge: PathLine(path).scale(Vec2(scale)).to_edge_line()) {
			edges.push(Vec2i(edge.x(), edge.y()));
		}

		// clip paths
		if (clip.size.x() > 0 && clip.size.y() > 0) {
			if (scale != 1.0) {
				clip.origin *= Vec2(scale);
				clip.size *= Vec2(scale);
			}
			Vec2i clipOrigin = Vec2i(clip.origin.x(), clip.origin.y());
			Vec2i clipEnd = clipOrigin + Vec2i(clip.size.x(), clip.size.y());
			// TODO ..
		}
		
		for (int i = 0; i < edges.length(); i+=2) {
			Vec2i p1 = edges[i], p2 = edges[i + 1];
			int y1 = p1.y() >> 16, y2 = p2.y() >> 16;
			if (y1 > y2) {
				_edges.push({ y2, y1, p2.x(), p1.x() - p2.x(), nullptr });
			} else if (y1 < y2) {
				_edges.push({ y1, y2, p1.x(), p2.x() - p1.x(), nullptr });
			}
			int y = _edges.back().min_y;
			int y2 = _edges.back().max_y;
			if (y < _start_y) {
				_start_y = y;
			}
			if (y2 > _end_y) {
				_end_y = y2;
			}
		}

		// make new edges and edges
		for (Edge& newEdge: _edges) {
			int y = e.min_y - _start_y;
			_newEdges.extend(y + 1);

			Edge* prev = nullptr;
			Edge* edge = _newEdges[y];

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
	}

	void XLineScaner::scan(void(*cb)(int32_t left, int32_t right, int32_t y)) {
		int i = 0;
		int y = _start_y;
		int e = _end_y + 1;

		auto getLeft = [](Edge* prev, Edge* left, int32_t y) {
			if (prev->x == left->x) {
				if ((prev->max_y > y && y > left->min_y) || (prev->min_y < y && y < left->max_y)) { // 1 points
					return left->next;
				} // else 0、2 points
			}
			return left;
		};

		while (y < e) {
			Edge *newEdge = _newEdges[i];
			Edge *prev, *left, *right;

			if (newEdge) {
				// check new edges
				prev = &_activeEdges;
				left = prev->next; // edge
				// F_DEBUG("%d", newEdge->x);

				if (!left) {
					prev->next = newEdge; // end
				} else {
					do { // sort
						start:
						if (newEdge->x < left->x || (newEdge.x == left->x && newEdge.incr_x < left->incr_x)) {
							Edge* tmp = newEdge->next;
							prev->next = newEdge;
							newEdge->next = left;
							newEdge = tmp;
						} else {
							prev = left;
							left = left->next;
							if (left) {
								goto start;
							} else {
								prev->next = newEdge; // end
								break;
							}
						}
					} while(newEdge);
				}
			}

			prev = right = &_activeEdges;

			while ((left = getLeft(right, prev->next, y)) && (right = left->next)) {

				cb(left->x >> 16, right->x >> 16, y);

				// check delete active edges
				if (y < left->max_y) {
					left->x += left->incr_x;
					if (y < right->max_y) {
						right->x += right->incr_x;
						prev = right;
					} else { // delete righ
						left->next = right->next;
						prev = left;
					}
				} else if (y < right->max_y) {
					right->x += right->incr_x;
					prev->next = right;
					prev = right;
				} else {
					prev->next = right->next;
					prev = prev->next;
				}
			}

			y++, i++;
		}
	}

}