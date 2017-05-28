/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include <stdlib.h>
#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define DEL 8

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Input_Number(int x, int y, int* number, int min, int max)
{
	int cx = 0,
		cy = y,
		i,
		done = 0;
	char ch,
		buffer[10];

	itoa(*number, buffer, 10);
	i = strlen(buffer);

	Blit_String_DB(x, y, 15, buffer, 0);

	cx = x + i * 8;
	Blit_Char_DB(cx, cy, '_', 15, 0);
	Show_Double_Buffer();

	while (!done) {
		/* TODO: cancel by hitting escape */
		ch = getch();

		if (ch == '\r') {
			if (i > 0) {
				*number = atoi(buffer);

				if (*number >= min && *number <= max)
					return true;
			}
		} else if (ch == DEL) {
			if (i > 0) {
				buffer[i] = 0;
				i--;

				Blit_Char_DB(cx, cy, ' ', 15, 0);
				cx -= 8;
				Blit_Char_DB(cx, cy, '_', 15, 0);

				Show_Double_Buffer();
			}
		} else if (ch >= '0' && ch <= '9') {
			buffer[i++] = ch;
			buffer[i] = 0;

			Blit_Char_DB(cx, cy, ch, 15, 0);

			if (i == 9) {
				i--;
			} else {
				cx += 8;
				Blit_Char_DB(cx, cy, '_', 15, 0);
			}

			Show_Double_Buffer();
		}
	}
}

bool Input_String(int x, int y, char* string, int max)
{
	/* TODO: implement max length */
	int cx = x,
		cy = y,
		i = 0,
		done = 0;
	char lastch = 0, ch;

	string[i] = 0;
	Blit_Char_DB(cx, cy, '_', 15, 0);
	Show_Double_Buffer();

	while (!done) {
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
		} else {
			string[i++] = ch;
			string[i] = 0;
			Blit_Char_DB(cx, cy, ch, 15, 0);
			cx += 8;
		}

		lastch = ch;
		Blit_Char_DB(cx, cy, '_', 15, 0);
		Show_Double_Buffer();
	}
}
