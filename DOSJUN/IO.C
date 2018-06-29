/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dosjun.h"
#include "features.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

char *Read_LengthString(FILE *fp, char *tag)
{
	char *string;
	length len;

	fread(&len, sizeof(length), 1, fp);
	if (len == 0)
		return null;

	string = SzAlloc(len + 1, char, tag);
	if (string == null)
		die("Read_LengthString: out of memory");

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

char **Get_Directory_Listing(char *pattern, int *count)
{
	char **filenames;
	struct ffblk ff;
	int i, done;

	filenames = SzAlloc(IO_MAXFILES, char *, "Get_Directory_Listing");
	if (filenames == null)
		die("Get_Directory_Listing: out of memory");

	i = 0;
	done = findfirst(pattern, &ff, 0);
	while (!done) {
		filenames[i++] = Duplicate_String(ff.ff_name, "Get_Directory_Listing");
		done = findnext(&ff);

		if (i == IO_MAXFILES) break;
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

unsigned char Get_Next_Scan_Code(void)
{
	unsigned char scan = 0;

	while (scan == 0) scan = Get_Scan_Code();
	return scan;
}

bool Check_Version(char *magic, unsigned char version, char *tag)
{
	if (strncmp(magic, FILE_MAGIC, 3) != 0 || version > VERSION_NOW)
		dief("%s: File does not belong to DOSJUN v%d.\n", tag, VERSION_NOW);

	return true;
}
