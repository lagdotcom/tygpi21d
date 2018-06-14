/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"
#include "code.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport grf *menu_bg;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

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
	Run_Code(gCampaign->script_start);

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
			Draw_GRF(&topleft, menu_bg, 0, 0);

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
