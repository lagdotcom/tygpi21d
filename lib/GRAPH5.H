#ifndef GAMELIB_GRAPH_5
#define GAMELIB_GRAPH_5

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "graph3.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MAX_VERTICES	16

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct vertex_typ {
	float x,y;
} vertex, *vertex_ptr;

typedef struct polygon_typ {
	int b_color;	/* border */
	int i_color;	/* interior */
	int closed;
	int filled;
	int lxo, lyo;	/* local origin */
	int num_vertices;
	vertex vertices[MAX_VERTICES];
} polygon, *polygon_ptr;

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* Polygon clipping extents */
extern int poly_clip_min_x,
	poly_clip_min_y,
	poly_clip_max_x,
	poly_clip_max_y;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Bline(int xo, int yo, int x1, int y1, unsigned char color);
int Clip_Line(int *x1, int *y1, int *x2, int *y2);
void Create_Tables(void);
void Draw_Boundary(int color);
void Draw_Polygon(polygon_ptr poly);
void Draw_Polygon_Clip(polygon_ptr poly);
void Rotate_Polygon(polygon_ptr poly, int angle);
void Scale_Polygon(polygon_ptr poly, float scale);
void Translate_Polygon(polygon_ptr poly, int dx, int dy);

#endif
