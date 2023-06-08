
#include "./ft_path.h"

using namespace qk;

Qk_FT_Outline* qk_ft_outline_create(int points, int contours)
{
	Qk_FT_Outline* ft = (Qk_FT_Outline*)malloc(sizeof(Qk_FT_Outline));
	ft->points = (Qk_FT_Vector*)malloc((size_t)(points + contours) * sizeof(Qk_FT_Vector));
	ft->tags = (char*)malloc((size_t)(points + contours) * sizeof(char));
	ft->contours = (short*)malloc((size_t)contours * sizeof(short));
	ft->contours_flag = (char*)malloc((size_t)contours * sizeof(char));
	ft->n_points = ft->n_contours = 0;
	ft->flags = 0x0;
	return ft;
}

void qk_ft_outline_destroy(Qk_FT_Outline* ft)
{
	free(ft->points);
	free(ft->tags);
	free(ft->contours);
	free(ft->contours_flag);
	free(ft);
}

static void qk_ft_outline_move_to(Qk_FT_Outline* ft, Vec2 p)
{
	ft->points[ft->n_points].x = FT_1616(p.x());
	ft->points[ft->n_points].y = FT_1616(p.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_ON;
	if(ft->n_points)
	{
		ft->contours[ft->n_contours] = ft->n_points - 1; // end point
		ft->n_contours++;
	}

	ft->contours_flag[ft->n_contours] = 1; // move to
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
	ft->contours_flag[ft->n_contours] = 0; // no move
	int index = ft->n_contours ? ft->contours[ft->n_contours - 1] + 1 : 0;
	if(index == ft->n_points)
			return;

	ft->points[ft->n_points].x = ft->points[index].x;
	ft->points[ft->n_points].y = ft->points[index].y;
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_ON;
	ft->n_points++;
}

static void qk_ft_outline_end(Qk_FT_Outline* ft)
{
	if (ft->n_points) {
		ft->contours[ft->n_contours] = ft->n_points - 1; // end point
		ft->n_contours++;
	}
}

Qk_FT_Outline* qk_ft_outline_convert(const Path* path)
{
	auto verbs = path->verbs();
	auto pts = path->pts();
	int ptsLen = path->ptsLen();
	Qk_FT_Outline* outline = qk_ft_outline_create(ptsLen,ptsLen);

	for (int i = 0, len = path->verbsLen(); i < len; i++)
	{
		switch(verbs[i]) {
			case Path::kVerb_Move:
				qk_ft_outline_move_to(outline, *pts);
				pts++;
				break;
			case Path::kVerb_Line:
				qk_ft_outline_line_to(outline, *pts);
				pts++;
				break;
			case Path::kVerb_Quad:
				qk_ft_outline_conic_to(outline, pts[0],pts[1]);
				pts += 2;
				break;
			case Path::kVerb_Cubic:
				qk_ft_outline_cubic_to(outline, pts[0],pts[1],pts[2]);
				pts += 3;
				break;
			case Path::kVerb_Close:
				qk_ft_outline_close(outline);
				break;
		}
	}

	qk_ft_outline_end(outline);
	return outline;
}

#define FT_Vec2(a) (Vec2(FT_1616_F((a).x),FT_1616_F((a).y)))

Qk_FT_Error qk_ft_path_convert(Qk_FT_Outline* outline, Path *out)
{
	auto pts = outline->points;
	auto contours = outline->contours;
	auto tags = outline->tags;

	for (int c = 0, i = 0; c < outline->n_contours; c++) {
		if (outline->contours_flag[c] == 1) { // move
			//Qk_DEBUG("tags[i] = %d, %d, %d, %d", i, tags[i], contours[c], outline->contours_flag[c]);
			Qk_ASSERT(tags[i] == Qk_FT_CURVE_TAG_ON);
			out->moveTo(FT_Vec2(pts[i]));
			i++;
		}

		while (i <= contours[c]) {
			switch (tags[i]) {
				case Qk_FT_CURVE_TAG_ON: // line to
					//Qk_DEBUG("Qk_FT_CURVE_TAG_ON, %d", i);
					if (out->ptsLen())
						out->lineTo(FT_Vec2(pts[i]));
					else
						out->moveTo(FT_Vec2(pts[i]));
					i++;
					break;
				case Qk_FT_CURVE_TAG_CONIC:
					Qk_ASSERT(i > 0);
					//Qk_DEBUG("Qk_FT_CURVE_TAG_CONIC, %d", i);
					out->quadTo(FT_Vec2(pts[i]), FT_Vec2(pts[i+1]));
					i+=2;
					break;
				case Qk_FT_CURVE_TAG_CUBIC:
					Qk_ASSERT(i > 0);
					//Qk_DEBUG("Qk_FT_CURVE_TAG_CUBIC, %d, %d, %d, %d", i, tags[i], tags[i+1], tags[i+2]);
					out->cubicTo(FT_Vec2(pts[i]), FT_Vec2(pts[i+1]), FT_Vec2(pts[i+2]));
					i+=3;
					break;
				default:
					Qk_FATAL("qk_ft_path_convert");
					break;
			}
		}
	}

	return 0;
}
