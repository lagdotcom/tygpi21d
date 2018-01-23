#ifndef DOSJUN_MONSTERS_H
#define DOSJUN_MONSTERS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"
#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MONSTERS_HEADER_PADDING		26
#define MONSTER_PADDING				1
#define MONSTER_SIZE				76

typedef enum row {
	rowFront,
	rowBack
} row;

typedef enum ai {
	aiMindless,
	aiRogue
} ai;

typedef enum monsterflag {
	mHasSkills = 1,
} monsterflag;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct monster {
	char name[NAME_SIZE];
	char image[8];
	monster_id id;
	stat_value stats[NUM_STATS];
	row_id row;
	ai_id ai;
	UINT32 experience;
	item_id weapon;
	monsterflag_id flags;
	unsigned char unused[MONSTER_PADDING];

	list* skills;
} monster;

typedef struct monsters_header {
	VERSION_HEADER;

	UINT16 num_monsters;

	unsigned char unused[MONSTERS_HEADER_PADDING];
} monsters_header;

typedef struct monsters {
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
