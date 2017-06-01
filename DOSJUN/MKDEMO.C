/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>
#include "dosjun.h"
#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define TSET(ix, c, f, wn, we, ws, ww, d) { \
	z.tiles[ix].ceil = c; \
	z.tiles[ix].floor = f; \
	z.tiles[ix].walls[North].texture = wn; \
	z.tiles[ix].walls[East].texture = we; \
	z.tiles[ix].walls[South].texture = ws; \
	z.tiles[ix].walls[West].texture = ww; \
	z.tiles[ix].description = d + 1; \
}

#define ISET(ix, x, n, t, f, v) { \
	strncpy(i.items[ix].name, n, NAME_SIZE - 1); \
	i.items[ix].id = x; \
	i.items[ix].type = t; \
	i.items[ix].flags = f; \
	i.items[ix].value = v; \
}

#define SSET(ix, st, sv) i.items[ix].stats[st] = sv

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

void Demo_Items(char *filename)
{
	items i;

	i.header.num_items = 8;
	Zero(i.header.unused, ITEMS_HEADER_PADDING);

	i.items = calloc(8, sizeof(item));

	ISET( 0, 0x100, "Longsword", PrimaryWeapon, IT_HEAVY, 100);
	SSET( 0, MinDamage, 4);
	SSET( 0, MaxDamage, 8);

	ISET( 1, 0x101, "Dagger", SmallWeapon, IT_LIGHT, 10);
	SSET( 1, MinDamage, 1);
	SSET( 1, MaxDamage, 2);

	ISET( 2, 0x200, "Buckler", Shield, 0, 15);
	SSET( 2, Armour, 1);

	ISET( 3, 0x300, "Leather Cap", Helmet, IT_LIGHT, 25);
	SSET( 3, Armour, 2);	

	ISET( 4, 0x400, "Platemail", BodyArmour, IT_HEAVY, 25000);
	SSET( 4, Armour, 13);

	ISET( 5, 0x500, "Spiked Boots", Footwear, IT_LIGHT, 150);
	SSET( 5, Armour, 2);
	SSET( 5, MaxDamage, 1);

	ISET( 6, 0x600, "Moonstone Pendant", Jewellery, IT_LIGHT, 2000);
	SSET( 6, Intelligence, 1);

	ISET( 7, 0x601, "Cloudy Tanzanite Ring", Jewellery, IT_LIGHT, 1450);
	SSET( 7, Strength, 2);
	SSET( 7, Dexterity, -1);

	Items_Save(filename, &i);
	printf("Wrote %s\n", filename);

	free(i.items);
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
	Demo_Campaign("DEMO.CMP");
	Demo_Items("DEMO.ITM");
	Demo_Zone("DEMO.ZON");
}
