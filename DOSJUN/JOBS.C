/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef enum {

} skill_id;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	stat_id stat;
	skill_id a;
	skill_id b;
} level_spec;

typedef struct {
	char *name;
	int hp_per_level;
	int mp_per_level;
	level_spec levels[JOB_LEVELS];
} job_spec;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport job_spec jobspecs[NUM_JOBS];

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Setup_Job(job_id job, char *name, int hp, int mp)
{
	jobspecs[job].name = name;
	jobspecs[job].hp_per_level = hp;
	jobspecs[job].mp_per_level = mp;
}

noexport void Setup_Job_Level(job_id job, int level, stat_id stat, skill_id sa, skill_id sb)
{
	jobspecs[job].levels[level - 1].stat = stat;
	jobspecs[job].levels[level - 1].a = sa;
	jobspecs[job].levels[level - 1].b = sb;
}

noexport void Add_Skill(character *c, skill_id sk)
{
	if (sk == -1) return;

	/* TODO */
}

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

/* M A I N /////////////////////////////////////////////////////////////// */

void Initialise_Jobs(void)
{
	Setup_Job(jFighter, "Fighter", 10, 0);
	Setup_Job_Level(jFighter, 1, -1, -1, -1);
	Setup_Job_Level(jFighter, 2, -1, -1, -1);
	Setup_Job_Level(jFighter, 3, -1, -1, -1);
	Setup_Job_Level(jFighter, 4, sStrength, -1, -1);
	Setup_Job_Level(jFighter, 5, -1, -1, -1);
	Setup_Job_Level(jFighter, 6, -1, -1, -1);
	Setup_Job_Level(jFighter, 7, -1, -1, -1);
	Setup_Job_Level(jFighter, 8, sStrength, -1, -1);
	Setup_Job_Level(jFighter, 9, -1, -1, -1);
	Setup_Job_Level(jFighter, 10, sStrength, -1, -1);

	Setup_Job(jCleric, "Cleric", 7, 3);
	Setup_Job_Level(jCleric, 1, -1, -1, -1);
	Setup_Job_Level(jCleric, 2, -1, -1, -1);
	Setup_Job_Level(jCleric, 3, -1, -1, -1);
	Setup_Job_Level(jCleric, 4, sIntelligence, -1, -1);
	Setup_Job_Level(jCleric, 5, -1, -1, -1);
	Setup_Job_Level(jCleric, 6, -1, -1, -1);
	Setup_Job_Level(jCleric, 7, -1, -1, -1);
	Setup_Job_Level(jCleric, 8, sStrength, -1, -1);
	Setup_Job_Level(jCleric, 9, -1, -1, -1);
	Setup_Job_Level(jCleric, 10, sIntelligence, -1, -1);

	Setup_Job(jMage, "Mage", 4, 4);
	Setup_Job_Level(jMage, 1, -1, -1, -1);
	Setup_Job_Level(jMage, 2, -1, -1, -1);
	Setup_Job_Level(jMage, 3, -1, -1, -1);
	Setup_Job_Level(jMage, 4, sIntelligence, -1, -1);
	Setup_Job_Level(jMage, 5, -1, -1, -1);
	Setup_Job_Level(jMage, 6, -1, -1, -1);
	Setup_Job_Level(jMage, 7, -1, -1, -1);
	Setup_Job_Level(jMage, 8, sIntelligence, -1, -1);
	Setup_Job_Level(jMage, 9, -1, -1, -1);
	Setup_Job_Level(jMage, 10, sIntelligence, -1, -1);

	Setup_Job(jBard, "Bard", 5, 0);
	Setup_Job_Level(jBard, 1, -1, -1, -1);
	Setup_Job_Level(jBard, 2, -1, -1, -1);
	Setup_Job_Level(jBard, 3, -1, -1, -1);
	Setup_Job_Level(jBard, 4, sDexterity, -1, -1);
	Setup_Job_Level(jBard, 5, -1, -1, -1);
	Setup_Job_Level(jBard, 6, -1, -1, -1);
	Setup_Job_Level(jBard, 7, -1, -1, -1);
	Setup_Job_Level(jBard, 8, sIntelligence, -1, -1);
	Setup_Job_Level(jBard, 9, -1, -1, -1);
	Setup_Job_Level(jBard, 10, sDexterity, -1, -1);

	Setup_Job(jRogue, "Rogue", 6, 0);
	Setup_Job_Level(jRogue, 1, -1, -1, -1);
	Setup_Job_Level(jRogue, 2, -1, -1, -1);
	Setup_Job_Level(jRogue, 3, -1, -1, -1);
	Setup_Job_Level(jRogue, 4, sDexterity, -1, -1);
	Setup_Job_Level(jRogue, 5, -1, -1, -1);
	Setup_Job_Level(jRogue, 6, -1, -1, -1);
	Setup_Job_Level(jRogue, 7, -1, -1, -1);
	Setup_Job_Level(jRogue, 8, sDexterity, -1, -1);
	Setup_Job_Level(jRogue, 9, -1, -1, -1);
	Setup_Job_Level(jRogue, 10, sDexterity, -1, -1);

	Setup_Job(jRanger, "Ranger", 6, 0);
	Setup_Job_Level(jRanger, 1, -1, -1, -1);
	Setup_Job_Level(jRanger, 2, -1, -1, -1);
	Setup_Job_Level(jRanger, 3, -1, -1, -1);
	Setup_Job_Level(jRanger, 4, sStrength, -1, -1);
	Setup_Job_Level(jRanger, 5, -1, -1, -1);
	Setup_Job_Level(jRanger, 6, -1, -1, -1);
	Setup_Job_Level(jRanger, 7, -1, -1, -1);
	Setup_Job_Level(jRanger, 8, sDexterity, -1, -1);
	Setup_Job_Level(jRanger, 9, -1, -1, -1);
	Setup_Job_Level(jRanger, 10, sStrength, -1, -1);
}

void Free_Jobs(void)
{
}

void Set_Job(character *c, job_id job)
{
	c->job = job;

	if (c->job_level[job] == 0) {
		Add_Skill(c, jobspecs[job].levels[0].a);
		Add_Skill(c, jobspecs[job].levels[0].b);

		c->job_level[job] = 1;
		c->total_level++;
	}
}

void Level_Up(character *c)
{
	unsigned char *level = &c->job_level[c->job];
	job_spec *j = &jobspecs[c->job];
	level_spec *l;

	c->stats[sHP] += j->hp_per_level;
	c->stats[sMaxHP] += j->hp_per_level;
	c->stats[sMP] += j->mp_per_level;
	c->stats[sMaxMP] += j->mp_per_level;

	if (c->job_level[c->job] < JOB_LEVELS) {
		l = &j->levels[*level];
		if (l->stat != -1) {
			c->stats[l->stat]++;
		}

		/* TODO: add skills */

		(*level)++;
	}

	c->experience = 0;
}

void Add_Experience(character *c, UINT32 xp)
{
	/* TODO: xp penalties, level up trigger */
	c->experience += xp;
}
