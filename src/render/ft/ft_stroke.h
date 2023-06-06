#ifndef Qk_FT_STROKER_H
#define Qk_FT_STROKER_H
/***************************************************************************/
/*                                                                         */
/*  ftstroke.h                                                             */
/*                                                                         */
/*    FreeType path stroker (specification).                               */
/*                                                                         */
/*  Copyright 2002-2006, 2008, 2009, 2011-2012 by                          */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#include "ft_raster.h"

 /**************************************************************
	*
	* @type:
	*   Qk_FT_Stroker
	*
	* @description:
	*   Opaque handler to a path stroker object.
	*/
	typedef struct Qk_FT_StrokerRec_*  Qk_FT_Stroker;


	/**************************************************************
	 *
	 * @enum:
	 *   Qk_FT_Stroker_LineJoin
	 *
	 * @description:
	 *   These values determine how two joining lines are rendered
	 *   in a stroker.
	 *
	 * @values:
	 *   Qk_FT_STROKER_LINEJOIN_ROUND ::
	 *     Used to render rounded line joins.  Circular arcs are used
	 *     to join two lines smoothly.
	 *
	 *   Qk_FT_STROKER_LINEJOIN_BEVEL ::
	 *     Used to render beveled line joins.  The outer corner of
	 *     the joined lines is filled by enclosing the triangular
	 *     region of the corner with a straight line between the
	 *     outer corners of each stroke.
	 *
	 *   Qk_FT_STROKER_LINEJOIN_MITER_FIXED ::
	 *     Used to render mitered line joins, with fixed bevels if the
	 *     miter limit is exceeded.  The outer edges of the strokes
	 *     for the two segments are extended until they meet at an
	 *     angle.  If the segments meet at too sharp an angle (such
	 *     that the miter would extend from the intersection of the
	 *     segments a distance greater than the product of the miter
	 *     limit value and the border radius), then a bevel join (see
	 *     above) is used instead.  This prevents long spikes being
	 *     created.  Qk_FT_STROKER_LINEJOIN_MITER_FIXED generates a miter
	 *     line join as used in PostScript and PDF.
	 *
	 *   Qk_FT_STROKER_LINEJOIN_MITER_VARIABLE ::
	 *   Qk_FT_STROKER_LINEJOIN_MITER ::
	 *     Used to render mitered line joins, with variable bevels if
	 *     the miter limit is exceeded.  The intersection of the
	 *     strokes is clipped at a line perpendicular to the bisector
	 *     of the angle between the strokes, at the distance from the
	 *     intersection of the segments equal to the product of the
	 *     miter limit value and the border radius.  This prevents
	 *     long spikes being created.
	 *     Qk_FT_STROKER_LINEJOIN_MITER_VARIABLE generates a mitered line
	 *     join as used in XPS.  Qk_FT_STROKER_LINEJOIN_MITER is an alias
	 *     for Qk_FT_STROKER_LINEJOIN_MITER_VARIABLE, retained for
	 *     backwards compatibility.
	 */
	typedef enum  Qk_FT_Stroker_LineJoin_
	{
		Qk_FT_STROKER_LINEJOIN_ROUND          = 0,
		Qk_FT_STROKER_LINEJOIN_BEVEL          = 1,
		Qk_FT_STROKER_LINEJOIN_MITER_VARIABLE = 2,
		Qk_FT_STROKER_LINEJOIN_MITER          = Qk_FT_STROKER_LINEJOIN_MITER_VARIABLE,
		Qk_FT_STROKER_LINEJOIN_MITER_FIXED    = 3

	} Qk_FT_Stroker_LineJoin;


	/**************************************************************
	 *
	 * @enum:
	 *   Qk_FT_Stroker_LineCap
	 *
	 * @description:
	 *   These values determine how the end of opened sub-paths are
	 *   rendered in a stroke.
	 *
	 * @values:
	 *   Qk_FT_STROKER_LINECAP_BUTT ::
	 *     The end of lines is rendered as a full stop on the last
	 *     point itself.
	 *
	 *   Qk_FT_STROKER_LINECAP_ROUND ::
	 *     The end of lines is rendered as a half-circle around the
	 *     last point.
	 *
	 *   Qk_FT_STROKER_LINECAP_SQUARE ::
	 *     The end of lines is rendered as a square around the
	 *     last point.
	 */
	typedef enum  Qk_FT_Stroker_LineCap_
	{
		Qk_FT_STROKER_LINECAP_BUTT = 0,
		Qk_FT_STROKER_LINECAP_ROUND,
		Qk_FT_STROKER_LINECAP_SQUARE

	} Qk_FT_Stroker_LineCap;


	/**************************************************************
	 *
	 * @enum:
	 *   Qk_FT_StrokerBorder
	 *
	 * @description:
	 *   These values are used to select a given stroke border
	 *   in @Qk_FT_Stroker_GetBorderCounts and @Qk_FT_Stroker_ExportBorder.
	 *
	 * @values:
	 *   Qk_FT_STROKER_BORDER_LEFT ::
	 *     Select the left border, relative to the drawing direction.
	 *
	 *   Qk_FT_STROKER_BORDER_RIGHT ::
	 *     Select the right border, relative to the drawing direction.
	 *
	 * @note:
	 *   Applications are generally interested in the `inside' and `outside'
	 *   borders.  However, there is no direct mapping between these and the
	 *   `left' and `right' ones, since this really depends on the glyph's
	 *   drawing orientation, which varies between font formats.
	 *
	 *   You can however use @Qk_FT_Outline_GetInsideBorder and
	 *   @Qk_FT_Outline_GetOutsideBorder to get these.
	 */
	typedef enum  Qk_FT_StrokerBorder_
	{
		Qk_FT_STROKER_BORDER_LEFT = 0,
		Qk_FT_STROKER_BORDER_RIGHT

	} Qk_FT_StrokerBorder;


	/**************************************************************
	 *
	 * @function:
	 *   Qk_FT_Stroker_New
	 *
	 * @description:
	 *   Create a new stroker object.
	 *
	 * @input:
	 *   library ::
	 *     FreeType library handle.
	 *
	 * @output:
	 *   astroker ::
	 *     A new stroker object handle.  NULL in case of error.
	 *
	 * @return:
	 *    FreeType error code.  0~means success.
	 */
	Qk_FT_Error
	Qk_FT_Stroker_New( Qk_FT_Stroker  *astroker );


	/**************************************************************
	 *
	 * @function:
	 *   Qk_FT_Stroker_Set
	 *
	 * @description:
	 *   Reset a stroker object's attributes.
	 *
	 * @input:
	 *   stroker ::
	 *     The target stroker handle.
	 *
	 *   radius ::
	 *     The border radius.
	 *
	 *   line_cap ::
	 *     The line cap style.
	 *
	 *   line_join ::
	 *     The line join style.
	 *
	 *   miter_limit ::
	 *     The miter limit for the Qk_FT_STROKER_LINEJOIN_MITER_FIXED and
	 *     Qk_FT_STROKER_LINEJOIN_MITER_VARIABLE line join styles,
	 *     expressed as 16.16 fixed-point value.
	 *
	 * @note:
	 *   The radius is expressed in the same units as the outline
	 *   coordinates.
	 */
	void
	Qk_FT_Stroker_Set( Qk_FT_Stroker           stroker,
									Qk_FT_Fixed             radius,
									Qk_FT_Stroker_LineCap   line_cap,
									Qk_FT_Stroker_LineJoin  line_join,
									Qk_FT_Fixed             miter_limit );

	/**************************************************************
	 *
	 * @function:
	 *   Qk_FT_Stroker_ParseOutline
	 *
	 * @description:
	 *   A convenience function used to parse a whole outline with
	 *   the stroker.  The resulting outline(s) can be retrieved
	 *   later by functions like @Qk_FT_Stroker_GetCounts and @Qk_FT_Stroker_Export.
	 *
	 * @input:
	 *   stroker ::
	 *     The target stroker handle.
	 *
	 *   outline ::
	 *     The source outline.
	 *
	 *
	 * @return:
	 *   FreeType error code.  0~means success.
	 *
	 * @note:
	 *   If `opened' is~0 (the default), the outline is treated as a closed
	 *   path, and the stroker generates two distinct `border' outlines.
	 *
	 *
	 *   This function calls @Qk_FT_Stroker_Rewind automatically.
	 */
	Qk_FT_Error
	Qk_FT_Stroker_ParseOutline( Qk_FT_Stroker   stroker,
														 const Qk_FT_Outline*  outline);


	/**************************************************************
	 *
	 * @function:
	 *   Qk_FT_Stroker_GetCounts
	 *
	 * @description:
	 *   Call this function once you have finished parsing your paths
	 *   with the stroker.  It returns the number of points and
	 *   contours necessary to export all points/borders from the stroked
	 *   outline/path.
	 *
	 * @input:
	 *   stroker ::
	 *     The target stroker handle.
	 *
	 * @output:
	 *   anum_points ::
	 *     The number of points.
	 *
	 *   anum_contours ::
	 *     The number of contours.
	 *
	 * @return:
	 *   FreeType error code.  0~means success.
	 */
	Qk_FT_Error
	Qk_FT_Stroker_GetCounts( Qk_FT_Stroker  stroker,
												Qk_FT_UInt    *anum_points,
												Qk_FT_UInt    *anum_contours );


	/**************************************************************
	 *
	 * @function:
	 *   Qk_FT_Stroker_Export
	 *
	 * @description:
	 *   Call this function after @Qk_FT_Stroker_GetBorderCounts to
	 *   export all borders to your own @Qk_FT_Outline structure.
	 *
	 *   Note that this function appends the border points and
	 *   contours to your outline, but does not try to resize its
	 *   arrays.
	 *
	 * @input:
	 *   stroker ::
	 *     The target stroker handle.
	 *
	 *   outline ::
	 *     The target outline handle.
	 */
	void
	Qk_FT_Stroker_Export( Qk_FT_Stroker   stroker,
										 Qk_FT_Outline*  outline );


	/**************************************************************
	 *
	 * @function:
	 *   Qk_FT_Stroker_Done
	 *
	 * @description:
	 *   Destroy a stroker object.
	 *
	 * @input:
	 *   stroker ::
	 *     A stroker handle.  Can be NULL.
	 */
	void
	Qk_FT_Stroker_Done( Qk_FT_Stroker  stroker );


#endif // Qk_FT_STROKER_H
