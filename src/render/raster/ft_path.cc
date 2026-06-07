
#include "./ft_path.h"

using namespace qk;

namespace qk {
	uint32_t alignUp(uint32_t ptr, uint32_t alignment = alignof(void*));
}

Qk_FT_Outline* qk_ft_outline_create(int points, int contours)
{
	 char* ptr = (char*)malloc(alignUp(sizeof(Qk_FT_Outline)) +
		alignUp(points * sizeof(Qk_FT_Vector)) +
		alignUp(points * sizeof(char)) +
		alignUp(contours * sizeof(short)) +
		alignUp(contours * sizeof(char)));
	uint32_t offset = 0;

	Qk_FT_Outline* ft = (Qk_FT_Outline*)(ptr + offset);
	offset += alignUp(sizeof(Qk_FT_Outline));
	ft->points = (Qk_FT_Vector*)(ptr + offset);
	offset += alignUp(points * sizeof(Qk_FT_Vector));
	ft->tags = (char*)(ptr + offset);
	offset += alignUp(points * sizeof(char));
	ft->contours = (short*)(ptr + offset);
	offset += alignUp(contours * sizeof(short));
	ft->contours_flag = (char*)(ptr + offset);
	memset(ft->contours_flag, 0, contours * sizeof(char)); // default to close path
	ft->n_points = ft->n_contours = 0;
	ft->flags = 0x0;
	return ft;
}

void qk_ft_outline_destroy(Qk_FT_Outline* ft)
{
	free(ft);
}

static void qk_ft_outline_move_to(Qk_FT_Outline* ft, Vec2 p)
{
	ft->points[ft->n_points].x = FT_1616(p.x());
	ft->points[ft->n_points].y = FT_1616(p.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_ON;
	
	//Qk_DLog("qk_ft_outline_move_to, %d, %d", ft->points[ft->n_points].x, ft->points[ft->n_points].y);
	
	if(ft->n_points)
	{
		ft->contours[ft->n_contours] = ft->n_points - 1; // end point of last contour
		ft->n_contours++;
	}

	ft->contours_flag[ft->n_contours] = 1; // open path
	ft->n_points++;
}

static void qk_ft_outline_line_to(Qk_FT_Outline* ft, Vec2 p)
{
	ft->points[ft->n_points].x = FT_1616(p.x());
	ft->points[ft->n_points].y = FT_1616(p.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_ON;
	ft->n_points++;
}

static void qk_ft_outline_cubic_to(Qk_FT_Outline* ft, Vec2 p1, Vec2 p2, Vec2 p3)
{
	ft->points[ft->n_points].x = FT_1616(p1.x());
	ft->points[ft->n_points].y = FT_1616(p1.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_CUBIC;
	ft->n_points++;

	ft->points[ft->n_points].x = FT_1616(p2.x());
	ft->points[ft->n_points].y = FT_1616(p2.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_CUBIC;
	ft->n_points++;

	ft->points[ft->n_points].x = FT_1616(p3.x());
	ft->points[ft->n_points].y = FT_1616(p3.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_ON;
	ft->n_points++;
}

static void qk_ft_outline_conic_to(Qk_FT_Outline* ft, Vec2 p1, Vec2 p2)
{
	ft->points[ft->n_points].x = FT_1616(p1.x());
	ft->points[ft->n_points].y = FT_1616(p1.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_CONIC;
	ft->n_points++;

	ft->points[ft->n_points].x = FT_1616(p2.x());
	ft->points[ft->n_points].y = FT_1616(p2.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_ON;
	ft->n_points++;
}

static void qk_ft_outline_close(Qk_FT_Outline* ft)
{
	ft->contours_flag[ft->n_contours] = 0; // close path
}

static void qk_ft_outline_end(Qk_FT_Outline* ft)
{
	if (ft->n_points) {
		ft->contours[ft->n_contours] = ft->n_points - 1; // end point
		ft->n_contours++;
	}
}

Qk_FT_Outline* qk_ft_outline_from(const Path* path)
{
	auto verbs = path->verbs();
	auto pts = path->pts().val();
	int ptsLen = path->ptsLen();
	Qk_FT_Outline* outline = qk_ft_outline_create(ptsLen,ptsLen);

	for (int i = 0, len = path->verbsLen(); i < len; i++)
	{
		switch(verbs[i]) {
			case Path::kMove_Verb:
				qk_ft_outline_move_to(outline, *pts);
				pts++;
				break;
			case Path::kLine_Verb:
				qk_ft_outline_line_to(outline, *pts);
				pts++;
				break;
			case Path::kQuad_Verb:
				qk_ft_outline_conic_to(outline, pts[0],pts[1]);
				pts += 2;
				break;
			case Path::kCubic_Verb:
				qk_ft_outline_cubic_to(outline, pts[0],pts[1],pts[2]);
				pts += 3;
				break;
			case Path::kClose_Verb:
				qk_ft_outline_close(outline);
				break;
		}
	}

	qk_ft_outline_end(outline);
	return outline;
}

Qk_FT_Outline* qk_ft_outline_from_contours(const PathContour *contours, int contoursNum) {
	int ptsLen = 0;
	for (int i = 0; i < contoursNum; i++)
		ptsLen += contours[i].ptsNum; // make sure the pts pointer is valid
	Qk_FT_Outline* outline = qk_ft_outline_create(ptsLen,ptsLen);

	for (int i = 0; i < contoursNum; i++) {
		const PathContour &c = contours[i];
		qk_ft_outline_move_to(outline, c.pts[0]);
		for (int j = 1; j < c.ptsNum; j++) {
			qk_ft_outline_line_to(outline, c.pts[j]);
		}
		if (c.close) {
			qk_ft_outline_close(outline);
		}
	}
	qk_ft_outline_end(outline);
	return outline;
}

#define FT_Vec2(a) (Vec2(FT_1616_F((a).x),FT_1616_F((a).y)))

Qk_FT_Error qk_ft_path_convert(Qk_FT_Outline* outline, Path *out)
{
	auto pts = outline->points;
	auto n_points = outline->n_points;
	auto contours = outline->contours;
	auto tags = outline->tags;

	for (int c = 0, i = 0; c < outline->n_contours; c++) {
		Vec2 begin;
		short end = contours[c];

		if (tags[i] == Qk_FT_CURVE_TAG_ON) {
			Qk_DLog("Qk_FT_CURVE_TAG_ON moveTo, %d, x: %f, y: %f", i, FT_1616_F(pts[i].x), FT_1616_F(pts[i].y));
			out->moveTo(begin = FT_Vec2(pts[i]));
			i++;
		}

		while (i <= end) {
			switch (tags[i]) {
				case Qk_FT_CURVE_TAG_ON: // line to
					Qk_DLog("Qk_FT_CURVE_TAG_ON, i: %d, x: %f, y: %f", i, FT_1616_F(pts[i].x), FT_1616_F(pts[i].y));
					out->lineTo(FT_Vec2(pts[i]));
					i++;
					break;
				case Qk_FT_CURVE_TAG_CONIC:
					Qk_ASSERT(i > 0);
					out->quadTo(FT_Vec2(pts[i]), i+1<=end ? FT_Vec2(pts[i+1]): begin);
					i += i+1<=end? 1: 2;
					break;
				case Qk_FT_CURVE_TAG_CUBIC:
					Qk_ASSERT(i > 0);
					//Qk_DLog("Qk_FT_CURVE_TAG_CUBIC, i: %d, xy:%d %d, xy1:%d %d, xy2:%d %d", i,
					//	pts[i].x, pts[i].y, pts[i+1].x, pts[i+1].y, to.x, to.y
					//);
					out->cubicTo(FT_Vec2(pts[i]), FT_Vec2(pts[i+1]), i+2<=end ? FT_Vec2(pts[i+2]): begin);
					i += i+2<=end? 2: 3;
					break;
				default:
					Qk_Fatal("qk_ft_path_convert");
					break;
			}
		}

		if (outline->contours_flag[c] == 0) { // close path
			out->close();
		}
	}

	return 0;
}

Qk_FT_Error qk_ft_stroke_border_export(Qk_FT_StrokeBorder border, Path* path)
{
	Qk_FT_Vector* pts = border->points;
	Qk_FT_Byte* tags = border->tags;
	Array<int> contours;

	for (int i = 0; i < border->num_points; i++) {
		if (tags[i] & Qk_FT_STROKE_TAG_END)
			contours.push(i); // index of the last point in the contour
	}

	for (int c = 0, i = 0; c < contours.length(); c++) {
		Vec2 begin;
		short end = contours[c];
		if (tags[i] & Qk_FT_STROKE_TAG_ON) {
			path->moveTo(begin = FT_Vec2(pts[i]));
			i++;
		}
		while (i <= end) {
			Qk_ASSERT(i < border->num_points, "invalid stroke border data");
			if (tags[i] & Qk_FT_STROKE_TAG_ON) { // line
				path->lineTo(FT_Vec2(pts[i]));
				i++;
			} else if (tags[i] & Qk_FT_STROKE_TAG_CUBIC) { // cubic
				path->cubicTo(FT_Vec2(pts[i]), FT_Vec2(pts[i+1]), i+2<=end ? FT_Vec2(pts[i+2]): begin);
				i += i+2<=end? 2: 3;
			} else { // conic
				path->quadTo(FT_Vec2(pts[i]), i+1<=end ? FT_Vec2(pts[i+1]): begin);
				i += i+1<=end? 1: 2;
			}
		}
		path->close(); // always close path for stroke border
	}

	return 0;
}
