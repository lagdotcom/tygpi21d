/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "features.h"
#include "gamelib.h"
#include "gfx.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */ 

#define SX 96
#define SY 8

#define MAX_FILENAME_LENGTH	20

#define FP_IMSZ		0

#define TEX_000W	0
#define TEX_000C	1
#define TEX_000R	2
#define TEX_000F	3
#define TEX_000L	4
#define TEX_0L1W	5
#define TEX_0L1C	6
#define TEX_0L1F	7
#define TEX_0R1W	8
#define TEX_0R1C	9
#define TEX_0R1F	10
#define TEX_100W	11
#define TEX_100C	12
#define TEX_100R	13
#define TEX_100F	14
#define TEX_100L	15
#define TEX_1L1W	16
#define TEX_1L1C	17
#define TEX_1L1F	18
#define TEX_1R1W	19
#define TEX_1R1C	20
#define TEX_1R1F	21
#define TEX_200W	22
#define TEX_200C	23
#define TEX_200R	24
#define TEX_200F	25
#define TEX_200L	26
#define TEX_2L1W	27
#define TEX_2L1C	28
#define TEX_2L1F	29
#define TEX_2L1L	30
#define TEX_2R1W	31
#define TEX_2R1C	32
#define TEX_2R1R	33
#define TEX_2R1F	34
#define TEX_2L2W	35
#define TEX_2L2C	36
#define TEX_2L2F	37
#define TEX_2R2W	38
#define TEX_2R2C	39
#define TEX_2R2F	40
#define TEXTURE_PIECES (TEX_2R2F + 1)

#define THING_CLOSE	0
#define THING_MID	1
#define THING_FAR	2
#define THING_PIECES (THING_FAR + 1)

/* G L O B A L S ///////////////////////////////////////////////////////// */

bool redraw_fp;
noexport pcx_picture current_pic;
noexport bool picture_loaded = false;
noexport char shown_picture[MAX_FILENAME_LENGTH];

#define VIEW_X	96
#define VIEW_Y	8
#define VIEW_W	128
#define VIEW_H	128

noexport box2d viewport = {
	{ VIEW_X, VIEW_Y },
	{ VIEW_X + VIEW_W, VIEW_Y + VIEW_H },
};

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

noexport void Draw_Tile_Segment(file_id texture_id, int dx, int dy, int piece)
{
#if FEATURE_TEXTURES
	grf *tex;
	point2d p;
	assert(piece < TEXTURE_PIECES, "Draw_Tile_Segment: piece number too high");
	
	if (texture_id == 0) return;
	tex = Lookup_File(gDjn, texture_id, true);
	assert(tex != null, "Draw_Tile_Segment: unknown texture ID");

	p.x = dx + 96;
	p.y = dy + 8;
	Draw_GRF_Clipped(&p, tex, piece, 0, &viewport);
#endif
}

noexport void Draw_Thing(file_id thing_id, int dx, int dy, int piece)
{
#if FEATURE_THINGS
	grf *thing;
	point2d p;
	assert(piece < THING_PIECES, "Draw_Thing: piece number too high");

	if (thing_id == 0) return;
	thing = Lookup_File(gDjn, thing_id, true);
	assert(thing != null, "Draw_Thing: unknown thing ID");

	p.x = dx + 96;
	p.y = dy + 8;
	Draw_GRF_Clipped(&p, thing, piece, 0, &viewport);
#endif
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
		Draw_Tile_Segment(t->walls[bwall].texture, 0, 48, TEX_2L2W);
		Draw_Tile_Segment(t->ceil, 0, 43, TEX_2L2C);
		Draw_Tile_Segment(t->floor, 0, 80, TEX_2L2F);
	}

	t = Get_Offset_Tile(2, -2);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 112, 48, TEX_2R2W);
		Draw_Tile_Segment(t->ceil, 114, 43, TEX_2R2C);
		Draw_Tile_Segment(t->floor, 114, 80, TEX_2R2F);
	}

	t = Get_Offset_Tile(2, 1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 16, 48, TEX_2L1W);
		Draw_Tile_Segment(t->walls[lwall].texture, 0, 43, TEX_2L1L);
		Draw_Tile_Segment(t->ceil, 2, 43, TEX_2L1C);
		Draw_Tile_Segment(t->floor, 2, 80, TEX_2L1F);

		Draw_Thing(t->thing, 6, 43, THING_FAR);
	}

	t = Get_Offset_Tile(2, -1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 80, 48, TEX_2R1W);
		Draw_Tile_Segment(t->ceil, 80, 43, TEX_2R1C);
		Draw_Tile_Segment(t->floor, 80, 80, TEX_2R1F);
		Draw_Tile_Segment(t->walls[rwall].texture, 112, 43, TEX_2R1R);

		Draw_Thing(t->thing, 86, 43, THING_FAR);
	}

	t = Get_Offset_Tile(2, 0);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 48, 48, TEX_200W);
		Draw_Tile_Segment(t->ceil, 44, 43, TEX_200C);
		Draw_Tile_Segment(t->walls[lwall].texture, 43, 43, TEX_200L);
		Draw_Tile_Segment(t->floor, 44, 80, TEX_200F);
		Draw_Tile_Segment(t->walls[rwall].texture, 80, 43, TEX_200R);

		Draw_Thing(t->thing, 43, 43, THING_FAR);
	}

	t = Get_Offset_Tile(1, 1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 0, 43, TEX_1L1W);
		Draw_Tile_Segment(t->ceil, 0, 32, TEX_1L1C);
		Draw_Tile_Segment(t->floor, 0, 85, TEX_1L1F);

		Draw_Thing(t->thing, -32, 32, THING_MID);
	}

	t = Get_Offset_Tile(1, -1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 85, 43, TEX_1R1W);
		Draw_Tile_Segment(t->ceil, 85, 32, TEX_1R1C);
		Draw_Tile_Segment(t->floor, 85, 85, TEX_1R1F);

		Draw_Thing(t->thing, 96, 32, THING_MID);
	}

	t = Get_Offset_Tile(1, 0);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 43, 43, TEX_100W);
		Draw_Tile_Segment(t->ceil, 33, 32, TEX_100C);
		Draw_Tile_Segment(t->walls[lwall].texture, 32, 32, TEX_100L);
		Draw_Tile_Segment(t->floor, 33, 85, TEX_100F);
		Draw_Tile_Segment(t->walls[rwall].texture, 85, 32, TEX_100R);

		Draw_Thing(t->thing, 32, 32, THING_MID);
	}

	t = Get_Offset_Tile(0, 1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 0, 32, TEX_0L1W);
		Draw_Tile_Segment(t->ceil, 0, 0, TEX_0L1C);
		Draw_Tile_Segment(t->floor, 0, 96, TEX_0L1F);

		Draw_Thing(t->thing, -96, 0, THING_CLOSE);
	}

	t = Get_Offset_Tile(0, -1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 96, 32, TEX_0R1W);
		Draw_Tile_Segment(t->ceil, 96, 0, TEX_0R1C);
		Draw_Tile_Segment(t->floor, 96, 96, TEX_0R1F);

		Draw_Thing(t->thing, 96, 0, THING_CLOSE);
	}

	t = Get_Offset_Tile(0, 0);
	if (t != null) {
		/* if this is null, something is really wrong. */
		Draw_Tile_Segment(t->walls[bwall].texture, 32, 32, TEX_000W);
		Draw_Tile_Segment(t->ceil, 1, 0, TEX_000C);
		Draw_Tile_Segment(t->walls[lwall].texture, 0, 0, TEX_000L);
		Draw_Tile_Segment(t->floor, 1, 96, TEX_000F);
		Draw_Tile_Segment(t->walls[rwall].texture, 96, 0, TEX_000R);

		Draw_Thing(t->thing, 0, 0, THING_CLOSE);
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

/* TODO: use file_id instead / GRF */
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
