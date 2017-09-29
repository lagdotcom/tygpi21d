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

/* G L O B A L S ///////////////////////////////////////////////////////// */

bool redraw_fp;
pcx_picture current_pic;
bool picture_loaded = false;

pcx_picture *textures = null;
int num_textures = 0;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport UINT32 Image_Size(pcx_picture_ptr image)
{
	UINT32 width = image->header.width;
	UINT32 height = image->header.height;
	width++;
	height++;

	return width * height;
}

noexport bool Load_Picture(char *filename, pcx_picture_ptr image, char *tag)
{
	FILE *fp;
	int num_bytes, index;
	UINT32 count, size;
	unsigned char data;
	char far *header, *write;

	fp = fopen(filename, "rb");
	if (!fp) {
		printf("Could not open for reading: %s\n", filename);
		return false;
	}

	header = (char far*)image;
	for (index = 0; index < 128; index++) {
		header[index] = (char)getc(fp);
	}

	size = Image_Size(image);
	image->buffer = Allocate(size, 1, tag);
	if (image->buffer == null) {
		fclose(fp);
		printf("Could not allocate picture buffer.\n");
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

noexport void Draw_Tile_Segment(colour textureId, int dx, int dy, int w, int h, char piece, int sx, int sy)
{
	pcx_picture_ptr texture;
	unsigned char *output;
	char PtrDist *input;
	int x, y, tex_width;

	if (textureId == 0) return;
	texture = &textures[textureId * TEXTURE_PIECES + piece];
	tex_width = texture->header.width + 1;

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

noexport tile *Get_Offset_Tile(int forward, int left)
{
	coord ax, ay;

	char fx = Get_X_Offset(gSave.header.facing);
	char fy = Get_Y_Offset(gSave.header.facing);
	char lx = fy;
	char ly = -fx;

	ax = gSave.header.x + fx * forward + lx * left;
	ay = gSave.header.y + fy * forward + ly * left;

	if (Is_Coord_Valid(ax, ay)) return TILE(gZone, ax, ay);
	return null;
}

void Clear_FP(void)
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
	direction bwall, lwall, rwall;

	Clear_FP();

	bwall = gSave.header.facing;
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
	}

	t = Get_Offset_Tile(2, -1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 80, 48, 32, 32, TXFR, 106, 10);
		Draw_Tile_Segment(t->ceil, 80, 43, 46, 5, TXFR, 106, 0);
		Draw_Tile_Segment(t->floor, 80, 80, 46, 5, TXFR, 106, 47);
		Draw_Tile_Segment(t->walls[rwall].texture, 112, 43, 16, 42, TXFR, 138, 5);
	}

	t = Get_Offset_Tile(2, 0);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 48, 48, 32, 32, TXFR, 69, 10);
		Draw_Tile_Segment(t->ceil, 44, 43, 40, 5, TXFR, 65, 0);
		Draw_Tile_Segment(t->walls[lwall].texture, 43, 43, 5, 42, TXFR, 64, 5);
		Draw_Tile_Segment(t->floor, 44, 80, 40, 5, TXFR, 65, 47);
		Draw_Tile_Segment(t->walls[rwall].texture, 80, 43, 5, 42, TXFR, 101, 5);
	}

	t = Get_Offset_Tile(1, 1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 0, 43, 43, 42, TXNR, 0, 22);
		Draw_Tile_Segment(t->ceil, 0, 32, 43, 11, TXNR, 0, 0);
		Draw_Tile_Segment(t->floor, 0, 85, 43, 11, TXNR, 0, 75);
	}

	t = Get_Offset_Tile(1, -1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 85, 43, 43, 42, TXNR, 107, 22);
		Draw_Tile_Segment(t->ceil, 85, 32, 43, 11, TXNR, 107, 0);
		Draw_Tile_Segment(t->floor, 85, 85, 43, 11, TXNR, 107, 75);
	}

	t = Get_Offset_Tile(1, 0);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 43, 43, 42, 42, TXNR, 54, 22);
		Draw_Tile_Segment(t->ceil, 33, 32, 62, 11, TXNR, 44, 0);
		Draw_Tile_Segment(t->walls[lwall].texture, 32, 32, 11, 64, TXNR, 43, 11);
		Draw_Tile_Segment(t->floor, 33, 85, 62, 11, TXNR, 44, 75);
		Draw_Tile_Segment(t->walls[rwall].texture, 85, 32, 11, 64, TXNR, 96, 11);
	}

	t = Get_Offset_Tile(0, 1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 0, 32, 32, 64, TXSI, 0, 32);
		Draw_Tile_Segment(t->ceil, 0, 0, 32, 32, TXSI, 0, 0);
		Draw_Tile_Segment(t->floor, 0, 96, 32, 32, TXSI, 0, 96);
	}

	t = Get_Offset_Tile(0, -1);
	if (t != null) {
		Draw_Tile_Segment(t->walls[bwall].texture, 96, 32, 32, 64, TXSI, 32, 32);
		Draw_Tile_Segment(t->ceil, 96, 0, 32, 32, TXSI, 32, 0);
		Draw_Tile_Segment(t->floor, 96, 96, 32, 32, TXSI, 32, 96);
	}

	t = Get_Offset_Tile(0, 0);
	if (t != null) {
		/* if this is null, something is really wrong. */
		Draw_Tile_Segment(t->walls[bwall].texture, 32, 32, 64, 64, TXIN, 32, 64);
		Draw_Tile_Segment(t->ceil, 1, 0, 126, 32, TXIN, 1, 0);
		Draw_Tile_Segment(t->walls[lwall].texture, 0, 0, 32, 128, TXIN, 0, 32);
		Draw_Tile_Segment(t->floor, 1, 96, 126, 32, TXIN, 1, 160);
		Draw_Tile_Segment(t->walls[rwall].texture, 96, 0, 32, 128, TXIN, 96, 32);
	}

	redraw_fp = false;
}

void Delete_Picture(void)
{
	if (picture_loaded) {
		picture_loaded = false;
		Free(current_pic.buffer);
	}
}

void Show_Picture(char *name)
{
	int y;
	unsigned char *output;
	char *input;
	char filename[20];
	sprintf(filename, "PICS\\%s.PCX", name);

	Delete_Picture();
	
	if (Load_Picture(filename, &current_pic, "Show_Picture")) {
		/* draw that thing */
		output = &double_buffer[8 * SCREEN_WIDTH + 96];
		input = current_pic.buffer;
		for (y = 0; y < 128; y++) {
			memcpy(output, input, 128);
			input += 128;
			output += SCREEN_WIDTH;
		}

		picture_loaded = true;
	}
}

void Free_Textures(void)
{
	int i;

	if (num_textures > 0) {
		for (i = 0; i < num_textures * TEXTURE_PIECES; i++)
			Free(textures[i].buffer);
		Free(textures);

		num_textures = 0;
	}
}

noexport void Load_Texture_Pieces(char *name, int index)
{
	int i;
	char filename[20];

	for (i = 0; i < TEXTURE_PIECES; i++) {
		sprintf(filename, "WALL\\%s%d.PCX", name, i + 1);
		if (!Load_Picture(filename, &textures[index * TEXTURE_PIECES + i], name)) {
			printf("Could not load: %s\n", filename);
			exit(1);
		}
	}
}

void Load_Textures(zone *z)
{
	int i;
	Free_Textures();

	/* TODO: use z->textures */
	num_textures = z->header.num_textures;
	textures = SzAlloc(num_textures * TEXTURE_PIECES, pcx_picture, "Load_Textures");
	for (i = 0; i < num_textures; i++)
		Load_Texture_Pieces(z->textures[i], i);
}
