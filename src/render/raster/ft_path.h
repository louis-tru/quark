// @private head
#ifndef __quark_render_ft_path__
#define __quark_render_ft_path__
extern "C" {
#include "./ft_stroke.h"
}
#include "../path.h"

// convert to 16.16 fixed-point pixel
#define FT_1616(x) (Qk_FT_Pos)((x) * 256.0)
// 16.16 fixed-point pixel convert to float
#define FT_1616_F(x) (float(x) * 0.00390625)

Qk_FT_Outline* qk_ft_outline_create(int points, int contours);
void           qk_ft_outline_destroy(Qk_FT_Outline* ft);
Qk_FT_Outline* qk_ft_outline_convert(const qk::Path* path);
Qk_FT_Error    qk_ft_path_convert(Qk_FT_Outline* outline, qk::Path *out);

#endif
