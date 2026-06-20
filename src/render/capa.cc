/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./capa.h"
#include "./gpu_canvas.h"

namespace qk {
	CAPABuilder::CAPABuilder(GPUCanvas *owner): _owner(owner) {
		reset();
	}

	bool CAPABuilder::buildColor(const Path &rawPath, const Color4f &color) {
		auto lines = _owner->_cache->getEdgeLines(rawPath, _owner->_allScaleAverage * 0.5f);
		if (lines.length() < 2)
			return false;

		auto &matrix = _owner->_state->matrix;
		Vec2 surfaceScale = _owner->_surfaceScale;
		auto pathIndex = _data.paths.length();
		auto edgeOffset = _data.edges.length();
		CAPAPath path {
			.matrixX = Vec4(
				matrix[0] * surfaceScale.x(),
				matrix[1] * surfaceScale.x(),
				matrix[2] * surfaceScale.x(),
				0
			),
			.matrixY = Vec4(
				matrix[3] * surfaceScale.y(),
				matrix[4] * surfaceScale.y(),
				matrix[5] * surfaceScale.y(),
				0
			),
			.color = color,
			.edgeOffset = edgeOffset,
			.edgeCount = lines.length() >> 1,
		};
		_data.paths.push(path);
		_data.edges.reset(edgeOffset + path.edgeCount);

		for (uint32_t i = 1; i < lines.length(); i += 2) {
			_data.edges[edgeOffset++] = CAPAEdge{
				.p0 = lines[i - 1],
				.p1 = lines[i],
				.pathIndex = pathIndex,
			};
		}

		return true;
	}

	cCAPADrawData& CAPABuilder::endBuild() {
		if (!_data.edges.length())
			return _data;
		_data.atlas = _owner->getTextureFromPool(
			_owner->_surfaceSize, kLuminance_8_ColorType,
			_owner->_surfaceSize, kComputeWrite_TextureFlags
		);
		return _data;
	}

	void CAPABuilder::reset(bool clear) {
		AllocatorScope scope(&_alloc);
		_data = {};
		if (clear)
			_alloc.clear();
		else
			_alloc.reset();
	}
}
