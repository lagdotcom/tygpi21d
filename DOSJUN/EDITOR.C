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

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture explore_bg;
jc_parser parser;
zone Z;

char *zone_filename;
bool redraw_details, redraw_zone, clear_screen;
int sel_x, sel_y;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Input_Multiline_String(int x, int y, char *string, int max);
bool Input_Number(int x, int y, int *number, int min, int max);
void Draw_Bounded_String(int x, int y, int w, int h, colour col, char *string, bool trans_flag);
void Draw_Line_DB(int xo, int yo, int x1, int y1, colour col);
void Draw_Square_DB(colour col, int x0, int y0, int x1, int y1, bool filled);
void Free_Zone(zone *z);
void Initialise_Zone(zone *z);
void Load_Zone(char *filename, zone *z);
void Save_Zone(char *filename, zone *z);

/* F U N C T I O N S ///////////////////////////////////////////////////// */

#define TX (8 + x*7)
#define TY (8 + y*7)

void Draw_Tile(coord x, coord y)
{
	tile* t = TILE(Z, x, y);

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

	for (x = 0; x < Z.header.width; x++) {
		for (y = 0; y < Z.header.height; y++) {
			Draw_Tile(x, y);
		}
	}

	redraw_zone = false;
}

#undef TY
#undef TX

#define DX 250

void Draw_Details(void)
{
	tile* t = TILE(Z, sel_x, sel_y);
	char buf[100];

	sprintf(buf, "%3d,%-3d", sel_x, sel_y);
	Blit_String_DB(DX, 8, 15, buf, 0);

	sprintf(buf, "C: %5d", t->ceil);
	Blit_String_DB(DX, 24, t->ceil, buf, 0);

	sprintf(buf, "F: %5d", t->floor);
	Blit_String_DB(DX, 32, t->floor, buf, 0);

	sprintf(buf, "WN: %4d", t->walls[North].texture);
	Blit_String_DB(DX, 48, t->walls[North].texture, buf, 0);

	sprintf(buf, "WE: %4d", t->walls[East].texture);
	Blit_String_DB(DX, 56, t->walls[East].texture, buf, 0);

	sprintf(buf, "WS: %4d", t->walls[South].texture);
	Blit_String_DB(DX, 64, t->walls[South].texture, buf, 0);

	sprintf(buf, "WW: %4d", t->walls[West].texture);
	Blit_String_DB(DX, 72, t->walls[West].texture, buf, 0);

	sprintf(buf, "D: %5d", t->description);
	Blit_String_DB(DX, 88, 15, buf, 0);
	Draw_Square_DB(0, 8, 160, 8 + 38*8, 160 + 4*8, 1);
	if (t->description)
		Draw_Bounded_String(8, 160, 38, 4, 15, Z.strings[t->description - 1], 0);

	if (t->on_enter) {
		if (parser.script_count) sprintf(buf, "S: %6s", parser.scripts[t->on_enter - 1].name);
		else sprintf(buf, "S: #%d", t->on_enter - 1);
		Blit_String_DB(DX, 96, 15, buf, 0);
	} else {
		Blit_String_DB(DX, 96, 15, "         ", 0);
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
	if (Get_Colour(&TILE(Z, sel_x, sel_y)->ceil)) {
		Draw_Tile(sel_x, sel_y);
		redraw_details = true;
	}
}

void Change_Floor()
{
	if (Get_Colour(&TILE(Z, sel_x, sel_y)->floor)) {
		Draw_Tile(sel_x, sel_y);
		redraw_details = true;
	}
}

void Change_Wall(direction dir)
{
	if (Get_Colour(&TILE(Z, sel_x, sel_y)->walls[dir].texture)) {
		Draw_Tile(sel_x, sel_y);
		redraw_details = true;
	}
}

#undef DX

string_id Add_Description(void)
{
	char buffer[MAX_STRING_LIST];

	Draw_Square_DB(0, 8, 8, 39*8, 88, true);
	Blit_String_DB(8, 8, 15, "- Hit enter twice to finish:", 0);
	if (Input_Multiline_String(8, 16, buffer, MAX_STRING_LIST)) {
		/* TODO: account for adding a string after importing code */
		Z.strings = Reallocate(Z.strings, Z.header.num_strings + 1, sizeof(char *), "Add_Description.strings");
		Z.strings[Z.header.num_strings] = Duplicate_String(buffer, "Add_Description.strings[i]");
		Z.header.num_strings++;
		return Z.header.num_strings;
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
		Draw_Square_DB(0, 8, 8, 39*8, 88, true);
		for (i = 0; i < PER_MENU_PAGE; i++) {
			if (offset + i >= count) break;

			sprintf(buffer, "%d: ", i);
			strncat(buffer, options[i], 34);

			Blit_String_DB(8, 8 + i*8, 15, buffer, 0);
		}

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

			case SCAN_ESC:	return MENU_CANCEL;
			case SCAN_DEL:	return MENU_DELETE;
			case SCAN_INS:	return MENU_ADD;

			case SCAN_0: return offset;
			case SCAN_1: return offset + 1;
			case SCAN_2: return offset + 2;
			case SCAN_3: return offset + 3;
			case SCAN_4: return offset + 4;
			case SCAN_5: return offset + 5;
			case SCAN_6: return offset + 6;
			case SCAN_7: return offset + 7;
			case SCAN_8: return offset + 8;
			case SCAN_9: return offset + 9;
		}
	}
}

void Change_Description(void)
{
	int result;
	tile *under = TILE(Z, sel_x, sel_y);

	if (Z.header.num_strings == 0) {
		under->description = Add_Description();
		return;
	}

	switch (result = Input_Scroller_Menu(Z.strings, Z.header.num_strings)) {
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
	for (i = 0; i < Z.header.num_code_strings; i++) {
		Free(Z.code_strings[i]);
	}

	/* Import strings */
	Z.header.num_code_strings = parser.string_count;
	Z.code_strings = Reallocate(Z.code_strings, parser.string_count, sizeof(char *), "Import_Code_Strings.code_strings");
	for (i = 0; i < parser.string_count; i++) {
		Z.code_strings[i] = Duplicate_String(parser.strings[i], "Import_Code_Strings.code_strings[i]");
	}
}

void Import_Code_Scripts(void)
{
	int i;
	bytecode *code;

	/* Free old scripts */
	for (i = 0; i < Z.header.num_scripts; i++) {
		Free(Z.scripts[i]);
	}

	/* Import scripts */
	Z.header.num_scripts = parser.script_count;
	Z.scripts = Reallocate(Z.scripts, parser.script_count, sizeof(bytecode *), "Import_Code_Scripts.scripts");
	Z.script_lengths = Reallocate(Z.script_lengths, parser.script_count, sizeof(length), "Import_Code_Scripts.script_lengths");
	for (i = 0; i < parser.script_count; i++) {
		Z.script_lengths[i] = parser.scripts[i].size;
		code = SzAlloc(parser.scripts[i].size, bytecode, "Import_Code_Scripts.code");
		memcpy(code, parser.scripts[i].code, parser.scripts[i].size);
		Z.scripts[i] = code;
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
			break;

		default:
			*script = result + 1;
			break;
	}

	Free(menu);
}

void Change_Enter_Script(void)
{
	tile *under = TILE(Z, sel_x, sel_y);

	Change_Script(&under->on_enter);
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

		Show_Double_Buffer();

		scan = Get_Next_Scan_Code();
		switch (scan) {
			case SCAN_Q:
				done = 1;
				break;

			case SCAN_SPACE:
				Save_Zone(zone_filename, &Z);
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
				if (sel_x < (Z.header.width - 1)) {
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
				if (sel_y < (Z.header.height - 1)) {
					sel_y++;
					Draw_Tile(sel_x, sel_y - 1);
					Draw_Tile(sel_x, sel_y);
					redraw_details = true;
				}
				break;

			case SCAN_N:
				Change_Wall(North);
				break;
			case SCAN_E:
				Change_Wall(East);
				break;
			case SCAN_S:
				Change_Wall(South);
				break;
			case SCAN_W:
				Change_Wall(West);
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

	Initialise_Zone(&Z);
	printf("Creating new zone file.\n");

	printf("Zone Name: ");
	scanf("%8s", &buffer);
	strcat(buffer, ".ZON");
	zone_filename = Duplicate_String(buffer, "Create_Zone.name");
	printf("Zone filename: %s\n", zone_filename);

	printf("Campaign Name: ");
	scanf("%8s", &buffer);
	strncpy(Z.header.campaign_name, buffer, 8);

	printf("Width: ");
	scanf("%hhu", &size);
	Z.header.width = size;

	printf("Height: ");
	scanf("%hhu", &size);
	Z.header.height = size;

	Z.tiles = SzAlloc(Z.header.width * Z.header.height, tile, "Create_Zone.tiles");
}

void Edit_Zone(char *filename)
{
	printf("Loading zone: %s\n", filename);
	Load_Zone(filename, &Z);

	zone_filename = Duplicate_String(filename, "Edit_Zone.name");
}

void main(int argc, char **argv)
{
	if (argc < 2) {
		Create_Zone();
	} else {
		Edit_Zone(argv[1]);
	}

	Initialise_Parser(&parser);

	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	Set_Video_Mode(VGA256);

	/* Get palette */
	PCX_Init(&explore_bg);
	PCX_Load("BACK.PCX", &explore_bg, 1);
	PCX_Delete(&explore_bg);

	Main_Editor_Loop();

	/* Cleanup */
	Set_Video_Mode(TEXT_MODE);
	Delete_Double_Buffer();

	Free_Zone(&Z);
	Free_Parser(&parser);
	Free(zone_filename);

	Stop_Memory_Tracking();
}
