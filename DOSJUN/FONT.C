/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "gamelib.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport int Draw_Font_Char(int sx, int sy, colour col, char ch, font *f, bool trans_flag)
{
	int c, r;
	int font_row = ch / 16;
	int font_col = ch % 16;
	int fx = font_col * f->header.width;
	int fy = font_row * f->header.height;
	int char_width = f->header.c_width[ch];
	int i = fy * f->header.width * 16 + fx;
	int x = sx;
	int y = sy;
	colour fc;

	for (r = 0; r < f->header.height; r++) {
		for (c = 0; c < char_width; c++) {
			fc = f->img[i] ? col : 0;
			if (fc || !trans_flag) {
				Plot_Pixel_Fast_DB(x, y, fc);
			}

			x++;
			i++;
		}

		i += f->stride - char_width + 1;
		x = sx;
		y++;
	}

	return sx + char_width;
}

void Draw_Font(int sx, int sy, colour col, char *string, font *f, bool trans_flag)
{
	int x = sx;
	int y = sy;
	char *ch = string;

	while (*ch != 0) {
		if (*ch == '\n') {
			x = sx;
			y += f->header.height;
			ch++;
			continue;
		}

		x = Draw_Font_Char(x, y, col, *ch, f, trans_flag);
		ch++;
	}
}

bool Load_Font(char *filename, font *f)
{
	pcx_picture pcx;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Could not open for reading: %s\n", filename);
		return false;
	}

	Log("Load_Font: %s", filename);

	fread(&f->header, sizeof(font_header), 1, fp);
	Check_Version_Header(f->header);
	fclose(fp);

	PCX_Init(&pcx);
	if (!Load_Picture(f->header.filename, &pcx, filename)) {
		return false;
	}

	f->img = pcx.buffer;
	f->stride = pcx.header.width;
	return true;
}

void Free_Font(font *f)
{
	Free(f->img);
}
