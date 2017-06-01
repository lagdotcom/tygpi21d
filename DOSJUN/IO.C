/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "types.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

char* Get_String(FILE* fp)
{
	char* string;
	length len;

	fread(&len, sizeof(length), 1, fp);
	string = malloc(len + 1);
	fread(string, 1, len + 1, fp);
	return string;
}

void Save_String(char *string, FILE *fp)
{
	length len;

	len = strlen(string);
	fwrite(&len, sizeof(length), 1, fp);
	fwrite(string, 1, len + 1, fp);
}

void IO_Error(char *message)
{
	printf(message);
	exit(1);
}
