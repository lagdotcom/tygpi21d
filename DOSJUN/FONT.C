/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "gamelib.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport int Char_Width(grf *f, char ch)
{
	if (ch >= f->num_images) {
		ch = ' ';
	}

	return f->images[ch].width;
}

/* TODO: use trans_flag? */
int Draw_Font_Char(int sx, int sy, colour col, char ch, grf *f, bool trans_flag)
{
	point p;
	p.x = sx;
	p.y = sy;

	Draw_GRF(&p, f, ch, col);
	return sx + Char_Width(f, ch);
}

void Draw_Font(int sx, int sy, colour col, char *string, grf *f, bool trans_flag)
{
	int x = sx,
		y = sy;
	char *ch = string;

	while (*ch != 0) {
		if (*ch == '\n') {
			x = sx;
			y += 8; /* TODO */
			ch++;
			continue;
		}

		x = Draw_Font_Char(x, y, col, *ch, f, trans_flag);
		ch++;
	}
}

/* Wrap a string to fit in a given box size. You must NOT Free() the returned string. */
noexport char *Font_Wrap(int w, int h, char *string, grf *f)
{
	/* TODO: pay attention to height */
	char *wrapped,
		*ch,
		*last_space;
	int last_space_x = 0,
		x = 0;

	wrapped = Duplicate_String(string, "Font_Wrap");
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

		x += Char_Width(f, *ch);
		last_space_x += Char_Width(f, *ch);
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
void Draw_Wrapped_Font(int x, int y, int w, int h, colour col, char *string, grf *f, bool margin)
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
	Free(wrapped);
}
