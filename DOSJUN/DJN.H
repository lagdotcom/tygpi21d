#ifndef DOSJUN_DJN_H
#define DOSJUN_DJN_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define DJN_PADDING			18
#define DJN_FILE_PADDING	3

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
	ftFont, /* UNUSED */
	ftStrings,
	ftPalette,
	ftDropTable,

	ftParty = 0xC0,
	ftGlobals,
	ftOverlay,
} djn_type;

typedef enum djn_subtype {
	stUnknown,
	stScreen,
	stTexture,
	stPortrait,
	stThing,
	stMonster,

	stAny = 255,
} djn_subtype;

typedef struct djn_file {
	file_id id;
	UINT32 offset;
	UINT32 size;
	UINT8 type;
	UINT8 flags;
	UINT8 subtype;
	unsigned char unused[DJN_FILE_PADDING];

	char *name;
	void *object;
	void *parent;	/* djn */
} djn_file;
#define DJN_FILESZ 16

typedef struct djn {
	VERSION_HEADER;
	UINT32 count;
	UINT32 directory;
	UINT16 flags;
	unsigned char unused[DJN_PADDING];

	djn_file *files;
	FILE *f;
	void *next; /* djn */
} djn;
#define DJN_HEADERSZ 32

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern djn_file *current_reading_file;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Add_to_Djn(djn *d, void *f, file_id id, djn_type ty);
void Close_Djn(djn *d);
void *Find_File_Type(djn *chain, djn_type ty);
int Find_Files_of_Type(djn *d, djn_type ty, djn_subtype sty, file_id *buffer, int max);
void Free_Djn(djn *d);
file_id Get_File_ID(djn *chain, void *data);
bool In_Djn(djn *d, file_id id, bool chained);
bool Load_Djn(char *filename, djn *d);
void *Lookup_File(djn *chain, file_id id, bool chained);
djn_file *Lookup_File_Entry(djn *chain, file_id id, bool load, bool chained);
bool Save_Djn(char *filename, djn *d);
void Unload_File(djn *chain, file_id id);

#define Lookup_File_Chained(chain, id) Lookup_File(chain, id, true)

#endif
