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
	skConcentrate,
	skReverberation,
	skCleave,

	NUM_SKILLS
} skill_id;

typedef struct skill_spec {
	char *name;
	char *description;
} skill_spec;

typedef struct level_spec {
	statistic stat;
	skill_id a;
	skill_id b;
} level_spec;

typedef struct job_spec {
	char *name;
	int hp_per_level;
	int mp_per_level;
	level_spec levels[JOB_LEVELS];
} job_spec;

extern skill_spec skills[NUM_SKILLS];
extern job_spec jobspecs[NUM_JOBS];

/* Check if a combatant/character has a skill learnt. */
#define Has_Skill(c, sk) In_List(c->skills, (void*)sk)

#define Job_Name(n) (jobspecs[n].name)

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Add_Experience(pc *c, UINT32 xp);
void Free_Jobs(void);
void Initialise_Jobs(void);
void Level_Up(pc *c);
void Set_Job(pc *c, job job);

/* Get string representation of stat */
char *Stat_Name(statistic st);

#endif
