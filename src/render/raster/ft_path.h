// @private head
#ifndef __quark_render_ft_path__
#define __quark_render_ft_path__
extern "C" {
#include "./ft_stroke.h"
}
#include "../path.h"

/**
 * A contour is a simple closed path. Paths can have multiple contours, and they can be nested.
 */
struct PathContour {
	int      start; // index of the first point in the contour
	int      ptsNum; // number of points in the contour
	float    area; // signed area of the contour
	float    normalSide; // sign assigned to the +normal side
	qk::Vec2 *pts; // pointer into the original path buffer
	qk::Vec2 sample; // a point just inside the contour, used for winding tests
	int      depth; // nesting depth
	bool     close; // whether the contour is closed
};

// convert to 16.16 fixed-point pixel
#define FT_1616(x) (Qk_FT_Pos)((x) * 256.0f)
// 16.16 fixed-point pixel convert to float
#define FT_1616_F(x) (float(x) * 0.00390625f)

Qk_FT_Outline* qk_ft_outline_create(int points, int contours);
void           qk_ft_outline_destroy(Qk_FT_Outline* ft);
Qk_FT_Outline* qk_ft_outline_from(const qk::Path* path);
Qk_FT_Outline* qk_ft_outline_from_contours(const PathContour *contours, int contoursNum);
Qk_FT_Error    qk_ft_path_convert(Qk_FT_Outline* outline, qk::Path *out);
Qk_FT_Error    qk_ft_stroke_border_export(Qk_FT_StrokeBorder border, qk::Path* path);

#endif
