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

#define SKILL_SELECTED	YELLOW
#define SKILL_DESC		31

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

noexport void Add_Skill(pc *pc, skill_id sk)
{
	if (sk == skNONE) return;

	Log("Add_Skill: %s +%s", pc->name, skills[sk].name);
	Add_to_List(pc->skills, (void*)sk);
}

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

/* M A I N /////////////////////////////////////////////////////////////// */

void Initialise_Jobs(void)
{
	Setup_Job(jFighter, "Fighter", 10, 0);
	Setup_Job_Level(jFighter, 1, sNONE, skConcentrate, skNONE);
	Setup_Job_Level(jFighter, 2, sNONE, skCleave, skNONE);
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
	Setup_Job_Level(jBard, 4, sDexterity, skReverberation, skNONE);
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

	Setup_Skill(skConcentrate, "Concentrate", "Take your time.\nStrike carefully.");
	Setup_Skill(skCleave, "Cleave", "Continue the slaughter.\nDon't stop after a kill.");

	Setup_Skill(skSing, "Sing", "Inspire the party to greater deeds.\nAttacks will hit more often.");
	Setup_Skill(skReverberation, "Reverberation", "Singing is more effective and lasts a few rounds.");

	Setup_Skill(skHide, "Hide", "Retreat into the shadows.\nNext attack does bonus damage.");
	Setup_Skill(skBludgeon, "Bludgeon", "Carry a large stick.\nYour hidden strikes stun your target.");
	Setup_Skill(skVenom, "Venom", "Carry a small bag.\nYour hidden strikes poison your target.");
}

void Free_Jobs(void)
{
	Log("Free_Jobs: %p", jobspecs);
}

void Set_Job(pc *c, job job)
{
	pc_header *ch = &c->header;
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

noexport box2d chooseBox = {
	{ 8, Y_LEARN },
	{ SCREEN_WIDTH, SCREEN_HEIGHT },
};

noexport box2d chooseBoxA = {
	{ 8, Y_DESC },
	{ 8 + 144, Y_DESC + 64 },
};

noexport box2d chooseBoxB = {
	{ 168, Y_DESC },
	{ 168 + 144, Y_DESC + 64 },
};

noexport skill_id Choose_Skill_Menu(skill_id a, skill_id b)
{
	unsigned char ch;
	bool first = true;

	Show_Formatted_String("Choose a skill to learn:", 0, 0, &chooseBox, gFont, 0, false);

	Show_Formatted_String(skills[a].description, 0, 0, &chooseBoxA, gFont, SKILL_DESC, false);
	Show_Formatted_String(skills[b].description, 0, 0, &chooseBoxB, gFont, SKILL_DESC, false);

	while (true) {
		Draw_Font(  8, Y_SKILL, first ? SKILL_SELECTED : WHITE, skills[a].name, gFont, true);
		Draw_Font(168, Y_SKILL, first ? WHITE : SKILL_SELECTED, skills[b].name, gFont, true);
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

noexport box2d autoBox = {
	{ 8, Y_DESC },
	{ SCREEN_WIDTH - 8, Y_DESC + 64 },
};

/* Let the user choose which skill to learn on level up. Returns true if a screen redraw is needed. */
noexport bool Choose_Skill(pc *pc, skill_id a, skill_id b)
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
		sprintf(buffer, "%s learns %s!", pc->name, skills[learnt].name);
		Draw_Font(8, Y_SKILL, WHITE, buffer, gFont, false);

		Show_Formatted_String(skills[learnt].description, 0, 0, &autoBox, gFont, SKILL_DESC, false);
	}

	Add_Skill(pc, learnt);
	return immediate;
}

void Level_Up(pc *pc)
{
	pc_header *ch = &pc->header;
	unsigned char *level = &ch->job_level[ch->job];
	job_spec *j = &jobspecs[ch->job];
	level_spec *l;

	Log("Level_Up: %s %d+1", pc->name, ch->total_level);

	/* go to the level up screen */
	redraw_everything = true;
	Fill_Double_Buffer(0);	/* TODO: show LEVELUP.PCX or something? */

	ch->stats[sHP] += j->hp_per_level;
	ch->stats[sMaxHP] += j->hp_per_level;
	sprintf(buffer, "+%d HP", j->hp_per_level);
	Draw_Font(8, Y_HP, WHITE, buffer, gFont, 0);

	if (j->mp_per_level > 0) {
		ch->stats[sMP] += j->mp_per_level;
		ch->stats[sMaxMP] += j->mp_per_level;
		sprintf(buffer, "+%d MP", j->mp_per_level);
		Draw_Font(8, Y_MP, WHITE, buffer, gFont, 0);
	}

	if (ch->job_level[ch->job] < JOB_LEVELS) {
		l = &j->levels[*level];
		if (l->stat != sNONE) {
			ch->stats[l->stat]++;
			sprintf(buffer, "+1 %s", Stat_Name(l->stat));
			Draw_Font(8, Y_STAT, WHITE, buffer, gFont, 0);
		}

		sprintf(buffer, "%s becomes %s level %d!", pc->name, Job_Name(ch->job), *level + 1);
		Draw_Font(8, Y_LEVEL, WHITE, buffer, gFont, 0);

		if (Choose_Skill(pc, l->a, l->b)) {
			Show_Double_Buffer();
			Get_Next_Scan_Code();
		}

		(*level)++;
	} else {
		sprintf(buffer, "%s gains a level!", pc->name);
		Draw_Font(8, Y_LEVEL, WHITE, buffer, gFont, 0);

		Show_Double_Buffer();
		Get_Next_Scan_Code();
	}

	ch->experience = 0;
	ch->total_level++;
}

UINT32 Experience_to_Level(pc *pc)
{
	UINT32 clevel = pc->header.job_level[pc->header.job];

	/* TODO: xp penalties, better formula */
	if (clevel > 10) clevel = 10;
	return (clevel * 200) - 100;
}

void Add_Experience(pc *pc, UINT32 xp)
{
	pc_header *ch = &pc->header;

	ch->experience += xp;
	if (ch->experience >= Experience_to_Level(pc)) {
		Level_Up(pc);
	}
}
