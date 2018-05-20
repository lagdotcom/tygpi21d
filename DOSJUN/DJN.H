#ifndef DOSJUN_DJN_H
#define DOSJUN_DJN_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef UINT16 file_id;

#define DJN_FILE_PADDING	4

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef enum djn_file_flags {
	ffPrivate = 1,
	ffTexture = 2,
} djn_file_flags;

typedef enum djn_flags {
	dfDesign = 1,
} djn_flags;

typedef enum djn_type {
	ftUnknown,
	ftCampaign,
	ftZone,
	ftNPC,
	ftPC,
	ftScript,
	ftSource,
	ftGraphic,
	ftMusic,
	ftSound,
	ftItem,
	ftMonster,
	ftFont,
	ftStrings,
	ftPalette,
} djn_type;

typedef struct djn_file {
	file_id id;
	UINT32 offset;
	UINT32 size;
	UINT8 type;
	UINT8 flags;
	unsigned char unused[DJN_FILE_PADDING];

	char *name;
	void *object;
} djn_file;
#define DJN_FILESZ 16

typedef struct djn {
	VERSION_HEADER;
	UINT32 count;
	UINT32 directory;
	UINT16 flags;

	djn_file *files;
	FILE *f;
	void *next; /* djn */
} djn;
#define DJN_HEADERSZ 14

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Close_Djn(djn *d);
void *Find_File_Type(djn *chain, djn_type ty);
void Free_Djn(djn *d);
bool Load_Djn(char *filename, djn *d);
void *Lookup_File(djn *d, file_id id);

#endif
