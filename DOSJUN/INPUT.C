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
	/* TODO: font */
	int cx = x,
		cy = y,
		i = 0;
	char ch;

	string[i] = 0;
	Blit_Char_DB(cx, cy, '_', WHITE, 0);
	Show_Double_Buffer();

	while (true) {
		ch = getch();

		if (ch == '\r') {
			if (i == 0) {
				continue;
			}

			string[i] = 0;
			return true;
		} else if (ch == DEL) {
			if (i > 0) {
				Blit_Char_DB(cx, cy, ' ', WHITE, 0);
				string[--i] = 0;
				cx -= 8;
			}
		} else if (i < (max - 1)) {
			string[i++] = ch;
			string[i] = 0;
			Blit_Char_DB(cx, cy, ch, WHITE, 0);
			cx += 8;
		}

		if (cx < 0) {
			cy -= 8;
			cx = SCREEN_WIDTH - 8;
		} else if (cx >= SCREEN_WIDTH) {
			cy += 8;
			cx = 0;
		}

		Blit_Char_DB(cx, cy, '_', WHITE, 0);
		Show_Double_Buffer();
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
		height = gFont->header.height,
		i;
	unsigned char key;

	for (i = 0; i < choices; i++) {
		Draw_Font(x, y + i*height, WHITE, " ", gFont, false);
		Draw_Font(x + 8, y + i*height, WHITE, menu[i], gFont, false);
	}

	Draw_Font_Char(x, y + choice*height, WHITE, '>', gFont, false);
	Show_Double_Buffer();

	while (true) {
		key = Get_Next_Scan_Code();

		switch (key) {
			case SCAN_DOWN:
				Draw_Font_Char(x, y + choice*height, WHITE, ' ', gFont, false);
				choice++;
				if (choice == choices) choice = 0;
				Draw_Font_Char(x, y + choice*height, WHITE, '>', gFont, false);
				Show_Double_Buffer();
				break;

			case SCAN_UP:
				Draw_Font_Char(x, y + choice*height, WHITE, ' ', gFont, false);
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
