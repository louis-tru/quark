// @private head
#ifndef Qk_FT_TYPES_H
#define Qk_FT_TYPES_H

/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    Qk_FT_Fixed                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    This type is used to store 16.16 fixed-point values, like scaling  */
/*    values or matrix coefficients.                                     */
/*                                                                       */
typedef signed long  Qk_FT_Fixed;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    Qk_FT_Int                                                             */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef for the int type.                                        */
/*                                                                       */
typedef signed int  Qk_FT_Int;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    Qk_FT_UInt                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef for the unsigned int type.                               */
/*                                                                       */
typedef unsigned int  Qk_FT_UInt;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    Qk_FT_Long                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef for signed long.                                         */
/*                                                                       */
typedef signed long  Qk_FT_Long;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    Qk_FT_ULong                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef for unsigned long.                                       */
/*                                                                       */
typedef unsigned long Qk_FT_ULong;

/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    Qk_FT_Short                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef for signed short.                                        */
/*                                                                       */
typedef signed short  Qk_FT_Short;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    Qk_FT_Byte                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    A simple typedef for the _unsigned_ char type.                     */
/*                                                                       */
typedef unsigned char  Qk_FT_Byte;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    Qk_FT_Bool                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    A typedef of unsigned char, used for simple booleans.  As usual,   */
/*    values 1 and~0 represent true and false, respectively.             */
/*                                                                       */
typedef unsigned char  Qk_FT_Bool;



/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    Qk_FT_Error                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    The FreeType error code type.  A value of~0 is always interpreted  */
/*    as a successful operation.                                         */
/*                                                                       */
typedef int  Qk_FT_Error;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    Qk_FT_Pos                                                             */
/*                                                                       */
/* <Description>                                                         */
/*    The type Qk_FT_Pos is used to store vectorial coordinates.  Depending */
/*    on the context, these can represent distances in integer font      */
/*    units, or 16.16, or 26.6 fixed-point pixel coordinates.            */
/*                                                                       */
typedef signed long  Qk_FT_Pos;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    Qk_FT_Vector                                                          */
/*                                                                       */
/* <Description>                                                         */
/*    A simple structure used to store a 2D vector; coordinates are of   */
/*    the Qk_FT_Pos type.                                                   */
/*                                                                       */
/* <Fields>                                                              */
/*    x :: The horizontal coordinate.                                    */
/*    y :: The vertical coordinate.                                      */
/*                                                                       */
typedef struct  Qk_FT_Vector_
{
  Qk_FT_Pos  x;
  Qk_FT_Pos  y;

} Qk_FT_Vector;


typedef long long int           Qk_FT_Int64;
typedef unsigned long long int  Qk_FT_UInt64;

typedef signed int              Qk_FT_Int32;
typedef unsigned int            Qk_FT_UInt32;


#define Qk_FT_BOOL( x )  ( (Qk_FT_Bool)( x ) )

#define Qk_FT_SIZEOF_LONG 4

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE  0
#endif


#endif // Qk_FT_TYPES_H
