/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define OP_SKIP	0
#define OP_JUMP 1
#define OP_END	2
#define OP_DATA	3

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Draw_GRF(int sx, int sy, grf *g, int img, int minx, int miny, int maxx, int maxy)
{
	int x, y, ex;
	grf_image *im;
	unsigned char *d, i, len;

	assert(img < g->num_images, "Draw_GRF: image index too high");

	im = &g->images[img];
	x = sx;
	y = sy;
	ex = sx + im->width;
	d = im->data;

	while (true) {
		switch (*d) {
			/* skip X, Y */
			case OP_SKIP:
				d++;
				x += *(d++);
				y += *(d++);
				break;

			/* skip -X, Y */
			case OP_JUMP:
				d++;
				x -= *(d++);
				y += *(d++);
				break;

			/* end image data */
			case OP_END:
				return;

			/* run of X pixels */
			default:
				len = *(d++) - OP_DATA + 1;
				for (i = 0; i < len; i++) {
					if (x >= minx && x < maxx && y >= miny && y < maxy)
						Plot_Pixel_Fast_DB(x, y, *(d++));

					x++;
					if (x >= ex) {
						x = sx;
						y++;
					}
				}
				break;
		}
	}
}

bool Load_GRF(char *filename, grf *g, char *tag)
{
	int i;
	grf_image *im;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		dief("Load_GRF: Could not open for reading: %s\n", filename);
		return false;
	}

	Log("Load_GRF: %s", filename);

	fread(g, GRF_HEADER_SZ, 1, fp);
	Check_Version_Header((*g));

	g->images = im = SzAlloc(g->num_images, grf_image, tag);
	for (i = 0; i < g->num_images; i++, im++) {
		fread(im, GRF_IMAGE_SZ, 1, fp);
		im->data = Allocate(im->datasize, 1, tag);
		fread(im->data, im->datasize, 1, fp);
	}

	fclose(fp);
	return true;
}

void Free_GRF(grf *g)
{
	int i;
	grf_image *im;

	for (i = 0, im = g->images; i < g->num_images; i++, im++) {
		Free(im->data);
	}

	Free(g->images);
	g->num_images = 0;
}
