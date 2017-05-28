/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MAX_STRINGS 1000

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture explore_bg;
zone Z;

bool redraw_details, redraw_zone;
int sel_x, sel_y;

char* strings[MAX_STRINGS];
int next_string;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

#define TX (8 + x*7)
#define TY (8 + y*7)

void Draw_Tile(x, y)
{
	tile* t = &Z.tiles[x][y];

	if (x == sel_x && y == sel_y) {
		Square_DB(15, TX, TY, TX + 6, TY + 6, 0);
	} else {
		Square_DB(0, TX, TY, TX + 6, TY + 6, 0);
	}

	Square_DB(t->floor, TX + 1, TY + 1, TX + 5, TY + 5, 1);
	Bline_DB(TX + 2, TY + 0, TX + 4, TY + 0, t->walls[North].texture);
	Bline_DB(TX + 2, TY + 6, TX + 4, TY + 6, t->walls[South].texture);
	Bline_DB(TX + 0, TY + 2, TX + 0, TY + 4, t->walls[West].texture);
	Bline_DB(TX + 6, TY + 2, TX + 6, TY + 4, t->walls[East].texture);
}

void Draw_Zone(void)
{
	int x, y;

	for (x = 0; x < ZONE_WIDTH; x++) {
		for (y = 0; y < ZONE_HEIGHT; y++) {
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
	tile* t = &Z.tiles[sel_x][sel_y];
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
	if (t->description)
		Blit_String_Box(8, 160, 38, 4, 15, &Z.text[t->description - 1], 0);

	redraw_details = false;
}

bool Get_Colour(colour* value)
{
	int temp = *value;

	Blit_String_DB(DX, 140, 15, "0-255:", 0);
	if (Input_Number(DX, 148, &temp, 0, 255)) {
		*value = (colour)temp;
		return true;
	}

	return false;
}

void Change_Ceiling()
{
	if (Get_Colour(&Z.tiles[sel_x][sel_y].ceil)) {
		Draw_Tile(sel_x, sel_y);
		redraw_details = true;
	}
}

void Change_Floor()
{
	if (Get_Colour(&Z.tiles[sel_x][sel_y].floor)) {
		Draw_Tile(sel_x, sel_y);
		redraw_details = true;
	}
}

void Change_Wall(direction dir)
{
	if (Get_Colour(&Z.tiles[sel_x][sel_y].walls[dir].texture)) {
		Draw_Tile(sel_x, sel_y);
		redraw_details = true;
	}
}

#undef DX

void Load_Text_List(void)
{
	int ti = 0,
		l;
	char *load;

	next_string = 0;

	while (true) {
		l = strlen(&Z.text[ti]);
		if (l == 0) return;

		load = malloc(l + 1);
		strcpy(load, &Z.text[ti]);
		ti += l + 1;
		strings[next_string++] = load;
	}
}

void Delete_Text_List(void)
{
	int i;

	for (i = 0; i < next_string; i++) {
		free(strings[i]);
	}

	next_string = 0;
}

int Get_Text_Offset(int id)
{
	int j, k;

	k = id + 1;
	for (j = 0; j < id; j++) {
		k += strlen(strings[j]);
	}

	return k;
}

void Change_Description(void)
{
	int offset = 0,
		i;

	char buffer[1000],
		*allocd,
		ch;

	/* We're drawing over everything, so... */
	redraw_details = true;
	redraw_zone = true;

	while (true) {
		for (i = 0; i < 10; i++) {
			if (offset + i >= next_string) break;

			sprintf(buffer, "%d: ", i);
			strncat(buffer, strings[offset + i], 34);

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
				if (offset + 10 < next_string) {
					offset += 10;
					break;
				}

			case 'q':
				Z.tiles[sel_x][sel_y].description = 0;
				return;

			case 'n':
				if (Input_String(8, 8, buffer, 1000)) {
					allocd = strdup(buffer);
					offset = Get_Text_Offset(next_string);
					strings[next_string++] = allocd;
					Z.tiles[sel_x][sel_y].description = offset;
					strcpy(&Z.text[offset - 1], allocd);
				} else {
					Z.tiles[sel_x][sel_y].description = 0;
				}

				return;

			case '0': case '1':	case '2': case '3': case '4':
			case '5': case '6':	case '7': case '8': case '9':
				i = offset + ch - '0';
				Z.tiles[sel_x][sel_y].description = Get_Text_Offset(i);
				return;
		}
	}
}

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int done = 0;

	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	Set_Video_Mode(VGA256);

	/* Get palette */
	PCX_Init(&explore_bg);
	PCX_Load("BACK.PCX", &explore_bg, 1);
	PCX_Delete(&explore_bg);

	/* Open up our zone file */
	Zone_Load("DEMO.ZON");
	Load_Text_List();

	redraw_details = true;
	redraw_zone = true;
	sel_x = 0;
	sel_y = 0;

	while (!done) {
		if (kbhit()) {
			switch (getch()) {
				case 'q':
					done = 1;
					break;

				case ' ':
					Zone_Save("DEMO.ZON");
					Blit_String_DB(100, 100, 15, "SAVED!", 0);
					break;

				case 'j':
					if (sel_x > 0) {
						sel_x--;
						Draw_Tile(sel_x + 1, sel_y);
						Draw_Tile(sel_x, sel_y);
						redraw_details = true;
					}
					break;

				case 'l':
					if (sel_x < (ZONE_WIDTH - 1)) {
						sel_x++;
						Draw_Tile(sel_x - 1, sel_y);
						Draw_Tile(sel_x, sel_y);
						redraw_details = true;
					}
					break;

				case 'i':
					if (sel_y > 0) {
						sel_y--;
						Draw_Tile(sel_x, sel_y + 1);
						Draw_Tile(sel_x, sel_y);
						redraw_details = true;
					}
					break;

				case 'k':
					if (sel_y < (ZONE_HEIGHT - 1)) {
						sel_y++;
						Draw_Tile(sel_x, sel_y - 1);
						Draw_Tile(sel_x, sel_y);
						redraw_details = true;
					}
					break;

				case 'n':
					Change_Wall(North);
					break;
				case 'e':
					Change_Wall(East);
					break;
				case 's':
					Change_Wall(South);
					break;
				case 'w':
					Change_Wall(West);
					break;
				case 'c':
					Change_Ceiling();
					break;
				case 'f':
					Change_Floor();
					break;

				case 'd':
					Change_Description();
					break;
			}
		}

		if (redraw_zone) Draw_Zone();
		if (redraw_details) Draw_Details();

		Show_Double_Buffer();
		Delay(1);
	}

	Delete_Text_List();

	/* Cleanup */
	Set_Video_Mode(TEXT_MODE);
	Delete_Double_Buffer();
}
