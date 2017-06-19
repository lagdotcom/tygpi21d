/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gamelib.h"
#include "dosjun.h"
#include "jc.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MAX_STRING_LIST 1000

typedef enum {
	esWall,
	esDoor,
	esEncounter
} editorstate;

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture explore_bg;
jc_parser parser;
zone gZone;
items gItems;
monsters gMonsters;

char *zone_filename;
bool redraw_details, redraw_zone, clear_screen;
int sel_x, sel_y;
editorstate state;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Input_Multiline_String(int x, int y, char *string, int max);
bool Input_Number(int x, int y, int *number, int min, int max);
void Draw_Bounded_String(int x, int y, int w, int h, colour col, char *string, bool trans_flag);
void Draw_Line_DB(int xo, int yo, int x1, int y1, colour col);
void Draw_Square_DB(colour col, int x0, int y0, int x1, int y1, bool filled);
void Free_Zone(zone *z);
void Initialise_Zone(zone *z);
void Load_Zone(char *filename, zone *z);
monster *Lookup_Monster(monsters *lib, monster_id id);
void Save_Zone(char *filename, zone *z);

/* F U N C T I O N S ///////////////////////////////////////////////////// */

#define TX (8 + x*7)
#define TY (8 + y*7)

void Draw_Tile(coord x, coord y)
{
	tile* t = TILE(gZone, x, y);

	if (x == sel_x && y == sel_y) {
		Draw_Square_DB(15, TX, TY, TX + 6, TY + 6, 0);
	} else {
		Draw_Square_DB(0, TX, TY, TX + 6, TY + 6, 0);
	}

	Draw_Square_DB(t->floor, TX + 1, TY + 1, TX + 5, TY + 5, 1);
	Draw_Line_DB(TX + 2, TY + 0, TX + 4, TY + 0, t->walls[North].texture);
	Draw_Line_DB(TX + 2, TY + 6, TX + 4, TY + 6, t->walls[South].texture);
	Draw_Line_DB(TX + 0, TY + 2, TX + 0, TY + 4, t->walls[West].texture);
	Draw_Line_DB(TX + 6, TY + 2, TX + 6, TY + 4, t->walls[East].texture);
}

void Draw_Zone(void)
{
	int x, y;

	for (x = 0; x < gZone.header.width; x++) {
		for (y = 0; y < gZone.header.height; y++) {
			Draw_Tile(x, y);
		}
	}

	redraw_zone = false;
}

#undef TY
#undef TX

#define DX 250

char Get_Walltype_Char(wall_type type)
{
	switch (type) {
		case wtNormal: return 'W';
		case wtDoor: return 'D';
		case wtLockedDoor: return 'L';
		default: return '?';
	}
}

char Get_Direction_Char(direction dir)
{
	switch (dir) {
		case North: return 'N';
		case East: return 'E';
		case South: return 'S';
		case West: return 'W';
		default: return '?';
	}
}

void Draw_Wall(tile *t, direction dir, int y)
{
	char buf[100];

	sprintf(buf, "%c%c: %4d",
		Get_Walltype_Char(t->walls[dir].type),
		Get_Direction_Char(dir),
		t->walls[dir].texture
	);

	Blit_String_DB(DX, y, t->walls[dir].texture, buf, 0);
}

void Draw_Details(void)
{
	tile* t = TILE(gZone, sel_x, sel_y);
	char buf[100];

	sprintf(buf, "%3d,%-3d", sel_x, sel_y);
	Blit_String_DB(DX, 8, 15, buf, 0);

	sprintf(buf, "C: %5d", t->ceil);
	Blit_String_DB(DX, 24, t->ceil, buf, 0);

	sprintf(buf, "F: %5d", t->floor);
	Blit_String_DB(DX, 32, t->floor, buf, 0);

	Draw_Wall(t, North, 48);
	Draw_Wall(t, East,  56);
	Draw_Wall(t, South, 64);
	Draw_Wall(t, West,  72);

	sprintf(buf, "D: %5u", t->description);
	Blit_String_DB(DX, 88, 15, buf, 0);
	Draw_Square_DB(0, 8, 160, 8 + 38*8, 160 + 4*8, 1);
	if (t->description)
		Draw_Bounded_String(8, 160, 38, 4, 15, gZone.strings[t->description - 1], 0);

	if (t->on_enter) {
		if (parser.script_count) sprintf(buf, "S: %6s", parser.scripts[t->on_enter - 1].name);
		else sprintf(buf, "S: #%-4u", t->on_enter - 1);
		Blit_String_DB(DX, 96, 15, buf, 0);
	} else {
		Blit_String_DB(DX, 96, 15, "         ", 0);
	}

	if (t->etable) {
		sprintf(buf, "E: #%-4u", t->etable - 1);
		Blit_String_DB(DX, 104, 15, buf, 0);
	} else {
		Blit_String_DB(DX, 104, 15, "         ", 0);
	}

	redraw_details = false;
}

bool Get_Colour(colour *value)
{
	bool success;
	int temp = *value;

	Blit_String_DB(DX, 140, 15, "0-255:", 0);
	success = Input_Number(DX, 148, &temp, 0, 255);
	Blit_String_DB(DX, 140, 15, "      ", 0);
	Blit_String_DB(DX, 148, 15, "        ", 0);

	if (success) {
		*value = (colour)temp;
		return true;
	}

	return false;
}

void Change_Ceiling()
{
	if (Get_Colour(&TILE(gZone, sel_x, sel_y)->ceil)) {
		Draw_Tile(sel_x, sel_y);
		redraw_details = true;
	}
}

void Change_Floor()
{
	if (Get_Colour(&TILE(gZone, sel_x, sel_y)->floor)) {
		Draw_Tile(sel_x, sel_y);
		redraw_details = true;
	}
}

void Change_Wall(direction dir)
{
	if (Get_Colour(&TILE(gZone, sel_x, sel_y)->walls[dir].texture)) {
		Draw_Tile(sel_x, sel_y);
		redraw_details = true;
	}
}

void Change_Door(direction dir)
{
	wall *w = &(TILE(gZone, sel_x, sel_y)->walls[dir]);

	w->type++;
	if (w->type > wtLockedDoor) w->type = wtNormal;
	redraw_details = true;
}

#undef DX

string_id Add_Description(void)
{
	char buffer[MAX_STRING_LIST];

	Draw_Square_DB(0, 8, 8, 39*8, 88, true);
	Blit_String_DB(8, 8, 15, "- Hit enter twice to finish:", 0);
	clear_screen = true;
	if (Input_Multiline_String(8, 16, buffer, MAX_STRING_LIST)) {
		gZone.strings = Reallocate(gZone.strings, gZone.header.num_strings + 1, sizeof(char *), "Add_Description.strings");
		gZone.strings[gZone.header.num_strings] = Duplicate_String(buffer, "Add_Description.strings[i]");
		gZone.header.num_strings++;
		return gZone.header.num_strings;
	}

	return 0;
}

#define PER_MENU_PAGE	10
#define MENU_CANCEL		-1
#define MENU_DELETE		-2
#define MENU_ADD		-3
int Input_Scroller_Menu(char **options, int count)
{
	char buffer[50];
	int offset = 0,
		i;

	/* We're drawing over everything, so... */
	clear_screen = true;

	while (true) {
		Draw_Square_DB(0, 4, 4, 39*8 + 4, 92, true);
		Draw_Square_DB(15, 4, 4, 39*8 + 4, 92, false);
		Draw_Square_DB(15, 6, 6, 39*8 + 2, 90, false);
		for (i = 0; i < PER_MENU_PAGE; i++) {
			if (offset + i >= count) break;

			sprintf(buffer, "%d: ", i);
			strncat(buffer, options[i], 34);

			Blit_String_DB(8, 8 + i*8, 15, buffer, 0);
		}

		#define _num_scan(n, sc) \
			case sc: \
				if (i <= n) continue; \
				return offset + n;

		Show_Double_Buffer();
		switch (Get_Next_Scan_Code()) {
			case SCAN_PGUP:
				if (offset > 0) {
					offset -= PER_MENU_PAGE;
				}
				break;

			case SCAN_PGDWN:
				if (offset + PER_MENU_PAGE < count) {
					offset += PER_MENU_PAGE;
				}
				break;

			case SCAN_ESC:
			case SCAN_Q:	return MENU_CANCEL;

			case SCAN_DEL:	return MENU_DELETE;

			case SCAN_INS:	return MENU_ADD;

			case SCAN_0: return offset;
			_num_scan(1, SCAN_1)
			_num_scan(2, SCAN_2)
			_num_scan(3, SCAN_3)
			_num_scan(4, SCAN_4)
			_num_scan(5, SCAN_5)
			_num_scan(6, SCAN_6)
			_num_scan(7, SCAN_7)
			_num_scan(8, SCAN_8)
			_num_scan(9, SCAN_9)
		}
	}
}

void Change_Description(void)
{
	int result;
	tile *under = TILE(gZone, sel_x, sel_y);

	if (gZone.header.num_strings == 0) {
		under->description = Add_Description();
		return;
	}

	switch (result = Input_Scroller_Menu(gZone.strings, gZone.header.num_strings)) {
		case MENU_CANCEL: return;

		case MENU_DELETE:
			under->description = 0;
			return;

		case MENU_ADD:
			under->description = Add_Description();
			return;

		default:
			under->description = result + 1;
			return;
	}
}

void Import_Code_Strings(void)
{
	int i;

	/* In case we're re-importing, free old string entries */
	for (i = 0; i < gZone.header.num_code_strings; i++) {
		Free(gZone.code_strings[i]);
	}

	/* Import strings */
	gZone.header.num_code_strings = parser.string_count;
	gZone.code_strings = Reallocate(gZone.code_strings, parser.string_count, sizeof(char *), "Import_Code_Strings.code_strings");
	for (i = 0; i < parser.string_count; i++) {
		gZone.code_strings[i] = Duplicate_String(parser.strings[i], "Import_Code_Strings.code_strings[i]");
	}
}

void Import_Code_Scripts(void)
{
	int i;
	bytecode *code;

	/* Free old scripts */
	for (i = 0; i < gZone.header.num_scripts; i++) {
		Free(gZone.scripts[i]);
	}

	/* Import scripts */
	gZone.header.num_scripts = parser.script_count;
	gZone.scripts = Reallocate(gZone.scripts, parser.script_count, sizeof(bytecode *), "Import_Code_Scripts.scripts");
	gZone.script_lengths = Reallocate(gZone.script_lengths, parser.script_count, sizeof(length), "Import_Code_Scripts.script_lengths");
	for (i = 0; i < parser.script_count; i++) {
		gZone.script_lengths[i] = parser.scripts[i].size;
		code = SzAlloc(parser.scripts[i].size, bytecode, "Import_Code_Scripts.code");
		memcpy(code, parser.scripts[i].code, parser.scripts[i].size);
		gZone.scripts[i] = code;
	}
}

void Load_Code(void)
{
	char buffer[100],
		*dot;

	Free_Parser(&parser);
	Initialise_Parser(&parser);

	strcpy(buffer, zone_filename);
	dot = strchr(buffer, '.');
	strcpy(dot, ".JC");

	if (Compile_JC(&parser, buffer, true) == 0) {
		Import_Code_Strings();
		Import_Code_Scripts();
	} else {
		Free_Parser(&parser);
		getch();
	}
}

void Change_Script(script_id *script)
{
	char **menu;
	int result,
		i;

	if (parser.script_count == 0) return;

	menu = SzAlloc(parser.script_count, char *, "Change_Script");
	for (i = 0; i < parser.script_count; i++) {
		/* note: NOT strdup */
		menu[i] = parser.scripts[i].name;
	}

	switch (result = Input_Scroller_Menu(menu, parser.script_count)) {
		case MENU_CANCEL:
		case MENU_ADD:
			break;

		case MENU_DELETE:
			*script = 0;
			redraw_details = true;
			break;

		default:
			*script = result + 1;
			redraw_details = true;
			break;
	}

	Free(menu);
}

void Change_Enter_Script(void)
{
	tile *under = TILE(gZone, sel_x, sel_y);

	Change_Script(&under->on_enter);
}

char *Describe_Encounter(encounter_id id)
{
	char buffer[1000],
		buffer2[1000];
	encounter *en = &gZone.encounters[id];
	monster *m;
	int i;

	buffer[0] = 0;
	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		if (en->monsters[i] == 0) continue;
		m = Lookup_Monster(&gMonsters, en->monsters[i]);

		if (m == null) sprintf(buffer2, "%d-%dx??? ", en->minimum[i], en->maximum[i]);
		else sprintf(buffer2, "%d-%dx%s ", en->minimum[i], en->maximum[i], m->name);
		
		strcat(buffer, buffer2);
	}

	return Duplicate_String(buffer, "Describe_Encounter");
}

monster_id Choose_Monster(monster_id current)
{
	char **menu;
	int i, result;

	menu = SzAlloc(gMonsters.header.num_monsters, char *, "Choose_Monster");
	for (i = 0; i < gMonsters.header.num_monsters; i++) {
		/* note: not strdup */
		menu[i] = gMonsters.monsters[i].name;
	}

	Fill_Double_Buffer(0);
	switch (result = Input_Scroller_Menu(menu, gMonsters.header.num_monsters)) {
		case MENU_ADD:
		case MENU_CANCEL:
			result = current;
			break;

		case MENU_DELETE:
			result = 0;
			break;

		default:
			result = gMonsters.monsters[result].id;
			break;
	}

	Free(menu);
	return result;
}

void Edit_Encounter(encounter_id id)
{
	char buffer[100];
	unsigned char scan;
	int cursor, i, result;
	bool redraw = true;
	encounter *en = &gZone.encounters[id];

	clear_screen = true;
	cursor = 0;

	while (true) {
		if (redraw) {
			redraw = false;
			Fill_Double_Buffer(0);

			sprintf(buffer, "Editing encounter #%u", id);
			Blit_String_DB(0, 0, 15, buffer, 0);

			for (i = 0; i < ENCOUNTER_SIZE; i++) {
				sprintf(buffer, "%d-%dx#%u", en->minimum[i], en->maximum[i], en->monsters[i]);
				Blit_String_DB(8, 16 + i*8, 15, buffer, 0);
			}
			Blit_Char_DB(0, 16 + cursor*8, '>', 15, 0);

			Blit_String_DB(0, 100, 15, "Press < to edit minimum.", 0);
			Blit_String_DB(0, 108, 15, "Press > to edit maximum.", 0);
			Blit_String_DB(0, 116, 15, "Press enter to edit monster.", 0);
		}

		Show_Double_Buffer();

		scan = Get_Next_Scan_Code();
		switch (scan) {
			case SCAN_ESC:
			case SCAN_Q: return;

			case SCAN_DOWN:
				Blit_Char_DB(0, 16 + cursor*8, ' ', 15, 0);
				cursor++;
				if (cursor >= ENCOUNTER_SIZE) cursor = 0;
				Blit_Char_DB(0, 16 + cursor*8, '>', 15, 0);
				break;

			case SCAN_UP:
				Blit_Char_DB(0, 16 + cursor*8, ' ', 15, 0);
				cursor--;
				if (cursor < 0) cursor = ENCOUNTER_SIZE - 1;
				Blit_Char_DB(0, 16 + cursor*8, '>', 15, 0);
				break;

			case SCAN_COMMA:
				result = en->minimum[cursor];
				if (Input_Number(0, 130, &result, 0, 255)) {
					en->minimum[cursor] = result;
					redraw = true;
				}
				break;

			case SCAN_PERIOD:
				result = en->maximum[cursor];
				if (Input_Number(0, 130, &result, 0, 255)) {
					en->maximum[cursor] = result;
					redraw = true;
				}
				break;

			case SCAN_ENTER:
				result = Choose_Monster(en->monsters[cursor]);
				if (result) {
					en->monsters[cursor] = result;
				}
				redraw = true;
				break;
		}
	}
}

void New_Encounter(encounter_id *id)
{
	*id = gZone.header.num_encounters;

	gZone.header.num_encounters++;
	gZone.encounters = Reallocate(gZone.encounters, gZone.header.num_encounters, sizeof(encounter), "New_Encounter");

	memset(&gZone.encounters[*id], 0, sizeof(encounter));
	Edit_Encounter(*id);
}

void Edit_EncounterTable(etable_id id)
{
	char buffer[100];
	char *temp;
	unsigned char scan;
	int cursor, i;
	bool redraw = true;
	etable *et = &gZone.etables[id];

	clear_screen = true;
	cursor = 0;

	while (true) {
		if (redraw) {
			redraw = false;
			Fill_Double_Buffer(0);

			sprintf(buffer, "Editing etable #%u", id);
			Blit_String_DB(0, 0, 15, buffer, 0);

			for (i = 0; i < et->possibilities && i < ENCOUNTER_SIZE; i++) {
				temp = Describe_Encounter(et->encounters[i]);
				sprintf(buffer, "%d%%: %s", et->percentages[i], temp);
				Free(temp);
				Blit_String_DB(8, 16 + i*8, 15, buffer, 0);
			}
			Blit_Char_DB(0, 16 + cursor*8, '>', 15, 0);

			Blit_String_DB(0, 100, 15, "Press + to add line.", 0);
			Blit_String_DB(0, 108, 15, "Press - to remove last.", 0);
		}

		Show_Double_Buffer();

		scan = Get_Next_Scan_Code();
		switch (scan) {
			case SCAN_ESC:
			case SCAN_Q: return;

			case SCAN_DOWN:
				Blit_Char_DB(0, 16 + cursor*8, ' ', 15, 0);
				cursor++;
				if (cursor >= et->possibilities) cursor = 0;
				Blit_Char_DB(0, 16 + cursor*8, '>', 15, 0);
				break;

			case SCAN_UP:
				Blit_Char_DB(0, 16 + cursor*8, ' ', 15, 0);
				cursor--;
				if (cursor < 0) cursor = et->possibilities - 1;
				Blit_Char_DB(0, 16 + cursor*8, '>', 15, 0);
				break;

			case SCAN_E:
				Edit_Encounter(et->encounters[cursor]);
				redraw = true;
				break;

			case SCAN_EQUALS:
				if (et->possibilities < ETABLE_SIZE) {
					et->possibilities++;
					redraw = true;
				}
				break;

			case SCAN_MINUS:
				if (et->possibilities > 1) {
					et->possibilities--;
					redraw = true;
				}
				break;
		}
	}
}

void New_EncounterTable(etable_id *id)
{
	*id = gZone.header.num_etables;

	gZone.header.num_etables++;
	gZone.etables = Reallocate(gZone.etables, gZone.header.num_etables, sizeof(etable), "New_EncounterTable");
	gZone.etables[*id].possibilities = 1;

	memset(&gZone.etables[*id], 0, sizeof(etable));
	Edit_EncounterTable(*id);
}

char *Describe_EncounterTable(etable_id id)
{
	char buffer[1000],
		buffer2[1000];
	etable *et = &gZone.etables[id];
	int i;

	buffer[0] = 0;
	for (i = 0; i < et->possibilities; i++) {
		sprintf(buffer2, "%d%%: %s ", et->percentages[i], Describe_Encounter(et->encounters[i]));
		strcat(buffer, buffer2);
	}

	return Duplicate_String(buffer, "Describe_EncounterTable");
}

void Change_EncounterTable(void)
{
	char **menu;
	int result,
		i;
	tile *under = TILE(gZone, sel_x, sel_y);

	if (gZone.header.num_etables == 0) {
		New_EncounterTable(&under->etable);
		return;
	}

	menu = SzAlloc(gZone.header.num_etables, char *, "Change_EncounterTable");
	for (i = 0; i < gZone.header.num_etables; i++) {
		menu[i] = Describe_EncounterTable(i);
	}

	switch (result = Input_Scroller_Menu(menu, gZone.header.num_etables)) {
		case MENU_CANCEL:
			break;

		case MENU_ADD:
			New_EncounterTable(&under->etable);
			break;

		case MENU_DELETE:
			under->etable = 0;
			redraw_details = true;
			break;

		default:
			under->etable = result + 1;
			Edit_EncounterTable(result);
			redraw_details = true;
			break;
	}

	for (i = 0; i < gZone.header.num_etables; i++) {
		Free(menu[i]);
	}
	Free(menu);
}

char *Editor_State_String(void)
{
	switch (state) {
		case esWall: return "Editing Walls     ";
		case esDoor: return "Editing Doors     ";
		case esEncounter: return "Editing Encounters";
		default: return "???";
	}
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Main_Editor_Loop(void)
{
	unsigned char scan;
	int done = 0;

	clear_screen = true;
	sel_x = 0;
	sel_y = 0;

	while (!done) {
		if (clear_screen) {
			Fill_Double_Buffer(0);

			clear_screen = false;
			redraw_zone = true;
			redraw_details = true;
		}

		if (redraw_zone) Draw_Zone();
		if (redraw_details) Draw_Details();

		Blit_String_DB(0, 0, 15, Editor_State_String(), 0);
		Show_Double_Buffer();

		scan = Get_Next_Scan_Code();
		switch (scan) {
			case SCAN_Q:
				done = 1;
				break;

			case SCAN_SPACE:
				Save_Zone(zone_filename, &gZone);
				Blit_String_DB(100, 100, 15, "SAVED!", 0);
				break;

			case SCAN_LEFT:
				if (sel_x > 0) {
					sel_x--;
					Draw_Tile(sel_x + 1, sel_y);
					Draw_Tile(sel_x, sel_y);
					redraw_details = true;
				}
				break;

			case SCAN_RIGHT:
				if (sel_x < (gZone.header.width - 1)) {
					sel_x++;
					Draw_Tile(sel_x - 1, sel_y);
					Draw_Tile(sel_x, sel_y);
					redraw_details = true;
				}
				break;

			case SCAN_UP:
				if (sel_y > 0) {
					sel_y--;
					Draw_Tile(sel_x, sel_y + 1);
					Draw_Tile(sel_x, sel_y);
					redraw_details = true;
				}
				break;

			case SCAN_DOWN:
				if (sel_y < (gZone.header.height - 1)) {
					sel_y++;
					Draw_Tile(sel_x, sel_y - 1);
					Draw_Tile(sel_x, sel_y);
					redraw_details = true;
				}
				break;

			case SCAN_N:
				if (state == esDoor) Change_Door(North);
				else if (state == esWall) Change_Wall(North);
				break;
			case SCAN_E:
				if (state == esDoor) Change_Door(East);
				else if (state == esWall) Change_Wall(East);
				else if (state == esEncounter) Change_EncounterTable();
				break;
			case SCAN_S:
				if (state == esDoor) Change_Door(South);
				else if (state == esWall) Change_Wall(South);
				break;
			case SCAN_W:
				if (state == esDoor) Change_Door(West);
				else if (state == esWall) Change_Wall(West);
				break;
			case SCAN_C:
				Change_Ceiling();
				break;
			case SCAN_F:
				Change_Floor();
				break;

			case SCAN_D:
				Change_Description();
				break;
			case SCAN_TAB:
				Change_Enter_Script();
				break;

			case SCAN_F1:
				state = esWall;
				break;
			case SCAN_F2:
				state = esDoor;
				break;
			case SCAN_F3:
				state = esEncounter;
				break;

			case SCAN_TILDE:
				Load_Code();
				break;
		}
	}
}

void Create_Zone(void)
{
	char buffer[100];
	coord size;

	Initialise_Zone(&gZone);
	printf("Creating new zone file.\n");

	printf("Zone Name: ");
	scanf("%8s", buffer);
	strcat(buffer, ".ZON");
	zone_filename = Duplicate_String(buffer, "Create_Zone.name");
	printf("Zone filename: %s\n", zone_filename);

	printf("Campaign Name: ");
	scanf("%8s", buffer);
	strncpy(gZone.header.campaign_name, buffer, 8);

	printf("Width: ");
	scanf("%hhu", &size);
	gZone.header.width = size;

	printf("Height: ");
	scanf("%hhu", &size);
	gZone.header.height = size;

	gZone.tiles = SzAlloc(gZone.header.width * gZone.header.height, tile, "Create_Zone.tiles");
}

void Edit_Zone(char *filename)
{
	printf("Loading zone: %s\n", filename);
	Load_Zone(filename, &gZone);

	zone_filename = Duplicate_String(filename, "Edit_Zone.name");
}

void Load_Tables(void)
{
	char filename[13];

	strncpy(filename, gZone.header.campaign_name, 8);
	strcat(filename, ".ITM");
	Load_Items(filename, &gItems);

	strncpy(filename, gZone.header.campaign_name, 8);
	strcat(filename, ".MON");
	Load_Monsters(filename, &gMonsters);
}

void main(int argc, char **argv)
{
	if (argc < 2) {
		Create_Zone();
	} else {
		Edit_Zone(argv[1]);
	}

	Initialise_Parser(&parser);
	Load_Tables();

	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	Set_Video_Mode(VGA256);

	/* Get palette */
	PCX_Init(&explore_bg);
	PCX_Load("BACK.PCX", &explore_bg, 1);
	PCX_Delete(&explore_bg);

	state = esWall;
	Main_Editor_Loop();

	/* Cleanup */
	Set_Video_Mode(TEXT_MODE);
	Delete_Double_Buffer();

	Free_Zone(&gZone);
	Free_Parser(&parser);
	Free_Items(&gItems);
	Free_Monsters(&gMonsters);
	Free(zone_filename);

	Stop_Memory_Tracking();
}
