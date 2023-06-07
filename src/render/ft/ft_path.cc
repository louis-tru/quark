
#include "./ft_path.h"

using namespace qk;

#define FT_COORD(x) (Qk_FT_Pos)((x) * 64)
#define FT_COORD_F(x) (float(x) * 0.015625f)

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
	ft->points[ft->n_points].x = FT_COORD(p.x());
	ft->points[ft->n_points].y = FT_COORD(p.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_ON;
	if(ft->n_points)
	{
		ft->contours[ft->n_contours] = ft->n_points - 1;
		ft->n_contours++;
	}

	ft->contours_flag[ft->n_contours] = 1; // move to
	ft->n_points++;
}

static void qk_ft_outline_line_to(Qk_FT_Outline* ft, Vec2 p)
{
	ft->points[ft->n_points].x = FT_COORD(p.x());
	ft->points[ft->n_points].y = FT_COORD(p.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_ON;
	ft->n_points++;
}

static void qk_ft_outline_cubic_to(Qk_FT_Outline* ft, Vec2 p1, Vec2 p2, Vec2 p3)
{
	ft->points[ft->n_points].x = FT_COORD(p1.x());
	ft->points[ft->n_points].y = FT_COORD(p1.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_CUBIC;
	ft->n_points++;

	ft->points[ft->n_points].x = FT_COORD(p2.x());
	ft->points[ft->n_points].y = FT_COORD(p2.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_CUBIC;
	ft->n_points++;

	ft->points[ft->n_points].x = FT_COORD(p3.x());
	ft->points[ft->n_points].y = FT_COORD(p3.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_ON;
	ft->n_points++;
}

static void qk_ft_outline_conic_to(Qk_FT_Outline* ft, Vec2 p1, Vec2 p2)
{
	ft->points[ft->n_points].x = FT_COORD(p1.x());
	ft->points[ft->n_points].y = FT_COORD(p1.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_CONIC;
	ft->n_points++;

	ft->points[ft->n_points].x = FT_COORD(p2.x());
	ft->points[ft->n_points].y = FT_COORD(p2.y());
	ft->tags[ft->n_points] = Qk_FT_CURVE_TAG_CONIC;
	ft->n_points++;
}

static void qk_ft_outline_close(Qk_FT_Outline* ft)
{
	ft->contours_flag[ft->n_contours] = 0; // close
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
		ft->contours[ft->n_contours] = ft->n_points - 1;
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

Qk_FT_Error qk_ft_path_convert(Qk_FT_Outline* outline, Path *out) {
	// TODO ...
	// outline

	return 0;
}
