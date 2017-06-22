#ifndef DOSJUN_SAVE_H
#define DOSJUN_SAVE_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SAVE_HEADER_PADDING			14

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	item_id item;
	inventory_flag flags;
	unsigned char quantity;
} inventory;

typedef struct {
	char name[NAME_SIZE];
	job_id job;
	unsigned char level;
	unsigned int stats[NUM_STATS];
	inventory items[INVENTORY_SIZE];
} character;

typedef struct {
	VERSION_HEADER;

	char campaign_name[8];
	zone_id zone;
	direction facing;
	coord x, y;
	unsigned char num_characters;
	zone_id num_zones;

	unsigned char unused[SAVE_HEADER_PADDING];
} save_header;

typedef struct {
	save_header header;

	character characters[PARTY_SIZE];
	int *script_globals;
	int **script_locals;
} save;


/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Load_Savefile(char *filename, save *s);
void Free_Savefile(save *s);
void Initialise_Savefile(save *s);
bool Save_Savefile(char *filename, save *s);

#endif
