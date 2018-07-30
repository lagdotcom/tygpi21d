/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include <stdlib.h>
#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define DEL 8
#define ESC	27

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Input_Number(int x, int y, int *number, int min, int max)
{
	/* TODO: font */
	int cx = 0,
		cy = y,
		i;
	char ch,
		buffer[10];

	itoa(*number, buffer, 10);
	i = strlen(buffer);

	Blit_String_DB(x, y, WHITE, buffer, 0);

	cx = x + i * 8;
	Blit_Char_DB(cx, cy, '_', WHITE, 0);
	Show_Double_Buffer();

	while (true) {
		ch = getch();

		if (ch == ESC) {
			return false;
		} else if (ch == '\r') {
			if (i > 0) {
				*number = atoi(buffer);

				if (*number >= min && *number <= max)
					return true;
			}
		} else if (ch == DEL) {
			if (i > 0) {
				buffer[i] = 0;
				i--;

				Blit_Char_DB(cx, cy, ' ', WHITE, 0);
				cx -= 8;
				Blit_Char_DB(cx, cy, '_', WHITE, 0);

				Show_Double_Buffer();
			}
		} else if (ch >= '0' && ch <= '9') {
			buffer[i++] = ch;
			buffer[i] = 0;

			Blit_Char_DB(cx, cy, ch, WHITE, 0);

			if (i == 9) {
				i--;
			} else {
				cx += 8;
				Blit_Char_DB(cx, cy, '_', WHITE, 0);
			}

			Show_Double_Buffer();
		}
	}
}

bool Input_String(int x, int y, char *string, int max)
{
	int i = 0;
	char ch;
	int ex = SCREEN_WIDTH - 1;
	int ey = y + Char_Height(gFont, ' ') - 1;

	string[i] = '_';
	string[i + 1] = 0;

	while (true) {
		Draw_Square_DB(0, x, y, ex, ey, true);
		Draw_Font(x, y, 0, string, gFont, true);
		Show_Double_Buffer();

		ch = getch();
		if (ch == '\r') {
			if (i == 0) {
				continue;
			}

			string[i] = 0;
			return true;
		} else if (ch == DEL) {
			if (i > 0) {
				string[i] = 0;
				string[--i] = '_';
			}
		} else if (i < (max - 1)) {
			string[i++] = ch;
			string[i] = '_';
			string[i + 1] = 0;
		}
	}
}

bool Input_Multiline_String(int x, int y, char *string, int max)
{
	/* TODO: font */
	int cx = x,
		cy = y,
		i = 0;
	char lastch = 0, ch;

	string[i] = 0;
	Blit_Char_DB(cx, cy, '_', WHITE, 0);
	Show_Double_Buffer();

	while (true) {
		ch = getch();

		if (ch == '\r') {
			if (i == 0) {
				return false;
			}

			if (lastch == ch) {
				string[i - 1] = 0;
				return true;
			}

			string[i++] = '\n';
			cx = x;
			cy += 8;
		} else if (ch == DEL) {
			if (i > 0) {
				string[--i] = 0;
				cx -= 8;
			}
		} else if (i < (max - 1)) {
			string[i++] = ch;
			string[i] = 0;
			Blit_Char_DB(cx, cy, ch, WHITE, 0);
			cx += 8;
		}

		lastch = ch;
		Blit_Char_DB(cx, cy, '_', WHITE, 0);
		Show_Double_Buffer();
	}
}

int Input_Menu(char **menu, int choices, int x, int y)
{
	int choice = 0,
		height = Char_Height(gFont, ' '),
		xoff = Char_Width(gFont, '>') - 1,
		i;
	unsigned char key;

	for (i = 0; i < choices; i++) {
		Draw_Square_DB(0, x, y + i * height, x + xoff, y + i * height + height, true);
		Draw_Font(x + xoff + 1, y + i*height, WHITE, menu[i], gFont, false);
	}

	Draw_Font_Char(x, y + choice*height, WHITE, '>', gFont, false);
	Show_Double_Buffer();

	while (true) {
		key = Get_Next_Scan_Code();

		switch (key) {
			case SCAN_DOWN:
				Draw_Square_DB(0, x, y + choice * height, x + xoff, y + choice * height + height, true);
				choice++;
				if (choice == choices) choice = 0;
				Draw_Font_Char(x, y + choice*height, WHITE, '>', gFont, false);
				Show_Double_Buffer();
				break;

			case SCAN_UP:
				Draw_Square_DB(0, x, y + choice * height, x + xoff, y + choice * height + height, true);
				if (choice == 0) choice = choices - 1;
				else choice--;
				Draw_Font_Char(x, y + choice*height, WHITE, '>', gFont, false);
				Show_Double_Buffer();
				break;

			case SCAN_ENTER:
				return choice;
		}

		Delay(1);
	}
}
