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

bool redraw_fp = false;
noexport file_id current_img_id = 0;
noexport grf *current_img = null;

#define VIEW_X	96
#define VIEW_Y	8
#define VIEW_W	128
#define VIEW_H	128

noexport box2d viewport = {
	{ VIEW_X, VIEW_Y },
	{ VIEW_X + VIEW_W, VIEW_Y + VIEW_H },
};

noexport void *fp_buffer;

int current_fp_effect = 0;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Save_FP(void)
{
	int y;
	unsigned char *buffer = fp_buffer;
	unsigned char *source = &double_buffer[VIEW_Y * SCREEN_WIDTH + VIEW_X];

	for (y = 0; y < VIEW_H; y++) {
		memcpy(buffer, source, VIEW_W);
		buffer += VIEW_W;
		source += SCREEN_WIDTH;
	}
}

noexport void Restore_FP(void)
{
	int y;
	unsigned char *buffer = fp_buffer;
	unsigned char *source = &double_buffer[VIEW_Y * SCREEN_WIDTH + VIEW_X];

	for (y = 0; y < VIEW_H; y++) {
		memcpy(source, buffer, VIEW_W);
		buffer += VIEW_W;
		source += SCREEN_WIDTH;
	}
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

	p.x = dx + VIEW_X;
	p.y = dy + VIEW_Y;
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

	p.x = dx + VIEW_X;
	p.y = dy + VIEW_Y;
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
	unsigned char *output = &double_buffer[VIEW_Y * SCREEN_WIDTH + VIEW_X];

	for (y = 0; y < VIEW_H; y++) {
		memset(output, 0, VIEW_W);
		output += SCREEN_WIDTH;
	}
}

noexport void Draw_Items_in_Tile(coord x, coord y, int distance)
{
	int i;
	itempos *ip;
	item *it;
	grf *g;
	point2d p;

	for (i = 0; i < gOverlay->items->size; i++) {
		ip = List_At(gOverlay->items, i);

		if (ip->x != x || ip->y != y || !ip->item)
			continue;

		it = Lookup_File(gDjn, ip->item, true);
		if (it == null || !it->image_id)
			continue;

		g = Lookup_File(gDjn, it->image_id, true);
		if (g == null || g->num_images <= distance)
			continue;

		p.x = VIEW_X;
		p.y = VIEW_Y + VIEW_H - 32;

		p.x += ip->tile_x * VIEW_W / 100;
		p.y += ip->tile_y * 32 / 100;

		p.x -= g->images[distance].width / 2;
		p.y -= g->images[distance].height / 2;

		Draw_GRF_Clipped(&p, g, distance, 0, &viewport);
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

	Draw_Items_in_Tile(gParty->x, gParty->y, 0);

	redraw_fp = false;
	Save_FP();
}

void Show_Picture(file_id ref)
{
	/* don't bother loading the picture if it's already loaded */
	if (current_img_id != ref) {
		current_img_id = ref;
		current_img = Lookup_File_Chained(gDjn, ref);

		if (current_img == null)
			dief("Show_Picture: %d not found", ref);
	}

	/* copy picture to screen */
	Draw_GRF_Clipped(&viewport.start, current_img, 0, 0, &viewport);
}

void Initialise_FP(void)
{
	fp_buffer = Allocate(VIEW_W * VIEW_H, 1, "Initialise_FP");
}

void Free_FP(void)
{
	Free(fp_buffer);
}

#define NUM_DROPS	15
typedef struct raindrop {
	int x;
	int y;
	UINT8 v;
} raindrop;

noexport void Reset_Drop(raindrop *d, bool init)
{
	d->x = randint(0, VIEW_W - 1);
	d->y = randint(init ? - 400 : -VIEW_H, 0);
	d->v = 0;
}

noexport void Water_Effect(void)
{
	static bool init = false;
	static raindrop drops[NUM_DROPS];
	static int timer;
	raindrop *d = drops;
	int i;

	if (!init) {
		for (i = 0, d = drops; i < NUM_DROPS; i++, d++) {
			Reset_Drop(d, true);
		}

		init = true;
		timer = 0;
	}

	if (timer) {
		timer--;
		return;
	}

	Restore_FP();
	for (i = 0, d = drops; i < NUM_DROPS; i++, d++) {
		d->y += d->v / 20;
		d->v += 3;

		if (d->y >= VIEW_H) {
			Reset_Drop(d, false);
		} else if (d->y > 2) {
			Plot_Pixel_Fast_DB(d->x + VIEW_X, d->y + VIEW_Y, 6);
			Plot_Pixel_Fast_DB(d->x + VIEW_X, d->y + VIEW_Y - 1, 22);
			Plot_Pixel_Fast_DB(d->x + VIEW_X, d->y + VIEW_Y - 2, 38);
			Plot_Pixel_Fast_DB(d->x + VIEW_X, d->y + VIEW_Y - 3, 54);
		}
	}
	Show_Double_Buffer();

	timer = 8;
}

void Continue_Effect(void)
{
	switch (current_fp_effect) {
		case FX_WATER:
			Water_Effect();
			break;
	}
}
