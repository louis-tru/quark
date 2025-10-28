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

#include "./entity.h"
#include "../window.h"
#include "../painter.h"

namespace qk {
	#define _async_call preRender().async_call

	typedef Entity::Bounds Bounds;

	Vec2 free_typesetting(View* view, View::Container &container);

	////////////////////////////////////////////////////////////////

	Entity::Entity(): View(), MorphView(this), _bounds{kDefault, 0.0f}, _ptsBounds(nullptr)
	{
		_bounds.pts = nullptr;
		_visible_area = true; // Always visible
		set_receive(false);
		// sizeof(Entity); // call to avoid compile warning
	}

	Entity::~Entity() {
		Release(_bounds.pts.exchange(nullptr));
	}

	const Bounds& Entity::bounds() const {
		return _bounds;
	}

	void Entity::set_bounds(const Bounds& bounds) {
		// release old pts in reader thread safe way
		// or add to delay task queue
		auto oldPts = _bounds.pts.load();
		_bounds.type = bounds.type;
		_bounds.radius = bounds.radius;
		_bounds.pts = bounds.pts.load();

		_async_call([](auto self, auto arg) {
			Release(arg.arg); // safe release old pts
			self->mark(kTransform, true); // mark to update
		}, this, oldPts);
	}

	Vec2 Entity::layout_offset_inside() {
		return -_origin_value;
	}

	bool Entity::overlap_test(Vec2 point) {
		if (_bounds.type == kDefault || _bounds.type == kCircle || _bounds.type == kPolygon) {
			Vec2 boxBounds[4];
			solve_box_Bounds(matrix(), boxBounds); // compute bounds for overlap test
			return test_overlap_from_convex_quadrilateral(boxBounds, point);
		}
		return false;
	}

	void Entity::solve_visible_area(const Mat &mat) {
		solve_bounds(); // compute bounds pts and circle
		_visible_area = true;
	}

	void Entity::solve_bounds() {
		Mat localMat(_translate, _scale, -_rotate_z, _skew);
		if (_bounds.type == kDefault || _bounds.type == kCircle) {
			if (_ptsBounds)
				_ptsBounds->clear(); // clear ptsBounds if have
			auto size = client_size();
			auto center = size * 0.5f - _origin_value; // use center as circle center
			auto radius = _bounds.type == kDefault ? size.length() * 0.5f: _bounds.radius;
			auto scale = std::max(std::abs(_scale.x()), std::abs(_scale.y()));
			_circleBounds = { localMat * center, radius * scale };
		}
		else if (_bounds.type == kPolygon || _bounds.type == kLineSegment) {
			auto origin = -solve_origin_value({}); // compute origin offset
			auto pts = _bounds.pts.load();
			if (!_ptsBounds) {
				_ptsBounds = new Array<Vec2>();
			}
			_ptsBounds->reset(pts->ptsLen());
			for (int i = 0; i < _ptsBounds->length(); i++) {
				_ptsBounds->at(i) = localMat * (pts->atPt(i) + origin); // transform pts
			}
			_circleBounds.center = localMat * origin; // use origin as circle center
		}
	}

	cArray<Vec2>& Entity::ptsOfBounds() {
		if (!_ptsBounds) {
			_ptsBounds = new Array<Vec2>();
		}
		if (_ptsBounds->length() == 0) {
			if (_bounds.type == kDefault || _bounds.type == kCircle) {
				*_ptsBounds.get() = circle_to_octagon(_circleBounds);
			}
		}
		return *_ptsBounds.get();
	}

	void Entity::solve_marks(const Mat &mat, View *parent, uint32_t mark) {
		if (mark & (kTransform | kVisible_Region)) { // Update transform matrix
			unmark(kTransform | kVisible_Region); // Unmark
			_origin_value = solve_origin_value(client_size() * 0.5f); // Check transform_origin change
			auto v = parent->layout_offset_inside() + _translate;
			_matrix = Mat(mat).set_translate(parent->position()) * Mat(v, _scale, -_rotate_z, _skew);
			_position = Vec2(_matrix[2],_matrix[5]); // the origin world coords
			solve_visible_area(_matrix);
		}
	}

	// Compute origin value from parameter
	// The origin value is the final value by computing the origin.
	Vec2 Entity::solve_origin_value(Vec2 from) {
		switch (_origin_x.kind) {
			default:
			case BoxOriginKind::Auto: break; // use from.x
			case BoxOriginKind::Value: from[0] += _origin_x.value; break;
			case BoxOriginKind::Ratio: from[0] += _origin_x.value * client_size().x(); break;
		}
		switch (_origin_y.kind) {
			default:
			case BoxOriginKind::Auto: break;
			case BoxOriginKind::Value: from[1] += _origin_y.value; break;
			case BoxOriginKind::Ratio: from[1] += _origin_y.value * client_size().y(); break;
		}
		return from;
	}

	void Entity::trigger_listener_change(uint64_t name, int count, int change) {
		if ( change > 0 ) {
			auto it = UIEventNames.findFor(name);
			if (it != UIEventNames.end()) {
				if (it->second.category() >= kClick_UIEventCategory &&
					it->second.category() <= kHighlighted_UIEventCategory) {
					set_receive(true); // receive input events
				}
			}
		}
	}

	void Entity::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			Container c{
				client_size(), {}, {}, {}, kFixed_FloatState, kFixed_FloatState, false, false
			};
			free_typesetting(this, c);
		}
	}

	Vec2 Entity::client_size() {
		if (_bounds.type == kCircle) {
			float diameter = _bounds.radius * 2.0f;
			return Vec2(diameter, diameter);
		} else if (_bounds.type == kPolygon || _bounds.type == kLineSegment) {
			auto pts = _bounds.pts.load();
			auto aabb = region_aabb_from_polygon(pts->pts());
			return aabb.end - aabb.begin; // width, height
		} else {
			return {};
		}
	}

	Region Entity::client_region() {
		auto begin = -_origin_value;
		return { begin, begin + client_size(), _translate };
	}

	ViewType Entity::viewType() const {
		return kEntity_ViewType;
	}

	Entity* Entity::asEntity() {
		return this;
	}

	MorphView* Entity::asMorphView() {
		return this;
	}

	void Entity::debugDraw(Painter *render) {
		if (window()->debugMode()) {
			auto canvas = render->canvas();
			Paint paint;
			paint.fill.color = Color4f(1,0,0,0.2f); // red fill
			if (_bounds.type == kCircle) {
				// Draw the circle
				canvas->drawPath(Path::MakeCircle(_circleBounds.center, _circleBounds.radius), paint);
			} else if (_bounds.type == kPolygon || _bounds.type == kLineSegment) {
				Path path;
				for (auto v: ptsOfBounds()) {
					path.lineTo(v); // Draw line to each point
				}
				if (_bounds.type == kLineSegment) {
					paint.style = Paint::kStroke_Style;
					paint.stroke.color = paint.fill.color;
					paint.strokeWidth = _bounds.halfThickness * 2.0f;
					paint.join = Paint::kBevel_Join;
					canvas->drawPath(path, paint); // stroke path
				} else {
					path.close(); // close path for polygon
					canvas->drawPath(path, paint); // fill path
				}
			}
		}
	}

	void Entity::draw(Painter *render) {
		debugDraw(render);
		render->visitView(this);
	}

	//--------------------------------------------------------------

	ArrivePositionEvent::ArrivePositionEvent(Agent *origin, Vec2 position, Vec2 nextLocation, uint32_t waypoint_index)
		: UIEvent(origin), _position(position), _nextLocation(nextLocation), _waypointIndex(waypoint_index) {
	}
	DiscoveryAgentEvent::DiscoveryAgentEvent(
		Agent *origin, Agent* agent, Vec2 location, uint32_t id, int level, bool entering
	) : UIEvent(origin), _agent(agent), _location(location)
		, _agentId(id), _level(level), _entering(entering)
	{}
	void DiscoveryAgentEvent::release() {
		_agent = nullptr; // clear weak reference
		UIEvent::release();
	}

	FollowTargetEvent::FollowTargetEvent(Agent *origin, State state): UIEvent(origin), _state(state) {
	}

	// An entity with agent properties for navigation and avoidance.
	Agent::Agent()
		: Entity()
		, _active(false), _following(false), _target()
		, _velocity(), _velocityMax(100.0f)
		, _currentWaypoint(0)
		, _safetyBuffer(5.0f), _followDistanceRange(0.0f, 0.0f)
		, _discoveryDistancesSq(nullptr), _waypoints(nullptr), _followTarget(nullptr)
	{
		//sizeof(Agent); // call to avoid compile warning
	}

	Agent::~Agent() {
		Releasep(_discoveryDistancesSq);
		Releasep(_waypoints);
		_followTarget = nullptr;
	}

	Agent* Agent::asAgent() {
		return this;
	}

	void Agent::onActivate() {
		_discoverys_rt.clear(); // clear discovery agents set
		_following = false;
		_followTarget = nullptr; // clear follow target
	}

	void Agent::set_active(bool val) {
		_active = val;
		_async_call([](auto self, auto arg) { self->_active = arg.arg; }, this, val);
	}

	void Agent::set_velocityMax(float val) {
		_velocityMax = Qk_Max(val, 0.0f);
	}

	bool Agent::isWaypoints() const {
		return _waypoints;
	}

	void Agent::set_safetyBuffer(float val) {
		_safetyBuffer = Float32::clamp(val, 0.0f, 100.0f);
	}

	cArray<float>& Agent::discoveryDistancesSq() const {
		static cArray<float> Empty;
		return _discoveryDistancesSq ? *_discoveryDistancesSq : Empty;
	}

	void Agent::set_discoveryDistancesSq(cArray<float>& val) {
		preRender().safeReleasep(_discoveryDistancesSq); // release previous and set nullptr
		if (val.length()) {
			_discoveryDistancesSq.store(new Array<float>(val));
		}
	}

	void Agent::setDiscoveryDistances(cArray<float>& val) {
		// convert to squared distances
		set_discoveryDistancesSq(val.map<float>([](auto d, auto i) { return d * d; }));
	}

	void Agent::set_followDistanceRange(Vec2 val) {
		_followDistanceRange[0] = Qk_Max(val[0], 0.0f);
		_followDistanceRange[1] = Qk_Max(val[1], _followDistanceRange[0]);
	}

	Agent* Agent::followTarget() const {
		return _followTarget.load();
	}

	void Agent::set_followTarget(Agent* target) {
		if (target == this || _followTarget == target)
			return; // cannot follow self or same target
		if (target->parent() != parent())
			return; // must be in the same world
		_followTarget = target; // set follow target, weak reference
		_active = target; // set active if have target
		_async_call([](auto self, auto arg) {
			self->_following = false; // reset following state
			self->_active = arg.arg;
		}, this, target);
	}

	void Agent::moveTo(Vec2 target, bool immediately) {
		_active = true;
		_target = target;
		struct Arg { Vec2 target; bool immediately; };
		_async_call([](auto self, auto arg) {
			Sp<Arg> self_sp(arg.arg); // rtti delete arg
			if (arg.arg->immediately) {
				self->set_translate(arg.arg->target, true);
				self->solve_bounds(); // compute bounds pts and circle
			}
			self->_active = true; // set active again
			self->_target = arg.arg->target; // set target position again
		}, this, new Arg{target, immediately});
	}

	struct NearestPathPoint {
		Vec2 point;    // The nearest point on the path
		int segIndex;  // The index of the segment
		float dist2;   // The squared distance to the nearest point
	};

	// Find the nearest point on the path defined by waypoints to the current position.
	NearestPathPoint findNearestPointOnPath(cArray<Vec2>& waypoints, Vec2 pos) {
		NearestPathPoint result{{0,0}, -1, std::numeric_limits<float>::infinity()};
		if (waypoints.size() < 2)
			return result;
		for (int i = 0; i < waypoints.length() - 1; ++i) {
			Vec2 a = waypoints[i];
			Vec2 b = waypoints[i+1];
			Vec2 proj = closest_point_on_segment(pos, a, b);
			float d2 = (proj - pos).lengthSq();
			if (d2 < result.dist2) {
				result.dist2 = d2;
				result.segIndex = i;
				result.point = proj;
			}
		}
		return result;
	}

	void Agent::setWaypoints(Path* waypoints, bool immediately) {
		if (waypoints == _waypoints.load())
			return; // same waypoints, ignore
		Releasep(_waypoints); // release previous waypoints
		if (waypoints && waypoints->ptsLen() >= 2) {
			waypoints->normalizedPath(); // normalize path
			Qk_ASSERT(waypoints->isNormalized(), "waypoints normalize failed");
			waypoints->seal(); // seal path
			waypoints->retain();
			_waypoints = waypoints;
		}
	}

	/**
	 * set waypoints for agent with velocityMax, and move to closest waypoint
	 * @param waypoints {cArray<Vec2>*} waypoints for agent
	 */
	void Agent::setWaypoints(cArray<Vec2>& waypoints, bool immediately) {
		if (waypoints.length() < 2) return;
		Sp<Path> path = new Path();
		for (auto v: waypoints)
			path->lineTo(v);
		setWaypoints(path.get(), immediately);
	}

	void Agent::returnToWaypoints(bool immediately) {
		auto waypoints = _waypoints.load();
		if (!waypoints)
			return;

		// search closest waypoint
		auto nearest = findNearestPointOnPath(waypoints->pts(), translate());
		Qk_ASSERT(nearest.segIndex != -1);

		_active = true;
		_target = nearest.point; // set target to closest point
		_currentWaypoint = nearest.segIndex;

		struct Arg { int  segIndex; bool immediately; } arg { nearest.segIndex, immediately };

		_async_call([](auto self, auto arg) {
			auto waypoints = self->_waypoints.load();
			auto segIndex = arg.arg.segIndex; // closest waypoint index
			if (waypoints && segIndex < waypoints->ptsLen()) {
				if (arg.arg.immediately) {
					// move to closest waypoint immediately
					self->set_translate(waypoints->atPt(segIndex), true);
					self->solve_bounds(); // compute bounds pts and circle
				}
				// set props again, avoid thread competition
				self->_active = true; // set active again
				self->_target = waypoints->atPt(segIndex); // set target to closest point again
				self->_currentWaypoint = segIndex; // set current waypoint again
			}
		}, this, arg);
	}

	void Agent::stop() {
		set_active(false);
	}

}
