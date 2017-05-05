#ifndef GAMELIB_GRAPH_8
#define GAMELIB_GRAPH_8

/* I N C L U D E S /////////////////////////////////////////////////////// */

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define FP_SHIFT		8		/* number of decimal digits */
#define FP_SHIFT_2N		256		/* 2^FP_SHIFT */
#define FP_FRAC_MASK	(FP_SHIFT_2N - 1)

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef long fixed;

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* global pixel position and color */
extern int g_x, g_y;
extern unsigned char g_color;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

#define Plot_DB(x, y, color) g_x = x; g_y = y; g_color = color; \
	Plot_Pixel_Global();

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

/* new optimisations */
void Plot_Pixel_Global(void);

/* fixed-point maths */
fixed Assign_Float(float number);
fixed Assign_Integer(long integer);
fixed Mul_Fixed(fixed f1, fixed f2);
fixed Div_Fixed(fixed f1, fixed f2);
fixed Add_Fixed(fixed f1, fixed f2);
fixed Sub_Fixed(fixed f1, fixed f2);
void Print_Fixed(fixed f1);

#endif
