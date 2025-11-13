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
	// Geometric epsilon for float comparisons
	constexpr float GEOM_EPS = 1e-6f;
	typedef Entity::Bounds Bounds;

	Vec2 free_typesetting(View* view, View::Container &container);
	void onUIEvent(cUIEventName& name, Agent* agent, UIEvent *evt);
	void onAgentMovement(Agent* agent, AgentMovementEvent::MovementState state) {
		onUIEvent(UIEvent_AgentMovement, agent, new AgentMovementEvent(agent, state));
	}

	////////////////////////////////////////////////////////////////

	Entity::Entity(): View(), MorphView(this), _bounds{kDefault, 0.0f}, _ptsBounds(nullptr)
		, _participate(true)
	{
		_bounds.pts = nullptr;
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
		auto path = bounds.pts.load();
		_bounds.type = bounds.type;
		_bounds.offset = bounds.offset;
		_bounds.radius = bounds.radius;
		_bounds.pts = path;
		Retain(path); // retain new pts
		
		if (!path) {
			if (bounds.type == kLineSegment || bounds.type == kPolygon) {
				_bounds.type = kDefault; // reset to default if pts is null
			}
		} else {
			path->seal(); // seal path, not allow modify
		}

		_async_call([](auto self, auto arg) {
			Release(arg.arg); // safe release old pts
			self->mark(kTransform, true); // mark to update
		}, this, oldPts);
	}

	bool Entity::participate() const {
		return _participate;
	}

	void Entity::set_participate(bool value) {
		_participate = value;
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
		_visible_area = true; // Always visible
	}

	void Entity::solve_bounds() {
		Mat localMat(_translate, _scale, -_rotate_z, _skew);
		if (_bounds.type == kDefault || _bounds.type == kCircle) {
			if (_ptsBounds)
				_ptsBounds->clear(); // clear ptsBounds if have
			auto size = client_size();
			auto center = size * 0.5f - _origin_value + _bounds.offset; // compute center offset
			auto radius = _bounds.type == kDefault ? size.length() * 0.5f: _bounds.radius;
			auto scale = std::max(std::abs(_scale.x()), std::abs(_scale.y()));
			_circleBounds = { localMat * center, radius * scale };
		}
		else if (_bounds.type == kPolygon || _bounds.type == kLineSegment) {
			auto center = _bounds.offset - compute_origin_value({}); // compute center offset
			auto pts = _bounds.pts.load();
			if (!_ptsBounds) {
				_ptsBounds = new Array<Vec2>();
			}
			if (pts) {
				_ptsBounds->reset(pts->ptsLen());
				for (int i = 0; i < _ptsBounds->length(); i++)
					_ptsBounds->at(i) = localMat * (pts->atPt(i) + center); // transform pts
				auto aabb = region_aabb_from_polygon(*_ptsBounds.get()); // compute aabb from pts
				_circleBounds.radius = (aabb.end - aabb.begin).length() * 0.5f;
			}
			_circleBounds.center = localMat * center; // compute center
		}
	}

	Array<Vec2>& Entity::ptsOfBounds() {
		if (!_ptsBounds) {
			_ptsBounds = new Array<Vec2>();
		}
		if (_ptsBounds->length() == 0) {
			if (_bounds.type == kDefault || _bounds.type == kCircle) {
				*_ptsBounds.get() = circle_to_octagon(_circleBounds);
			} else {
				_ptsBounds->push(Vec2()); // ensure not empty
				_ptsBounds->push(Vec2()); // ensure not empty
			}
		}
		return *_ptsBounds.get();
	}

	void Entity::solve_marks(const Mat &mat, View *parent, uint32_t mark) {
		if (mark & (kTransform | kVisible_Region)) { // Update transform matrix
			unmark(kTransform | kVisible_Region); // Unmark
			_origin_value = compute_origin_value(client_size() * 0.5f); // Check transform_origin change
			auto v = parent->layout_offset_inside() + _translate;
			_matrix = Mat(mat).set_translate(parent->position()) * Mat(v, _scale, -_rotate_z, _skew);
			_position = Vec2(_matrix[2],_matrix[5]); // the origin world coords
			solve_visible_area(_matrix);
		}
	}

	// Compute origin value from parameter
	// The origin value is the final value by computing the origin.
	Vec2 Entity::compute_origin_value(Vec2 from) {
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
			if (pts) {
				auto aabb = region_aabb_from_polygon(pts->pts());
				return aabb.end - aabb.begin; // width, height
			}
		}
		return {};
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

	void Entity::debugDraw(Painter *painter) {
		if (window()->debugMode()) {
			auto lastMatrix = painter->matrix();
			auto canvas = painter->canvas();
			Mat mat = matrix();
			mat.translate(-translate());
			painter->set_matrix(&mat); // set entity local matrix
			Paint paint;
			paint.antiAlias = false;
			paint.fill.color = Color4f(1,0,0,0.2f); // red fill
			if (_bounds.type == kDefault || _bounds.type == kCircle) {
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
					// paint.join = Paint::kBevel_Join;
					canvas->drawPath(path, paint); // stroke path
				} else {
					path.close(); // close path for polygon
					canvas->drawPath(path, paint); // fill path
				}
			}
			painter->set_matrix(lastMatrix); // restore previous matrix
		}
	}

	void Entity::draw(Painter *painter) {
		debugDraw(painter);
		painter->visitView(this, &_matrix);
	}

	bool Entity::test_entity_vs_entity(Entity *o, MTV *outMTV, Vec2 *outMtvVec, bool computeMTV) {
		Qk_ASSERT(_bounds.type != Entity::kLineSegment); // line segment agent does not move
		bool isPoly = _bounds.type == Entity::kPolygon;
		auto &pts = ptsOfBounds();
		auto &circ = _circleBounds;
		bool collided = false;
		if (o->_bounds.type == Entity::kDefault || o->_bounds.type == Entity::kCircle) {
			// circle vs circle
			collided = isPoly ?
				test_polygon_vs_polygon(pts, o->ptsOfBounds(), outMTV, computeMTV):
				test_circle_vs_circle(circ, o->_circleBounds, outMTV, computeMTV);
		}
 		else if (o->_bounds.type == Entity::kPolygon) {
			// polygon check (approx circle->poly using SAT code) - optionally enable
			collided = test_polygon_vs_polygon(pts, o->ptsOfBounds(), outMTV, computeMTV);
		}
		else { // line segment
			MTV mtv;
			outMTV->overlap = std::numeric_limits<float>::infinity();
			// collision prevention: do a quick circle vs walls check for move
			auto halfWidth = o->_bounds.halfThickness;
			auto &ptsB = o->ptsOfBounds();
			auto a = ptsB.at(0); // get last point
			for (int i = 1; i < ptsB.length(); ++i) {
				auto b = ptsB.at(i);
				if (isPoly ? 
						test_polygon_vs_line_segment(pts, {a, b, halfWidth}, &mtv, computeMTV): 
						test_circle_vs_line_segment(circ, {a, b, halfWidth}, &mtv, computeMTV)) {
					collided = true;
					*outMtvVec += mtv.axis * mtv.overlap; // accumulate MTV
				}
				if (outMTV->overlap > mtv.overlap)
					*outMTV = mtv; // select minimum translation vector
				a = b;
			}
			return collided;
		}
		if (collided)
			*outMtvVec += outMTV->axis * outMTV->overlap; // convert to vector
		return collided;
	}

	//--------------------------------------------------------------

	AgentStateEvent::AgentStateEvent(Agent *origin)
		: UIEvent(origin)
		, _velocity(origin->velocity()), _heading(origin->heading())
		, _target(origin->target()), _moving(origin->moving()) {
	}
	ReachWaypointEvent::ReachWaypointEvent(Agent *origin, Vec2 toNext, uint32_t waypoint_index)
		: AgentStateEvent(origin), _toNext(toNext), _waypointIndex(waypoint_index) {
	}
	AgentMovementEvent::AgentMovementEvent(Agent *origin, MovementState state)
		: AgentStateEvent(origin), _movementState(state) {
	}
	DiscoveryAgentEvent::DiscoveryAgentEvent(
		Agent *origin, Agent* agent, Vec2 location, uint32_t id, uint32_t level, bool entering
	) : AgentStateEvent(origin), _agent(agent), _location(location)
		, _agentId(id), _level(level), _entering(entering)
	{}
	void DiscoveryAgentEvent::release() {
		_agent = nullptr; // clear weak reference
		UIEvent::release();
	}

	inline float randf(float min, float max) {
		return min + (rand() / static_cast<float>(RAND_MAX)) * (max - min);
	}

	// An entity with agent properties for navigation and avoidance.
	Agent::Agent()
		: Entity()
		, _active(false), _moving(false)
		, _floatingStation(false)
		, _target()
		, _velocitySteer(), _velocity(), _heading(), _velocityMax(100.0f)
		, _currentWaypoint(0)
		, _safetyBuffer(5.0f)
		, _avoidanceFactor(0.5f)
		, _avoidanceVelocityFactor(1.0f)
		, _followMinDistance(0.0f)
		, _followMaxDistance(0.0f)
		, _discoveryDistances(nullptr), _waypoints(nullptr), _followTarget(nullptr)
	{
		// sizeof(Agent); // call to avoid compile warning
	}

	Agent::~Agent() {
		Releasep(_discoveryDistances);
		Releasep(_waypoints);
		_followTarget = nullptr;
	}

	Agent* Agent::asAgent() {
		return this;
	}

	void Agent::onActivate() {
		if (level() == 0) { // remove or invisible
			_discoverys_rt.clear(); // clear discovery agents set
			_moving = false;
			_followTarget = nullptr; // clear follow target
		}
	}

	void Agent::set_avoidanceFactor(float value) {
		_avoidanceFactor = Float32::clamp(value, 0.0f, 10.0f);
	}

	void Agent::set_avoidanceVelocityFactor(float value) {
		_avoidanceVelocityFactor = Float32::clamp(value, 0.0f, 3.0f);
	}

	void Agent::set_active(bool val) {
		_active = val;
	}

	void Agent::set_velocityMax(float val) {
		_velocityMax = Qk_Max(val, 0.0f);
	}

	void Agent::set_floatingStation(bool val) {
		_floatingStation = val;
	}

	Path* Agent::waypoints() const {
		return _waypoints;
	}

	void Agent::set_waypoints(Path* waypoints) {
		setWaypoints(waypoints, false);
	}

	void Agent::set_safetyBuffer(float val) {
		_safetyBuffer = Float32::clamp(val, 0.0f, 100.0f);
	}

	cArray<float>& Agent::discoveryDistances() const {
		static cArray<float> Empty;
		return _discoveryDistances ? *_discoveryDistances : Empty;
	}

	void Agent::set_discoveryDistances(cArray<float>& val) {
		preRender().safeReleasep(_discoveryDistances); // release previous and set nullptr
		if (val.length()) {
			_discoveryDistances.store(new Array<float>(val));
		}
	}

	void Agent::set_followMinDistance(float val) {
		_followMinDistance = std::max(val, 0.0f);
		_followMaxDistance = std::max(_followMaxDistance, _followMinDistance);
	}

	void Agent::set_followMaxDistance(float val) {
		_followMaxDistance = std::max(val, _followMinDistance);
	}

	Agent* Agent::followTarget() const {
		return _followTarget.load();
	}

	void Agent::set_followTarget(Agent* other) {
		if (other == this)
			return; // cannot follow self or same target
		if (other && other->parent() != parent())
			return; // must be in the same world
		if (other) {
			_followTarget = other; // set follow target, weak reference
		} else {
			moveTo(_target, false); // move to target position
		}
	}

	void Agent::moveTo(Vec2 target, bool immediately) {
		_target = target;
		_followTarget = nullptr; // clear follow target
		struct Arg { Vec2 target; bool immediately; };
		_async_call([](auto self, auto arg) {
			Sp<Arg> self_sp(arg.arg); // rtti delete arg
			if (arg.arg->immediately) {
				self->set_translate(arg.arg->target, true);
			}
			self->_target = arg.arg->target; // set target position again
			if (!self->_moving) {
				self->_moving = true;
				onAgentMovement(self, AgentMovementEvent::Started);
			}
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

	void Agent::setWaypoints(Path* waypoints, bool immediately) {
		if (waypoints == _waypoints.load()) {
			returnToWaypoints(immediately);
			return; // same waypoints, ignore
		}
		Releasep(_waypoints); // release previous waypoints
		if (waypoints && waypoints->ptsLen() >= 2) {
			waypoints->normalizedPath(); // normalize path
			Qk_ASSERT(waypoints->isNormalized(), "waypoints normalize failed");
			waypoints->seal(); // seal path
			waypoints->retain();
			_waypoints = waypoints;
			returnToWaypoints(immediately);
		}
	}

	void Agent::returnToWaypoints(bool immediately) {
		auto waypoints = _waypoints.load();
		if (!waypoints)
			return;
		// search closest waypoint
		auto nearest = findNearestPointOnPath(waypoints->pts(), translate());
		Qk_ASSERT(nearest.segIndex != -1);
		moveTo(nearest.point, immediately); // move to closest point first
		_currentWaypoint = nearest.segIndex + 1;

		_async_call([](auto self, auto arg) {
			auto waypoints = self->_waypoints.load();
			if (waypoints && arg.arg < waypoints->ptsLen()) {
				self->_currentWaypoint = arg.arg; // set current waypoint again
			}
		}, this, nearest.segIndex + 1);
	}

	void Agent::stop() {
		set_waypoints(nullptr); // clear waypoints
		moveTo(translate()); // move to current position and clear follow target
	}

	void Agent::reportDirectionChange(Vec2 dir) {
		float dot = dir.dot(_heading);
		// Only report direction change if heading differs significantly.
		// Quick zero-dir guard: skip costly lengthSq() unless dot is invalid.
		if (dot < 0.9f && (dot > GEOM_EPS || dir.lengthSq() > GEOM_EPS)) {
			_heading = dir;//(_heading * 0.85f + dir * 0.15f).normalized();
			onUIEvent(UIEvent_AgentHeadingChange, this, new AgentStateEvent(this));
		}
	}

	bool Agent::isSpeedBelowRatio(float ratio) const {
		float speedSq = _velocity.lengthSq();
		float maxSpeedSq = _velocityMax * _velocityMax;
		return speedSq < maxSpeedSq * ratio * ratio;
	}
}
