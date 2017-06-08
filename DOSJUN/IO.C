/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MAX_FILES	20

/* F U N C T I O N S ///////////////////////////////////////////////////// */

char *Read_LengthString(FILE *fp)
{
	char *string;
	length len;

	fread(&len, sizeof(length), 1, fp);
	string = malloc(len + 1);
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

	filenames = calloc(MAX_FILES, sizeof(char*));

	i = 0;
	done = findfirst(pattern, &ff, 0);
	while (!done) {
		filenames[i++] = strdup(ff.ff_name);
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
		free(listing[i]);
	}
	free(listing);
}
