#ifndef DOSJUN_INPUT_H
#define DOSJUN_INPUT_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Input_Number(int x, int y, int *number, int min, int max);
bool Input_String(int x, int y, char *string, int max);
bool Input_Multiline_String(int x, int y, char *string, int max);
int Input_Menu(char **menu, int choices, int x, int y);

#endif
