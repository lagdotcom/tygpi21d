/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define OP_SKIP	0
#define OP_JUMP 1
#define OP_END	2
#define OP_DATA	3

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport box2d whole_screen = {
	{ 0, 0 },
	{ SCREEN_WIDTH, SCREEN_HEIGHT }
};

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Draw_GRF(point2d *xy, grf *g, int img, colour tint)
{
	Draw_GRF_Clipped(xy, g, img, tint, &whole_screen);
}

void Draw_GRF_Clipped(point2d *xy, grf *g, int img, colour tint, box2d *bounds)
{
	int x, y, ex;
	grf_image *im;
	unsigned char *d, i, len, c;

	if (img >= g->num_images)
		dief("Draw_GRF_Clipped: image index too high (%d > %d)", img, g->num_images);

	if (bounds == null)
		bounds = &whole_screen;

	im = &g->images[img];
	x = xy->x;
	y = xy->y;
	ex = x + im->width;
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
					c = *(d++);
					if (x >= bounds->start.x && x < bounds->end.x && y >= bounds->start.y && y < bounds->end.y)
						Plot_Pixel_Fast_DB(x, y, tint ? tint : c);

					x++;
					if (x >= ex) {
						x = xy->x;
						y++;
					}
				}
				break;
		}
	}
}

bool Read_GRF(FILE *fp, grf *g)
{
	int i;
	grf_image *im;

	fread(g, GRF_HEADER_SZ, 1, fp);
	Check_Version_Header_p(g, "Read_GRF");

	g->images = im = SzAlloc(g->num_images, grf_image, "Read_GRF.images");
	for (i = 0; i < g->num_images; i++, im++) {
		fread(im, GRF_IMAGE_SZ, 1, fp);
		im->data = Allocate(im->datasize, 1, "Read_GRF.image");
		fread(im->data, im->datasize, 1, fp);
	}

	return true;
}

bool Load_GRF(char *filename, grf *g, char *tag)
{
	bool result;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		dief("Load_GRF: Could not open for reading: %s\n", filename);
		return false;
	}

	Log("Load_GRF: %s (%s)", filename, tag);
	result = Read_GRF(fp, g);
	fclose(fp);
	return result;
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
