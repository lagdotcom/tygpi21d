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
bool redraw_details, redraw_zone;
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

	Blit_String_DB(8, 8, 15, "- Hit enter twice to finish:", 0);
	if (Input_Multiline_String(8, 16, buffer, MAX_STRING_LIST)) {
		Z.strings = realloc(Z.strings, sizeof(char *) * (Z.header.num_strings + 1));
		Z.strings[Z.header.num_strings] = strdup(buffer);
		Z.header.num_strings++;
		Z.header.start_script_strings++;
		return Z.header.num_strings;
	}

	return 0;
}

void Change_Description(void)
{
	int offset = 0,
		i;

	char buffer[50],
		*allocd,
		ch;

	tile *under = TILE(Z, sel_x, sel_y);

	/* We're drawing over everything, so... */
	redraw_details = true;
	redraw_zone = true;

	if (Z.header.num_strings == 0) {
		under->description = Add_Description();
		return;
	}

	while (true) {
		for (i = 0; i < 10; i++) {
			if (offset + i >= Z.header.num_strings) break;

			sprintf(buffer, "%d: ", i);
			strncat(buffer, Z.strings[offset + i], 34);

			Blit_String_DB(8, 8 + i*8, 15, buffer, 0);
		}

		Show_Double_Buffer();
		ch = getch();
		switch (ch) {
			case ',':
				if (offset > 0) {
					offset -= 10;
					break;
				}

			case '.':
				if (offset + 10 < Z.header.num_strings) {
					offset += 10;
					break;
				}

			case 'q':
				under->description = 0;
				return;

			case 'n':
				under->description = Add_Description();

				return;

			case '0': case '1':	case '2': case '3': case '4':
			case '5': case '6':	case '7': case '8': case '9':
				under->description = offset + ch - '0' + 1;
				return;
		}
	}
}

void Import_Code_Strings(void)
{
	int i, total;

	total = Z.header.start_script_strings + parser.string_count;

	/* In case we're re-importing, free old string entries */
	for (i = Z.header.start_script_strings; i < Z.header.num_strings; i++) {
		free(Z.strings[i]);
	}

	/* Import strings */
	Z.header.num_strings = total;
	Z.strings = realloc(Z.strings, sizeof(char *) * total);
	for (i = 0; i < parser.string_count; i++) {
		Z.strings[i + parser.string_offset] = strdup(parser.strings[i]);
	}
}

void Import_Code_Scripts(void)
{
	int i;
	bytecode *code;

	/* Free old scripts */
	for (i = 0; i < Z.header.num_scripts; i++) {
		free(Z.strings[i]);
	}

	/* Import scripts */
	Z.header.num_scripts = parser.script_count;
	Z.scripts = realloc(Z.scripts, sizeof(bytecode *));
	Z.script_lengths = realloc(Z.script_lengths, sizeof(length));
	for (i = 0; i < parser.script_count; i++) {
		Z.script_lengths[i] = parser.scripts[i].size;
		code = szalloc(parser.scripts[i].size, bytecode);
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

	parser.string_offset = Z.header.start_script_strings;
	if (Compile_JC(&parser, buffer, true) == 0) {
		Import_Code_Strings();
		Import_Code_Scripts();
	} else {
		Free_Parser(&parser);
		getch();
	}
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Main_Editor_Loop(void)
{
	unsigned char scan;
	int done = 0;

	redraw_details = true;
	redraw_zone = true;
	sel_x = 0;
	sel_y = 0;

	while (!done) {
		if (redraw_zone) Draw_Zone();
		if (redraw_details) Draw_Details();

		Show_Double_Buffer();

		scan = Get_Scan_Code();
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
	gets(buffer);
	strcat(buffer, ".ZON");
	zone_filename = strdup(buffer);

	printf("Campaign Name: ");
	gets(buffer);
	strncpy(Z.header.campaign_name, buffer, 8);

	printf("Width: ");
	scanf("%d", &size);
	Z.header.width = size;

	printf("Height: ");
	scanf("%d", &size);
	Z.header.height = size;

	Z.tiles = szalloc(Z.header.width * Z.header.height, tile);
}

void Edit_Zone(char *filename)
{
	printf("Loading zone: %s\n", filename);
	Load_Zone(filename, &Z);

	zone_filename = strdup(filename);
}

void main(int argc, char **argv)
{
	if (argc < 2) {
		Create_Zone();
	} else {
		Edit_Zone(argv[1]);
	}

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
	free(zone_filename);
}
