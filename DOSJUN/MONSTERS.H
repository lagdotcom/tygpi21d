#ifndef DOSJUN_MONSTERS_H
#define DOSJUN_MONSTERS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"
#include "strings.h"
#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MONSTER_PADDING				12
#define MONSTER_SIZE				64

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
	VERSION_HEADER;
	str_id name_id;
	file_id image_id;
	row_id row;
	ai_id ai;
	UINT32 experience;
	file_id weapon;
	monsterflag_id flags;
	file_id drops_id;
	unsigned char unused[MONSTER_PADDING];
	stat_value stats[NUM_STATS];

	list* skills;
} monster;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Monster(monster *m);
bool Load_Monster(char *filename, monster *m);
bool Read_Monster(FILE *fp, monster *m);
bool Save_Monster(char *filename, monster *m);

#endif
