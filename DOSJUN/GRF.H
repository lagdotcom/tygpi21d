#ifndef DOSJUN_GRF_H
#define DOSJUN_GRF_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define GRF_HEADER_SZ		32
#define GRF_HEADER_PADDING	26
#define GRF_IMAGE_SZ		8		

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct grf_image {
	UINT16 width;
	UINT16 height;
	UINT16 datasize;
	char padding[2];
	unsigned char *data;
} grf_image;

typedef struct grf {
	VERSION_HEADER;
	UINT16 num_images;
	char padding[GRF_HEADER_PADDING];
	grf_image *images;
} grf;

typedef struct point2d {
	int x;
	int y;
} point2d;

typedef struct size2d {
	int w;
	int h;
} size2d;

typedef struct box2d {
	point2d start;
	point2d end;
} box2d;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Draw_GRF(point2d *xy, grf *g, int img, colour tint);

/* Draw a GRF image onto the screen, clipped within a box.
bounds: if null, defaults to the whole screen
*/
void Draw_GRF_Clipped(point2d *xy, grf *g, int img, colour tint, box2d *bounds);

void Free_GRF(grf *g);
bool Load_GRF(char *filename, grf *g, char *tag);
bool Read_GRF(FILE *fp, grf *g); 

#endif /* DOSJUN_GRF_H */
