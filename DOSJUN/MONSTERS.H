#ifndef DOSJUN_MONSTERS_H
#define DOSJUN_MONSTERS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"
#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

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

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Monster(monster *m);
bool Load_Monster(char *filename, monster *m);
bool Read_Monster(FILE *fp, monster *m);
bool Save_Monster(char *filename, monster *m);

#endif
