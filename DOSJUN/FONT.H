#ifndef DOSJUN_FONT_H
#define DOSJUN_FONT_H

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define FONT_HEADER_PADDING 14

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct font_header {
	VERSION_HEADER;
	char filename[12];
	unsigned char width;
	unsigned char height;
	unsigned char unused[FONT_HEADER_PADDING];

	unsigned char c_width[256];
} font_header;

typedef struct font {
	font_header header;
	char *img;
	int stride;
} font;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Draw_Font(int x, int y, colour col, char *string, font *f, bool trans_flag);
int Draw_Font_Char(int sx, int sy, colour col, char ch, font *f, bool trans_flag);

/* Clears an area, then prints a string into it, with wrapping. */
void Draw_Wrapped_Font(int x, int y, int w, int h, colour col, char *string, font *f, bool margin);

bool Load_Font(char *filename, font *f);
void Free_Font(font *f);

#endif
