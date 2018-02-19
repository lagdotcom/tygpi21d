/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define sNONE (-1)
#define skNONE (-1)

#define Y_LEVEL	8
#define Y_HP	24
#define Y_MP	32
#define Y_STAT	40
#define Y_LEARN 56
#define Y_SKILL	64
#define Y_DESC	72

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport char buffer[100];
job_spec jobspecs[NUM_JOBS];
skill_spec skills[NUM_SKILLS];

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Setup_Job(job job, char *name, int hp, int mp)
{
	jobspecs[job].name = name;
	jobspecs[job].hp_per_level = hp;
	jobspecs[job].mp_per_level = mp;
}

noexport void Setup_Job_Level(job job, int level, statistic stat, skill_id sa, skill_id sb)
{
	jobspecs[job].levels[level - 1].stat = stat;
	jobspecs[job].levels[level - 1].a = sa;
	jobspecs[job].levels[level - 1].b = sb;
}

noexport void Setup_Skill(skill_id sk, char *name, char *description)
{
	skills[sk].name = name;
	skills[sk].description = description;
}

noexport void Add_Skill(character *c, skill_id sk)
{
	if (sk == skNONE) return;

	Log("Add_Skill: %s +%s", c->header.name, skills[sk].name);
	Add_to_List(c->skills, (void*)sk);
}

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

/* M A I N /////////////////////////////////////////////////////////////// */

void Initialise_Jobs(void)
{
	Setup_Job(jFighter, "Fighter", 10, 0);
	Setup_Job_Level(jFighter, 1, sNONE, skConcentrate, skNONE);
	Setup_Job_Level(jFighter, 2, sNONE, skNONE, skNONE);
	Setup_Job_Level(jFighter, 3, sNONE, skNONE, skNONE);
	Setup_Job_Level(jFighter, 4, sStrength, skNONE, skNONE);
	Setup_Job_Level(jFighter, 5, sNONE, skNONE, skNONE);
	Setup_Job_Level(jFighter, 6, sNONE, skNONE, skNONE);
	Setup_Job_Level(jFighter, 7, sNONE, skNONE, skNONE);
	Setup_Job_Level(jFighter, 8, sStrength, skNONE, skNONE);
	Setup_Job_Level(jFighter, 9, sNONE, skNONE, skNONE);
	Setup_Job_Level(jFighter, 10, sStrength, skNONE, skNONE);

	Setup_Job(jCleric, "Cleric", 7, 3);
	Setup_Job_Level(jCleric, 1, sNONE, skNONE, skNONE);
	Setup_Job_Level(jCleric, 2, sNONE, skNONE, skNONE);
	Setup_Job_Level(jCleric, 3, sNONE, skNONE, skNONE);
	Setup_Job_Level(jCleric, 4, sIntelligence, skNONE, skNONE);
	Setup_Job_Level(jCleric, 5, sNONE, skNONE, skNONE);
	Setup_Job_Level(jCleric, 6, sNONE, skNONE, skNONE);
	Setup_Job_Level(jCleric, 7, sNONE, skNONE, skNONE);
	Setup_Job_Level(jCleric, 8, sStrength, skNONE, skNONE);
	Setup_Job_Level(jCleric, 9, sNONE, skNONE, skNONE);
	Setup_Job_Level(jCleric, 10, sIntelligence, skNONE, skNONE);

	Setup_Job(jMage, "Mage", 4, 4);
	Setup_Job_Level(jMage, 1, sNONE, skNONE, skNONE);
	Setup_Job_Level(jMage, 2, sNONE, skNONE, skNONE);
	Setup_Job_Level(jMage, 3, sNONE, skNONE, skNONE);
	Setup_Job_Level(jMage, 4, sIntelligence, skNONE, skNONE);
	Setup_Job_Level(jMage, 5, sNONE, skNONE, skNONE);
	Setup_Job_Level(jMage, 6, sNONE, skNONE, skNONE);
	Setup_Job_Level(jMage, 7, sNONE, skNONE, skNONE);
	Setup_Job_Level(jMage, 8, sIntelligence, skNONE, skNONE);
	Setup_Job_Level(jMage, 9, sNONE, skNONE, skNONE);
	Setup_Job_Level(jMage, 10, sIntelligence, skNONE, skNONE);

	Setup_Job(jBard, "Bard", 5, 0);
	Setup_Job_Level(jBard, 1, sNONE, skSing, skNONE);
	Setup_Job_Level(jBard, 2, sNONE, skNONE, skNONE);
	Setup_Job_Level(jBard, 3, sNONE, skNONE, skNONE);
	Setup_Job_Level(jBard, 4, sDexterity, skNONE, skNONE);
	Setup_Job_Level(jBard, 5, sNONE, skNONE, skNONE);
	Setup_Job_Level(jBard, 6, sNONE, skNONE, skNONE);
	Setup_Job_Level(jBard, 7, sNONE, skNONE, skNONE);
	Setup_Job_Level(jBard, 8, sIntelligence, skNONE, skNONE);
	Setup_Job_Level(jBard, 9, sNONE, skNONE, skNONE);
	Setup_Job_Level(jBard, 10, sDexterity, skNONE, skNONE);

	Setup_Job(jRogue, "Rogue", 6, 0);
	Setup_Job_Level(jRogue, 1, sNONE, skHide, skNONE);
	Setup_Job_Level(jRogue, 2, sNONE, skBludgeon, skVenom);
	Setup_Job_Level(jRogue, 3, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRogue, 4, sDexterity, skNONE, skNONE);
	Setup_Job_Level(jRogue, 5, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRogue, 6, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRogue, 7, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRogue, 8, sDexterity, skNONE, skNONE);
	Setup_Job_Level(jRogue, 9, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRogue, 10, sDexterity, skNONE, skNONE);

	Setup_Job(jRanger, "Ranger", 6, 0);
	Setup_Job_Level(jRanger, 1, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRanger, 2, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRanger, 3, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRanger, 4, sStrength, skNONE, skNONE);
	Setup_Job_Level(jRanger, 5, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRanger, 6, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRanger, 7, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRanger, 8, sDexterity, skNONE, skNONE);
	Setup_Job_Level(jRanger, 9, sNONE, skNONE, skNONE);
	Setup_Job_Level(jRanger, 10, sStrength, skNONE, skNONE);

	Setup_Skill(skSing, "Sing", "Inspire the party to greater deeds.\nAttacks will hit more often.");

	Setup_Skill(skHide, "Hide", "Retreat into the shadows.\nNext attack does bonus damage.");
	Setup_Skill(skBludgeon, "Bludgeon", "Carry a large stick.\nYour hidden strikes stun your target.");
	Setup_Skill(skVenom, "Venom", "Carry a small bag.\nYour hidden strikes poison your target.");
}

void Free_Jobs(void)
{
}

void Set_Job(character *c, job job)
{
	character_header *ch = &c->header;
	ch->job = job;

	if (ch->job_level[job] == 0) {
		Add_Skill(c, jobspecs[job].levels[0].a);
		Add_Skill(c, jobspecs[job].levels[0].b);

		ch->job_level[job] = 1;
		ch->total_level++;
	}
}

char *Stat_Name(statistic st)
{
	switch (st) {
		case sMaxHP: return "HP";
		case sMaxMP: return "MP";
		case sMinDamage: return "Minimum Damage";
		case sMaxDamage: return "Maximum Damage";
		case sArmour: return "Armour";
		case sStrength: return "Strength";
		case sDexterity: return "Dexterity";
		case sIntelligence: return "Intelligence";
		case sHitBonus: return "Hit Bonus";
		case sDodgeBonus: return "Dodge Bonus";
		case sToughness: return "Toughness";
		default: return "???";
	}
}

noexport skill_id Choose_Skill_Menu(skill_id a, skill_id b)
{
	unsigned char ch;
	bool first = true;

	Draw_Font(8, Y_LEARN, 15, "Choose a skill to learn:", FNT, true);

	Draw_Wrapped_Font(  8, Y_DESC, 144, 64, 31, skills[a].description, FNT, false);
	Draw_Wrapped_Font(168, Y_DESC, 144, 64, 31, skills[b].description, FNT, false);

	while (true) {
		Draw_Font(  8, Y_SKILL, first ? 11 : 15, skills[a].name, FNT, true);
		Draw_Font(168, Y_SKILL, first ? 15 : 11, skills[b].name, FNT, true);
		Show_Double_Buffer();

		ch = Get_Next_Scan_Code();

		switch (ch) {
			case SCAN_ENTER:
				return first ? a : b;

			case SCAN_LEFT:
				first = true;
				break;

			case SCAN_RIGHT:
				first = false;
				break;
		}
	}
}

/* Let the user choose which skill to learn on level up. Returns true if a screen redraw is needed. */
noexport bool Choose_Skill(character *c, skill_id a, skill_id b)
{
	/* TODO: amnesia? */
	bool immediate = true;
	skill_id learnt = skNONE;

	if (a == skNONE) {
		if (b == skNONE) {
			return true;
		}

		learnt = b;
	} else {
		if (b != skNONE) {
			immediate = false;
			learnt = Choose_Skill_Menu(a, b);
		} else {
			learnt = a;
		}
	}

	if (immediate) {
		sprintf(buffer, "%s learns %s!", c->header.name, skills[learnt].name);
		Blit_String_DB(8, Y_SKILL, 15, buffer, 0);

		Draw_Wrapped_Font(8, Y_DESC, SCREEN_WIDTH - 16, 64, 31, skills[learnt].description, FNT, false);
	}

	Add_Skill(c, learnt);
	return immediate;
}

void Level_Up(character *c)
{
	character_header *ch = &c->header;
	unsigned char *level = &ch->job_level[ch->job];
	job_spec *j = &jobspecs[ch->job];
	level_spec *l;

	Log("Level_Up: %s %d+1", ch->name, ch->total_level);

	/* go to the level up screen */
	redraw_everything = true;
	Fill_Double_Buffer(0);	/* TODO: show LEVELUP.PCX or something? */

	ch->stats[sHP] += j->hp_per_level;
	ch->stats[sMaxHP] += j->hp_per_level;
	sprintf(buffer, "+%d HP", j->hp_per_level);
	Draw_Font(8, Y_HP, 15, buffer, FNT, 0);

	if (j->mp_per_level > 0) {
		ch->stats[sMP] += j->mp_per_level;
		ch->stats[sMaxMP] += j->mp_per_level;
		sprintf(buffer, "+%d MP", j->mp_per_level);
		Draw_Font(8, Y_MP, 15, buffer, FNT, 0);
	}

	if (ch->job_level[ch->job] < JOB_LEVELS) {
		l = &j->levels[*level];
		if (l->stat != sNONE) {
			ch->stats[l->stat]++;
			sprintf(buffer, "+1 %s", Stat_Name(l->stat));
			Draw_Font(8, Y_STAT, 15, buffer, FNT, 0);
		}

		sprintf(buffer, "%s becomes %s level %d!", ch->name, Job_Name(ch->job), *level + 1);
		Draw_Font(8, Y_LEVEL, 15, buffer, FNT, 0);

		if (Choose_Skill(c, l->a, l->b)) {
			Show_Double_Buffer();
			Get_Next_Scan_Code();
		}

		(*level)++;
	} else {
		sprintf(buffer, "%s gains a level!", ch->name);
		Draw_Font(8, Y_LEVEL, 15, buffer, FNT, 0);

		Show_Double_Buffer();
		Get_Next_Scan_Code();
	}

	ch->experience = 0;
	ch->total_level++;
}

void Add_Experience(character *c, UINT32 xp)
{
	character_header *ch = &c->header;

	/* TODO: xp penalties, level up trigger */
	ch->experience += xp;
}
