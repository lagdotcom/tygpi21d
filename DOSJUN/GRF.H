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

typedef struct point {
	int x;
	int y;
} point;

typedef struct box {
	point start;
	point end;
} box;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Draw_GRF(point *xy, grf *g, int img, colour tint);
void Draw_GRF_Clipped(point *xy, grf *g, int img, colour tint, box *bounds);
void Free_GRF(grf *g);
bool Load_GRF(char *filename, grf *g, char *tag);
bool Read_GRF(FILE *fp, grf *g); 

#endif /* DOSJUN_GRF_H */
