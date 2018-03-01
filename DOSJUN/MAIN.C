/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"
#include "code.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture menu_bg;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Character(character *c, job job, int str,
	int intelligence, int dex, int hp, int mp)
{
	character_header *ch = &c->header;
	int i;

	ch->experience = 0;
	ch->total_level = 0;
	for (i = 0; i < NUM_JOBS; i++) {
		ch->job_level[i] = 0;
	}

	ch->stats[sStrength] = str;
	ch->stats[sDexterity] = dex;
	ch->stats[sIntelligence] = intelligence;
	ch->stats[sHP] = ch->stats[sMaxHP] = hp;
	ch->stats[sMP] = ch->stats[sMaxMP] = mp;

	c->skills = New_List(ltInteger, "Initialise_Character.skills");
	c->buffs = New_Object_List(sizeof(buff), "Initialise_Character.buffs");

	Set_Job(c, job);
}

void Load_Campaign_Data(void)
{
	char buffer[13];

	strcpy(buffer, gSave.header.campaign_name);
	strcat(buffer, ".ITM");
	Free_Items(&gItems);
	Load_Items(buffer, &gItems);

	strcpy(buffer, gSave.header.campaign_name);
	strcat(buffer, ".MON");
	Free_Monsters(&gMonsters);
	Load_Monsters(buffer, &gMonsters);

	strcpy(buffer, gCampaign.zones[gSave.header.zone]);
	strcat(buffer, ".ZON");
	Free_Zone(&gZone);
	Load_Zone(buffer, &gZone);

	Load_Textures(&gZone);
}

void Start_Campaign(char *name)
{
	char buffer[13];
	int i;

	strncpy(gSave.header.campaign_name, name, 8);

	strcpy(buffer, name);
	strcat(buffer, ".CMP");
	Load_Campaign(buffer, &gCampaign);

	gSave.header.zone = gCampaign.header.start_zone;
	gSave.header.x = gCampaign.header.start_x;
	gSave.header.y = gCampaign.header.start_y;
	gSave.header.facing = gCampaign.header.start_facing;
	gSave.header.num_zones = gCampaign.header.num_zones;
	gSave.header.encounter_chance = 0;
	gSave.header.danger = 1;

	gSave.script_globals = SzAlloc(MAX_GLOBALS, int, "Start_Campaign.globals");
	gSave.script_locals = SzAlloc(gSave.header.num_zones, int *, "Start_Campaign.locals");
	if (gSave.script_globals == null || gSave.script_locals == null) goto _dead;

	for (i = 0; i < gSave.header.num_zones; i++) {
		gSave.script_locals[i] = SzAlloc(MAX_LOCALS, int, "Start_Campaign.locals.i");
		if (gSave.script_locals[i] == null) goto _dead;
	}

	Load_Campaign_Data();
	return;

_dead:
	die("Start_Campaign: out of memory");
}

gamestate Start_New_Game(void)
{
	Fill_Double_Buffer(0);

	Draw_Wrapped_Font(0, 0, SCREEN_WIDTH, 32, WHITE, "You've always wanted to play one of those 'Escape the Room' games, so you get together a group of friends and go to a local one.", FNT, false);
	gSave.header.num_characters = 6;

	Draw_Font(0, 40, WHITE, "Who's the bossy one?", FNT, true);
	Input_String(168, 40, gSave.characters[0].header.name, NAME_SIZE);
	/*                                                  Str Int Dex  HP MP*/
	Initialise_Character(&gSave.characters[0], jBard,     8, 13, 13, 10, 0);

	Draw_Font(0, 56, WHITE, "Who's the strong one?", FNT, true);
	Input_String(176, 56, gSave.characters[1].header.name, NAME_SIZE);
	Initialise_Character(&gSave.characters[1], jFighter, 14,  9, 11, 20, 0);

	Draw_Font(0, 72, WHITE, "Who's the nerd?", FNT, true);;
	Input_String(128, 72, gSave.characters[2].header.name, NAME_SIZE);
	Initialise_Character(&gSave.characters[2], jMage,     9, 14, 11,  8, 8);

	Draw_Font(0, 88, WHITE, "Who's kinda shifty?", FNT, true);
	Input_String(160, 88, gSave.characters[3].header.name, NAME_SIZE);
	Initialise_Character(&gSave.characters[3], jRogue,    9, 11, 14, 12, 0);

	Draw_Font(0, 104, WHITE, "Who cares a lot?", FNT, true);
	Input_String(136, 104, gSave.characters[4].header.name, NAME_SIZE);
	Initialise_Character(&gSave.characters[4], jCleric,  13, 13,  8, 14, 6);

	Draw_Font(0, 120, WHITE, "Who likes guns?", FNT, true);
	Input_String(128, 120, gSave.characters[5].header.name, NAME_SIZE);
	Initialise_Character(&gSave.characters[5], jRanger,  13,  8, 13, 13, 0);

	Start_Campaign("ETR");
	trigger_on_enter = true;
	trigger_zone_enter = true;
	can_save = false;

	return gsDungeon;
}

bool Load_Game(void)
{
	char **filenames;
	char buffer[13];
	int choice,
		count;

	Fill_Double_Buffer(0);

	filenames = Get_Directory_Listing("*.SAV", &count);
	choice = Input_Menu(filenames, count, 0, 0);

	Load_Savefile(filenames[choice], &gSave);

	strcpy(buffer, gSave.header.campaign_name);
	strcat(buffer, ".CMP");
	Load_Campaign(buffer, &gCampaign);

	Load_Campaign_Data();
	gState = gsDungeon;
	trigger_on_enter = false;
	trigger_zone_enter = true;
	can_save = true;

	Free_Directory_Listing(filenames, count);

	return true;
}

/* M A I N /////////////////////////////////////////////////////////////// */

gamestate Show_Main_Menu(void)
{
	int option;
	bool done = false;
	char *menu[3];
	gamestate next;
	menu[0] = "NEW GAME";
	menu[1] = "LOAD GAME";
	menu[2] = "QUIT";

	PCX_Init(&menu_bg);
	PCX_Load("MAIN.PCX", &menu_bg, true);

	while (!done) {
		memcpy(double_buffer, menu_bg.buffer, SCREEN_WIDTH * SCREEN_HEIGHT);
		option = Input_Menu(menu, 3, 100, 140);

		switch (option) {
			case 0:
				next = Start_New_Game();
				done = true;
				break;

			case 1:
				if (Load_Game()) {
					done = true;
					next = gsDungeon;
				}
				break;

			case 2:
				next = gsQuit;
				done = true;
				break;
		}
	}

	PCX_Delete(&menu_bg);
	return next;
}
