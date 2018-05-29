/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"
#include "code.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

grf *menu_bg;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Character(pc *c, job job, int str,
	int intelligence, int dex, int hp, int mp)
{
	pc_header *ch = &c->header;
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

#if 0
noexport gamestate Start_New_Game(void)
{
	Fill_Double_Buffer(0);

	Run_Code(gCampaign->script_id);

	Draw_Wrapped_Font(0, 0, SCREEN_WIDTH, 32, WHITE, "You've always wanted to play one of those 'Escape the Room' games, so you get together a group of friends and go to a local one.", gFont, false);
	gSave->header.num_characters = 6;

	Draw_Font(0, 40, WHITE, "Who's the bossy one?", gFont, true);
	Input_String(168, 40, gSave->characters[0].header.name, NAME_SIZE);
	/*                                                  Str Int Dex  HP MP*/
	Initialise_Character(&gSave->characters[0], jBard,     8, 13, 13, 10, 0);

	Draw_Font(0, 56, WHITE, "Who's the strong one?", gFont, true);
	Input_String(176, 56, gSave->characters[1].header.name, NAME_SIZE);
	Initialise_Character(&gSave->characters[1], jFighter, 14,  9, 11, 20, 0);

	Draw_Font(0, 72, WHITE, "Who's the nerd?", gFont, true);;
	Input_String(128, 72, gSave->characters[2].header.name, NAME_SIZE);
	Initialise_Character(&gSave->characters[2], jMage,     9, 14, 11,  8, 8);

	Draw_Font(0, 88, WHITE, "Who's kinda shifty?", gFont, true);
	Input_String(160, 88, gSave->characters[3].header.name, NAME_SIZE);
	Initialise_Character(&gSave->characters[3], jRogue,    9, 11, 14, 12, 0);

	Draw_Font(0, 104, WHITE, "Who cares a lot?", gFont, true);
	Input_String(136, 104, gSave->characters[4].header.name, NAME_SIZE);
	Initialise_Character(&gSave->characters[4], jCleric,  13, 13,  8, 14, 6);

	Draw_Font(0, 120, WHITE, "Who likes guns?", gFont, true);
	Input_String(128, 120, gSave->characters[5].header.name, NAME_SIZE);
	Initialise_Character(&gSave->characters[5], jRanger,  13,  8, 13, 13, 0);

	Start_Campaign("ETR");

	return gsDungeon;
}
#endif

gamestate Start_New_Game(void)
{
	gSave = SzAlloc(1, djn, "Start_New_Game.save");
	if (!gSave) goto _dead;
	Initialise_Savefile(gSave);

	gGlobals = Find_File_Type(gSave, ftGlobals);
	Initialise_Globals(gGlobals, gCampaign);

	gParty = Find_File_Type(gSave, ftParty);
	gParty->encounter_chance = 0;
	gParty->danger = 1;

	redraw_everything = true;
	trigger_on_enter = true;
	trigger_zone_enter = true;
	can_save = false;

	Fill_Double_Buffer(0);
	gState = gsCutscene;
	Run_Code(gCampaign->script_id);

	return gsDungeon;

_dead:
	die("Start_New_Game: out of memory");
	return gsQuit;
}

void Move_to_Zone(file_id id)
{
	gZone = Lookup_File(gDjn, id);
	if (gZone == null)
		dief("Move_to_Zone: could not find zone #%d", id);

	gParty->zone = id;

	gSave->next = null;
	gOverlay = Lookup_File(gSave, id);
	gSave->next = gDjn;
	if (gOverlay == null) {
		gOverlay = SzAlloc(1, overlay, "Move_to_Zone.overlay");
		Initialise_Overlay(gOverlay, gZone);
		Add_to_Djn(gSave, gOverlay, id, ftOverlay);
	}
}

bool Load_Game(void)
{
	char **filenames;
	int choice,
		count;

	Fill_Double_Buffer(0);

	filenames = Get_Directory_Listing("*.SAV", &count);
	choice = Input_Menu(filenames, count, 0, 0);

	Load_Savefile(filenames[choice], gSave);

	/* TODO: check this is the right campaign for the save file!! */

	Move_to_Zone(gParty->zone);
	gState = gsDungeon;
	redraw_everything = true;
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
	sng s;
	gamestate next;
	menu[0] = "NEW GAME";
	menu[1] = "LOAD GAME";
	menu[2] = "QUIT";

	menu_bg = Lookup_File(gDjn, gCampaign->menubg_id);

	Load_SNG("ANTICAR.SNG", &s);
	Start_SNG(&s);

	while (!done) {
		Fill_Double_Buffer(0);
		if (menu_bg)
			Draw_GRF(0, 0, menu_bg, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

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

	Stop_SNG();
	Free_SNG(&s);

	if (menu_bg)
		Unload_File(gDjn, gCampaign->menubg_id);

	return next;
}
