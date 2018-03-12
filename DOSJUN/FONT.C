/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "gamelib.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

int Draw_Font_Char(int sx, int sy, colour col, char ch, font *f, bool trans_flag)
{
	int c,
		r,
		font_row = ch / 16,
		font_col = ch % 16,
		fx = font_col * f->header.width,
		fy = font_row * f->header.height,
		char_width = f->header.c_width[ch],
		i = fy * f->header.width * 16 + fx,
		x = sx,
		y = sy;
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

		i += f->stride - char_width;
		x = sx;
		y++;
	}

	return sx + char_width;
}

void Draw_Font(int sx, int sy, colour col, char *string, font *f, bool trans_flag)
{
	int x = sx,
		y = sy;
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

/* Wrap a string to fit in a given box size. You must NOT Free() the returned string. */
noexport char *Font_Wrap(int w, int h, char *string, font *f)
{
	/* TODO: use Duplicate_String()? */
	/* TODO: pay attention to height */
	char wrapped[200],
		*ch,
		*last_space;
	int last_space_x = 0,
		x = 0;

	strcpy(wrapped, string);
	ch = wrapped;

	while (*ch) {
		switch (*ch) {
			case '\n':
				x = 0;
				/* FALL THROUGH */
			case ' ':
				last_space = ch;
				last_space_x = 0;
				break;
		}

		x += f->header.c_width[*ch];
		last_space_x += f->header.c_width[*ch];
		if (x >= w && last_space > 0) {
			*last_space = '\n';

			x = last_space_x;
			last_space = 0;
		}

		ch++;
	}

	return wrapped;
}

/* Clears an area, then prints a string into it, with wrapping. */
void Draw_Wrapped_Font(int x, int y, int w, int h, colour col, char *string, font *f, bool margin)
{
	char *wrapped;

	if (margin) {
		x += 4;
		y += 4;
		w -= 8;
		h -= 8;
	}
	
	wrapped = Font_Wrap(w, h, string, f);
	Draw_Square_DB(0, x, y, x + w - 1, y + h - 1, true);
	Draw_Font(x, y, col, wrapped, f, true);
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
	f->stride = pcx.header.width + 1; /* PCX header correction */
	return true;
}

void Free_Font(font *f)
{
	Log("Free_Font: %p", f);

	Free(f->img);
}
