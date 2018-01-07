#ifndef DOSJUN_CLASS_H
#define DOSJUN_CLASS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "save.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define JOB_LEVELS	10

typedef enum skill_id {
	skSing,

	skHide,
	skBludgeon,
	skVenom,

	NUM_SKILLS
} skill_id;

typedef struct skill_spec {
	char *name;
	char *description;
} skill_spec;

extern skill_spec skills[NUM_SKILLS];

/* Check if a combatant/character has a skill learnt. */
#define Has_Skill(c, sk) In_List(c->skills, (void*)sk)

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Add_Experience(character *c, UINT32 xp);
void Free_Jobs(void);
void Initialise_Jobs(void);
void Level_Up(character *c);
void Set_Job(character *c, job job);

#endif
