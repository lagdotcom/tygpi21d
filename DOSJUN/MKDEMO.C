/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>
#include "dosjun.h"
#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define RAT 0x001

#define Set_Tile(ix, c, f, wn, we, ws, ww, d) { \
	z.tiles[ix].ceil = c; \
	z.tiles[ix].floor = f; \
	z.tiles[ix].walls[North].texture = wn; \
	z.tiles[ix].walls[East].texture = we; \
	z.tiles[ix].walls[South].texture = ws; \
	z.tiles[ix].walls[West].texture = ww; \
	z.tiles[ix].description = d + 1; \
}

#define Set_Item(ix, x, n, t, f, v) { \
	strncpy(i.items[ix].name, n, NAME_SIZE - 1); \
	i.items[ix].id = x; \
	i.items[ix].type = t; \
	i.items[ix].flags = f; \
	i.items[ix].value = v; \
}

#define Set_Monster(ix, x, n) { \
	strncpy(m.monsters[ix].name, n, NAME_SIZE - 1); \
	m.monsters[ix].id = x; \
}

#define Set_ItemStat(ix, st, sv) i.items[ix].stats[st] = sv
#define Set_MonsterStat(ix, st, sv) m.monsters[ix].stats[st] = sv

#define Set_Encounter(ix, slot, mon, min, max) { \
	z.encounters[ix].monsters[slot] = mon; \
	z.encounters[ix].minimum[slot] = min; \
	z.encounters[ix].maximum[slot] = max; \
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

	c.zones = SzAlloc(1, char *, "Demo_Campaign");
	c.zones[0] = "DEMO";

	Save_Campaign(filename, &c);
	printf("Wrote %s\n", filename);

	Free(c.zones);
}

void Demo_Items(char *filename)
{
	items i;

	i.header.num_items = 8;
	Zero(i.header.unused, ITEMS_HEADER_PADDING);

	i.items = SzAlloc(8, item, "Demo_Items");

	Set_Item( 0, 0x100, "Longsword", itPrimaryWeapon, ifHeavy, 100);
	Set_ItemStat( 0, sMinDamage, 4);
	Set_ItemStat( 0, sMaxDamage, 8);

	Set_Item( 1, 0x101, "Dagger", itSmallWeapon, ifLight, 10);
	Set_ItemStat( 1, sMinDamage, 1);
	Set_ItemStat( 1, sMaxDamage, 2);

	Set_Item( 2, 0x200, "Buckler", itShield, 0, 15);
	Set_ItemStat( 2, sArmour, 1);

	Set_Item( 3, 0x300, "Leather Cap", itHelmet, ifLight, 25);
	Set_ItemStat( 3, sArmour, 2);	

	Set_Item( 4, 0x400, "Platemail", itBodyArmour, ifHeavy, 25000);
	Set_ItemStat( 4, sArmour, 13);

	Set_Item( 5, 0x500, "Spiked Boots", itFootwear, ifLight, 150);
	Set_ItemStat( 5, sArmour, 2);
	Set_ItemStat( 5, sMaxDamage, 1);

	Set_Item( 6, 0x600, "Moonstone Pendant", itJewellery, ifLight, 2000);
	Set_ItemStat( 6, sIntelligence, 1);

	Set_Item( 7, 0x601, "Cloudy Tanzanite Ring", itJewellery, ifLight, 1450);
	Set_ItemStat( 7, sStrength, 2);
	Set_ItemStat( 7, sDexterity, -1);

	Save_Items(filename, &i);
	printf("Wrote %s\n", filename);

	Free(i.items);
}

void Demo_Monsters(char *filename)
{
	monsters m;

	m.header.num_monsters = 1;
	Zero(m.header.unused, MONSTERS_HEADER_PADDING);

	m.monsters = SzAlloc(1, monster, "Demo_Monsters");

	Set_Monster(0, RAT, "Large Rat");
	Set_MonsterStat(0, sMaxHP, 3);
	Set_MonsterStat(0, sMaxMP, 0);
	Set_MonsterStat(0, sMinDamage, 1);
	Set_MonsterStat(0, sMaxDamage, 1);
	Set_MonsterStat(0, sArmour, 0);
	Set_MonsterStat(0, sStrength, 2);
	Set_MonsterStat(0, sDexterity, 5);
	Set_MonsterStat(0, sIntelligence, 1);

	Save_Monsters(filename, &m);
	printf("Wrote %s\n", filename);

	Free(m.monsters);
}

void Demo_Zone(char *filename)
{
	zone z;
	Initialise_Zone(&z);
	Zero(z.header.unused, ZONE_HEADER_PADDING);

	strcpy(z.header.campaign_name, "DEMO");
	z.header.width = 10;
	z.header.height = 10;
	z.tiles = SzAlloc(10 * 10, tile, "Demo_Zone.tiles");
	/*        I   C   F  WN  WE  WS  WW  S */
	Set_Tile( 0, 10,  9, 11,  0, 11, 11, 0);
	Set_Tile( 1, 10,  9, 11,  0, 11,  0, 0);
	Set_Tile( 2, 10,  9, 11,  0,  0,  0, 0);
	Set_Tile( 3,  5, 10,  2,  0,  2,  0, 1);
	Set_Tile( 4,  5,  6,  2,  3,  4,  0, 2);
	Set_Tile(12, 10,  9,  0, 15,  0, 15, 0);
	Set_Tile(22, 10,  9,  0, 15, 15,  0, 0);
	Set_Tile(21, 10,  9, 15,  0, 15, 15, 0);

	z.header.num_strings = 3;
	z.strings = SzAlloc(3, char *, "Demo_Zone.strings");
	z.strings[0] = "Other than its ridiculous colour,\nthere's nothing particularly\ninteresting about this corridor.";
	z.strings[1] = "New text.";
	z.strings[2] = "Another line of text!\nOne day this will be a REAL GAME.";

	z.header.num_scripts = 0;
	z.scripts = null;
	z.script_lengths = null;

	z.header.num_encounters = 1;
	z.encounters = SzAlloc(1, encounter, "Demo_Zone.encounters");
	Set_Encounter( 0, 0, RAT, 1, 3);

	Save_Zone(filename, &z);
	printf("Wrote %s\n", filename);

	Free_Zone(&z);
}

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	Demo_Campaign("DEMO.CMP");
	Demo_Items("DEMO.ITM");
	Demo_Monsters("DEMO.MON");
	Demo_Zone("DEMO.ZON");

	Stop_Memory_Tracking();
}
