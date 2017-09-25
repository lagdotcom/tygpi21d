/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"
#include "code.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture menu_bg;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Character(character *c, job_id job, int str,
	int intelligence, int dex, int hp, int mp)
{
	int i;

	c->experience = 0;
	c->total_level = 0;
	for (i = 0; i < NUM_JOBS; i++) {
		c->job_level[i] = 0;
	}

	c->stats[sStrength] = str;
	c->stats[sDexterity] = dex;
	c->stats[sIntelligence] = intelligence;
	c->stats[sHP] = c->stats[sMaxHP] = hp;
	c->stats[sMP] = c->stats[sMaxMP] = mp;

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

	gSave.script_globals = SzAlloc(MAX_GLOBALS, int, "Start_Campaign.globals");
	gSave.script_locals = SzAlloc(gSave.header.num_zones, int *, "Start_Campaign.locals");
	for (i = 0; i < gSave.header.num_zones; i++) {
		gSave.script_locals[i] = SzAlloc(MAX_LOCALS, int, "Start_Campaign.locals[i]");
	}

	Load_Campaign_Data();
}

gamestate Start_New_Game(void)
{
	Fill_Double_Buffer(0);

	Blit_String_DB(0,  0, 15, "You've always wanted to play one of", 0);
	Blit_String_DB(0,  8, 15, "those 'Escape the Room' games, so you", 0);
	Blit_String_DB(0, 16, 15, "get together a group of friends and go", 0);
	Blit_String_DB(0, 24, 15, "to a local one.", 0);
	gSave.header.num_characters = 6;

	Blit_String_DB(0, 40, 15, "Who's the bossy one?", 0);
	Input_String(168, 40, gSave.characters[0].name, NAME_SIZE);
	/*                                                  Str Int Dex  HP MP*/
	Initialise_Character(&gSave.characters[0], jBard,     8, 13, 13, 10, 0);

	Blit_String_DB(0, 56, 15, "Who's the strong one?", 0);
	Input_String(176, 56, gSave.characters[1].name, NAME_SIZE);
	Initialise_Character(&gSave.characters[1], jFighter, 14,  9, 11, 20, 0);

	Blit_String_DB(0, 72, 15, "Who's the nerd?", 0);
	Input_String(128, 72, gSave.characters[2].name, NAME_SIZE);
	Initialise_Character(&gSave.characters[2], jMage,     9, 14, 11,  8, 8);

	Blit_String_DB(0, 88, 15, "Who's kinda shifty?", 0);
	Input_String(160, 88, gSave.characters[3].name, NAME_SIZE);
	Initialise_Character(&gSave.characters[3], jRogue,    9, 11, 14, 12, 0);

	Blit_String_DB(0, 104, 15, "Who cares a lot?", 0);
	Input_String(136, 104, gSave.characters[4].name, NAME_SIZE);
	Initialise_Character(&gSave.characters[4], jCleric,  13, 13,  8, 14, 6);

	Blit_String_DB(0, 120, 15, "Who likes guns?", 0);
	Input_String(128, 120, gSave.characters[5].name, NAME_SIZE);
	Initialise_Character(&gSave.characters[5], jRanger,  13,  8, 13, 13, 0);

	Start_Campaign("ETR");
	trigger_on_enter = true;

	Save_Savefile("ETR.SAV", &gSave);
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
