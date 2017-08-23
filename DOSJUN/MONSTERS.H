#ifndef DOSJUN_MONSTERS_H
#define DOSJUN_MONSTERS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MONSTERS_HEADER_PADDING		27
#define MONSTER_PADDING				8

typedef enum {
	rowFront,
	rowBack
} row;

typedef enum {
	aiMindless
} ai;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	char name[NAME_SIZE];
	char image[8];
	monster_id id;
	stat stats[NUM_STATS];
	monster_row row;
	monster_ai ai;

	unsigned char unused[MONSTER_PADDING];
} monster;

typedef struct {
	VERSION_HEADER;

	unsigned char num_monsters;

	unsigned char unused[MONSTERS_HEADER_PADDING];
} monsters_header;

typedef struct {
	monsters_header header;

	monster *monsters;
} monsters;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Monsters(monsters *m);
void Initialise_Monsters(monsters *m);
bool Load_Monsters(char *filename, monsters *m);
monster *Lookup_Monster(monsters *lib, monster_id id);
bool Save_Monsters(char *filename, monsters *m);

#endif
