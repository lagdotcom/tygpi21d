/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <alloc.h>
#include "dosjun.h"
#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define TSET(i, c, f, wn, we, ws, ww, d) { \
	z.tiles[i].ceil = c; \
	z.tiles[i].floor = f; \
	z.tiles[i].walls[North].texture = wn; \
	z.tiles[i].walls[East].texture = we; \
	z.tiles[i].walls[South].texture = ws; \
	z.tiles[i].walls[West].texture = ww; \
	z.tiles[i].description = d + 1; \
}

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Zero(unsigned char *padding, int bytes)
{
	memset(padding, 0, bytes);
}

void Demo_Campaign(char *filename)
{
	campaign c;

	c.header.start_zone = 0;
	c.header.start_x = 0;
	c.header.start_y = 0;
	c.header.start_facing = East;
	c.header.num_zones = 1;
	Zero(c.header.unused, CAMPAIGN_HEADER_PADDING);

	c.zones = calloc(1, sizeof(char*));
	c.zones[0] = "DEMO";

	Campaign_Save(filename, &c);
	printf("Wrote %s\n", filename);

	free(c.zones);
}

void Demo_Zone(char *filename)
{
	zone z;

	z.header.width = 10;
	z.header.height = 10;
	z.header.num_strings = 3;
	z.header.num_scripts = 0;
	Zero(z.header.unused, ZONE_HEADER_PADDING);

	z.tiles = calloc(10 * 10, sizeof(tile));
	/*    I   C   F  WN  WE  WS  WW  S */
	TSET( 0, 10,  9, 11,  0, 11, 11, 0);
	TSET( 1, 10,  9, 11,  0, 11,  0, 0);
	TSET( 2, 10,  9, 11,  0,  0,  0, 0);
	TSET( 3,  5, 10,  2,  0,  2,  0, 1);
	TSET( 4,  5,  6,  2,  3,  4,  0, 2);
	TSET(12, 10,  9,  0, 15,  0, 15, 0);
	TSET(22, 10,  9,  0, 15, 15,  0, 0);
	TSET(21, 10,  9, 15,  0, 15, 15, 0);

	z.strings = calloc(3, sizeof(char*));
	z.strings[0] = "Other than its ridiculous colour,\nthere's nothing particularly\ninteresting about this corridor.";
	z.strings[1] = "New text.";
	z.strings[2] = "Another line of text!\nOne day this will be a REAL GAME.";

	z.scripts = null;

	Zone_Save(filename, &z);
	printf("Wrote %s\n", filename);

	free(z.tiles);
	free(z.strings);
}

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	Demo_Campaign("DEMO.CAM");
	Demo_Zone("DEMO.ZON");
}
