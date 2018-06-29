/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "features.h"
#include "gamelib.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

int Char_Width(grf *f, char ch)
{
	if (ch >= f->num_images) {
		ch = ' ';
	}

	return f->images[ch].width;
}

int Char_Height(grf *f, char ch)
{
	if (ch >= f->num_images) {
		ch = ' ';
	}

	return f->images[ch].height;
}

int Draw_Font_Char(int sx, int sy, colour tint, char ch, grf *f, bool trans_flag)
{
#if FEATURE_FONT
	int w;
	point2d p;
	p.x = sx;
	p.y = sy;
	w = Char_Width(f, ch);

	if (!trans_flag) {
		Draw_Square_DB(0, sx, sy, sx + w - 1, sy + Char_Height(f, ch) - 1, true);
	}

	Draw_GRF(&p, f, ch, tint);
	return sx + w;
#else
	return sx;
#endif
}

void Draw_Font(int sx, int sy, colour tint, const char *string, grf *f, bool trans_flag)
{
	int x = sx,
		y = sy;
	const char *ch = string;

	while (*ch != 0) {
		if (*ch == '\n') {
			x = sx;
			y += 8; /* TODO */
			ch++;
			continue;
		}

		x = Draw_Font_Char(x, y, tint, *ch, f, trans_flag);
		ch++;
	}
}

/* Measure how much space a string would take up when printed. Does not account for formatting chars. */
size2d Measure_String(grf *f, const char *string)
{
	int x = 0,
		y = 0;
	size2d result = { 0, 0 };
	size_t i,
		len = strlen(string);

	for (i = 0; i < len; i++) {
		if (string[i] == '\n') {
			result.h += y;
			x = 0;
			continue;
		}

		x += Char_Width(f, string[i]);
		y = Char_Height(f, string[i]);

		if (x > result.w)
			result.w = x;
	}

	result.h += y;
	return result;
}
