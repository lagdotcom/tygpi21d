#ifndef DOSJUN_CLASS_H
#define DOSJUN_CLASS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "save.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define JOB_LEVELS	10

typedef enum skill_id {
	skConcentrate,
	sk_F1b,
	skCleave,
	sk_F2b,
	sk_F3a,
	sk_F3b,
	sk_F4a,
	sk_F4b,
	sk_F5a,
	sk_F5b,
	sk_F6a,
	sk_F6b,
	sk_F7a,
	sk_F7b,
	sk_F8a,
	sk_F8b,
	sk_F9a,
	sk_F9b,
	sk_F10a,
	sk_F10b,

	sk_C1a,
	sk_C1b,
	sk_C2a,
	sk_C2b,
	sk_C3a,
	sk_C3b,
	sk_C4a,
	sk_C4b,
	sk_C5a,
	sk_C5b,
	sk_C6a,
	sk_C6b,
	sk_C7a,
	sk_C7b,
	sk_C8a,
	sk_C8b,
	sk_C9a,
	sk_C9b,
	sk_C10a,
	sk_C10b,

	sk_M1a,
	sk_M1b,
	sk_M2a,
	sk_M2b,
	sk_M3a,
	sk_M3b,
	sk_M4a,
	sk_M4b,
	sk_M5a,
	sk_M5b,
	sk_M6a,
	sk_M6b,
	sk_M7a,
	sk_M7b,
	sk_M8a,
	sk_M8b,
	sk_M9a,
	sk_M9b,
	sk_M10a,
	sk_M10b,

	skSing,
	sk_B1b,
	sk_B2a,
	sk_B2b,
	sk_B3a,
	sk_B3b,
	skReverberation,
	sk_B4b,
	sk_B5a,
	sk_B5b,
	sk_B6a,
	sk_B6b,
	sk_B7a,
	sk_B7b,
	sk_B8a,
	sk_B8b,
	sk_B9a,
	sk_B9b,
	sk_B10a,
	sk_B10b,

	skHide,
	sk_Ro1b,
	skBludgeon,
	skVenom,
	sk_Ro3a,
	sk_Ro3b,
	sk_Ro4a,
	sk_Ro4b,
	sk_Ro5a,
	sk_Ro5b,
	sk_Ro6a,
	sk_Ro6b,
	sk_Ro7a,
	sk_Ro7b,
	sk_Ro8a,
	sk_Ro8b,
	sk_Ro9a,
	sk_Ro9b,
	sk_Ro10a,
	sk_Ro10b,

	sk_Ra1a,
	sk_Ra1b,
	sk_Ra2a,
	sk_Ra2b,
	sk_Ra3a,
	sk_Ra3b,
	sk_Ra4a,
	sk_Ra4b,
	sk_Ra5a,
	sk_Ra5b,
	sk_Ra6a,
	sk_Ra6b,
	sk_Ra7a,
	sk_Ra7b,
	sk_Ra8a,
	sk_Ra8b,
	sk_Ra9a,
	sk_Ra9b,
	sk_Ra10a,
	sk_Ra10b,

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
char *Skill_Name(skill_id sk);

/* Get string representation of stat */
char *Stat_Name(statistic st);

#endif
