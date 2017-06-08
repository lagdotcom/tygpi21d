/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MAX_FILES	20

/* F U N C T I O N S ///////////////////////////////////////////////////// */

char *Read_LengthString(FILE *fp)
{
	char *string;
	length len;

	fread(&len, sizeof(length), 1, fp);
	string = SzAlloc(len + 1, char, "Read_LengthString");
	fread(string, 1, len + 1, fp);
	return string;
}

void Write_LengthString(char *string, FILE *fp)
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

char **Get_Directory_Listing(char *pattern, int *count)
{
	char **filenames;
	struct ffblk ff;
	int i, done;

	filenames = SzAlloc(MAX_FILES, char *, "Get_Directory_Listing");

	i = 0;
	done = findfirst(pattern, &ff, 0);
	while (!done) {
		filenames[i++] = Duplicate_String(ff.ff_name, "Get_Directory_Listing");
		done = findnext(&ff);

		if (i == MAX_FILES) break;
	}

	*count = i;
	return filenames;
}

void Free_Directory_Listing(char **listing, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		Free(listing[i]);
	}
	Free(listing);
}
