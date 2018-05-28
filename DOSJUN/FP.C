/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "gamelib.h"
#include "gfx.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */ 

#define SX 96
#define SY 8

#define TEXTURE_PIECES 4
#define TXIN	0
#define TXSI	1
#define TXNR	2
#define TXFR	3

#define MAX_FILENAME_LENGTH	20

#define FP_IMSZ		0

/* G L O B A L S ///////////////////////////////////////////////////////// */

bool redraw_fp;
noexport pcx_picture current_pic;
noexport bool picture_loaded = false;
noexport char shown_picture[MAX_FILENAME_LENGTH];

noexport pcx_picture *textures = null;
noexport int num_textures = 0;

noexport pcx_picture *things = null;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport UINT32 Image_Size(pcx_picture_ptr image)
{
	UINT32 width = image->header.width;
	UINT32 height = image->header.height;
	width++;
	height++;

	return width * height;
}

bool Load_Picture(char *filename, pcx_picture_ptr image, char *tag)
{
	FILE *fp;
	int num_bytes, index;
	UINT32 count, size;
	unsigned char data;
	char far *header, *write;

	fp = fopen(filename, "rb");
	if (!fp) {
		dief("Load_Picture: Could not open for reading: %s\n", filename);
		return false;
	}

	Log("Load_Picture: %s", filename);

	header = (char far*)image;
	for (index = 0; index < 128; index++) {
		header[index] = (char)getc(fp);
	}

	size = Image_Size(image);
#if FP_IMSZ
	image->buffer = Allocate(size, 1, tag);
#else
	image->buffer = Allocate(image->header.width + 1, image->header.height + 1, tag);
#endif
	if (image->buffer == null) {
		fclose(fp);
		dief("Load_Picture: Could not allocate picture buffer.\n");
		return false;
	}

	count = 0;
	write = image->buffer;
	while (count < size) {
		data = (unsigned char)getc(fp);

		if (data >= 192) {
			num_bytes = data - 192;
			data = (unsigned char)getc(fp);
			while (num_bytes-- > 0) {
				*write = data;
				count++;
				write++;
			}
		} else {
			*write = data;
			count++;
			write++;
		}
	}

	fclose(fp);
	return true;
}

noexport void Paste_DB(int dx, int dy, pcx_picture *texture, int w, int h, int sx, int sy)
{
	unsigned char *output;
	char PtrDist *input;
	int x, y, tex_width;
	assert((dx + w) <= SCREEN_WIDTH, "Paste_DB: dx + w > width");
	assert((dy + h) <= SCREEN_HEIGHT, "Paste_DB: dy + h > height");

	tex_width = texture->header.width + 1;
	assert((sx + w) <= tex_width, "Paste_DB: sx + w > img.width");
	assert((sy + h) <= (texture->header.height + 1), "Paste_DB: sy + h > img.height");

	output = &double_buffer[(dy + SY) * SCREEN_WIDTH + (dx + SX)];
	input = &texture->buffer[sy * tex_width + sx];

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (*input != 0)
				*output = *input;

			output++;
			input++;
		}

		output += SCREEN_WIDTH - w;
		input += tex_width - w;
	}
}

noexport void Draw_Tile_Segment(colour textureId, int dx, int dy, int w, int h, char piece, int sx, int sy)
{
	int pieceStart;
	assert(piece < TEXTURE_PIECES, "Draw_Tile_Segment: piece number too high");
	assert(textureId <= num_textures, "Draw_Tile_Segment: texture number too high");
	
	if (textureId == 0) return;

	pieceStart = (textureId - 1) * TEXTURE_PIECES;
	Paste_DB(dx, dy, &textures[pieceStart + piece], w, h, sx, sy);
}

noexport void Draw_Thing(thing_id th, int dx, int dy, int w, int h, int sx, int sy)
{
	assert(th < thINVALID, "Draw_Thing: thing number too high");

	if (th == 0) return;

	Paste_DB(dx, dy, &things[th - 1], w, h, sx, sy);
}

noexport tile *Get_Offset_Tile(int forward, int left)
{
	coord ax, ay;

	char fx = Get_X_Offset(gParty->facing);
	char fy = Get_Y_Offset(gParty->facing);
	char lx = fy;
	char ly = -fx;

	ax = gParty->x + fx * forward + lx * left;
	ay = gParty->y + fy * forward + ly * left;

	if (Is_Coord_Valid(ax, ay)) return TILE(gZone, ax, ay);
	return null;
}

noexport void Load_Texture_Pieces(char *name, int index)
{
	int i;
	char filename[MAX_FILENAME_LENGTH];

	for (i = 0; i < TEXTURE_PIECES; i++) {
		sprintf(filename, "WALL\\%s%d.PCX", name, i + 1);
		if (!Load_Picture(filename, &textures[index * TEXTURE_PIECES + i], "Load_Texture_Pieces.n"))
			dief("Load_Texture_Pieces: Could not load: %s\n", filename);
	}
}

noexport void Load_Thing(char *name, thing_id th)
{
	char filename[MAX_FILENAME_LENGTH];

	sprintf(filename, "THING\\%s.PCX", name);
	if (!Load_Picture(filename, &things[th - 1], name))
		dief("Load_Thing: Could not load: %s\n", filename);
}

noexport void Load_Things(void)
{
	if (things != null) return;

	things = SzAlloc(thINVALID - 1, pcx_picture, "Load_Things");
	Load_Thing("SHINY", thShiny);
	Load_Thing("BARREL", thBarrel);
}

noexport void Free_Things(void)
{
	int i = 0;

	Log("Free_Things: %p", things);

	if (things == null) return;

	for (i = 0; i < thINVALID - 1; i++)
		Free(things[i].buffer);

	Free(things);
}

/* M A I N /////////////////////////////////////////////////////////////// */

noexport void Clear_FP(void)
{
	int y;
	unsigned char *output = &double_buffer[8 * SCREEN_WIDTH + 96];

	for (y = 0; y < 128; y++) {
		memset(output, 0, 128);
		output += SCREEN_WIDTH;
	}
}

void Draw_FP(void)
{
	tile *t;
	dir bwall, lwall, rwall;

	Clear_FP();

	bwall = gParty->facing;
	lwall = Turn_Left(bwall);
	rwall = Turn_Right(bwall);

	t = Get_Offset_Tile(2, 2);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 0, 48, 16, 32, TXFR, 0, 10);
		Draw_Tile_Segment(t->ceil, 0, 43, 14, 5, TXFR, 0, 0);
		Draw_Tile_Segment(t->floor, 0, 80, 14, 5, TXFR, 0, 47);
	}

	t = Get_Offset_Tile(2, -2);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 112, 48, 16, 32, TXFR, 154, 10);
		Draw_Tile_Segment(t->ceil, 114, 43, 14, 5, TXFR, 156, 0);
		Draw_Tile_Segment(t->floor, 114, 80, 14, 5, TXFR, 156, 47);
	}

	t = Get_Offset_Tile(2, 1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 16, 48, 32, 32, TXFR, 32, 10);
		Draw_Tile_Segment(t->walls[lwall].texture, 0, 43, 16, 42, TXFR, 16, 5);
		Draw_Tile_Segment(t->ceil, 2, 43, 46, 5, TXFR, 18, 0);
		Draw_Tile_Segment(t->floor, 2, 80, 46, 5, TXFR, 18, 47);

		Draw_Thing(t->thing, 6, 43, 42, 42, 150, 86);
	}

	t = Get_Offset_Tile(2, -1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 80, 48, 32, 32, TXFR, 106, 10);
		Draw_Tile_Segment(t->ceil, 80, 43, 46, 5, TXFR, 106, 0);
		Draw_Tile_Segment(t->floor, 80, 80, 46, 5, TXFR, 106, 47);
		Draw_Tile_Segment(t->walls[rwall].texture, 112, 43, 16, 42, TXFR, 138, 5);

		Draw_Thing(t->thing, 86, 43, 42, 42, 150, 86);
	}

	t = Get_Offset_Tile(2, 0);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 48, 48, 32, 32, TXFR, 69, 10);
		Draw_Tile_Segment(t->ceil, 44, 43, 40, 5, TXFR, 65, 0);
		Draw_Tile_Segment(t->walls[lwall].texture, 43, 43, 5, 42, TXFR, 64, 5);
		Draw_Tile_Segment(t->floor, 44, 80, 40, 5, TXFR, 65, 47);
		Draw_Tile_Segment(t->walls[rwall].texture, 80, 43, 5, 42, TXFR, 101, 5);

		Draw_Thing(t->thing, 43, 43, 42, 42, 150, 86);
	}

	t = Get_Offset_Tile(1, 1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 0, 43, 43, 42, TXNR, 0, 22);
		Draw_Tile_Segment(t->ceil, 0, 32, 43, 11, TXNR, 0, 0);
		Draw_Tile_Segment(t->floor, 0, 85, 43, 11, TXNR, 0, 75);

		Draw_Thing(t->thing, 0, 32, 32, 64, 160, 0);
	}

	t = Get_Offset_Tile(1, -1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 85, 43, 43, 42, TXNR, 107, 22);
		Draw_Tile_Segment(t->ceil, 85, 32, 43, 11, TXNR, 107, 0);
		Draw_Tile_Segment(t->floor, 85, 85, 43, 11, TXNR, 107, 75);

		Draw_Thing(t->thing, 96, 32, 32, 64, 128, 0);
	}

	t = Get_Offset_Tile(1, 0);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 43, 43, 42, 42, TXNR, 54, 22);
		Draw_Tile_Segment(t->ceil, 33, 32, 62, 11, TXNR, 44, 0);
		Draw_Tile_Segment(t->walls[lwall].texture, 32, 32, 11, 64, TXNR, 43, 11);
		Draw_Tile_Segment(t->floor, 33, 85, 62, 11, TXNR, 44, 75);
		Draw_Tile_Segment(t->walls[rwall].texture, 85, 32, 11, 64, TXNR, 96, 11);

		Draw_Thing(t->thing, 32, 32, 64, 64, 128, 0);
	}

	t = Get_Offset_Tile(0, 1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 0, 32, 32, 64, TXSI, 0, 32);
		Draw_Tile_Segment(t->ceil, 0, 0, 32, 32, TXSI, 0, 0);
		Draw_Tile_Segment(t->floor, 0, 96, 32, 32, TXSI, 0, 96);

		Draw_Thing(t->thing, 0, 0, 32, 128, 96, 0);
	}

	t = Get_Offset_Tile(0, -1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 96, 32, 32, 64, TXSI, 32, 32);
		Draw_Tile_Segment(t->ceil, 96, 0, 32, 32, TXSI, 32, 0);
		Draw_Tile_Segment(t->floor, 96, 96, 32, 32, TXSI, 32, 96);

		Draw_Thing(t->thing, 96, 0, 32, 128, 0, 0);
	}

	t = Get_Offset_Tile(0, 0);
	if (t != null) {
		/* if this is null, something is really wrong. */
		Draw_Tile_Segment(t->walls[bwall].texture, 32, 32, 64, 64, TXIN, 32, 64);
		Draw_Tile_Segment(t->ceil, 1, 0, 126, 32, TXIN, 1, 0);
		Draw_Tile_Segment(t->walls[lwall].texture, 0, 0, 32, 128, TXIN, 0, 32);
		Draw_Tile_Segment(t->floor, 1, 96, 126, 32, TXIN, 1, 160);
		Draw_Tile_Segment(t->walls[rwall].texture, 96, 0, 32, 128, TXIN, 96, 32);

		Draw_Thing(t->thing, 0, 0, 128, 128, 0, 0);
	}

	redraw_fp = false;
}

void Delete_Picture(void)
{
	Log("Delete_Picture: %p", &current_pic);

	if (picture_loaded) {
		picture_loaded = false;
		shown_picture[0] = 0;
		Free(current_pic.buffer);
	}
}

void Show_Picture(char *name)
{
	int y;
	unsigned char *output;
	char *input;
	char filename[MAX_FILENAME_LENGTH];

	sprintf(filename, "PICS\\%s.PCX", name);

	/* don't bother loading the picture if it's already loaded */
	if (!picture_loaded || !streq(filename, shown_picture)) {
		Delete_Picture();

		if (!Load_Picture(filename, &current_pic, name)) {
			die("Show_Picture: could not load");
		}

		picture_loaded = true;
		strcpy(shown_picture, filename);
	}

	/* copy picture to screen */
	output = &double_buffer[8 * SCREEN_WIDTH + 96];
	input = current_pic.buffer;
	for (y = 0; y < 128; y++) {
		memcpy(output, input, 128);
		input += 128;
		output += SCREEN_WIDTH;
	}
}
