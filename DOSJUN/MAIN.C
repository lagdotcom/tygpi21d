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
	gParty->zone = 0;
	gParty->encounter_chance = 0;
	gParty->danger = 1;

	gOptions = Find_File_Type(gSave, ftOptions);

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
	if (gParty->zone) {
		if (gZone->header.on_exit) Run_Code(gZone->header.on_exit);

		Fire_Event(evZoneExited, null);
		Expire_Listeners(eeZone);
	}

	gZone = Lookup_File_Chained(gDjn, id);
	if (gZone == null)
		dief("Move_to_Zone: could not find zone #%d", id);

	gParty->zone = id;

	if (!In_Djn(gSave, id, false)) {
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

	gOptions = Find_File_Type(gSave, ftOptions);

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
	bool first = true;
	char *menu[3];
	gamestate next;
	RGB_color black;

	menu[0] = "NEW GAME";
	menu[1] = "LOAD GAME";
	menu[2] = "QUIT";

	menu_bg = Lookup_File_Chained(gDjn, gCampaign->menubg_id);

	Start_Music(gCampaign->menumusic_id);

	black.red = 0; black.green = 0; black.blue = 0;
	Fill_Palette(&black);

	while (!done) {
		Fill_Double_Buffer(0);
		if (menu_bg)
			Draw_GRF(&gTopLeft, menu_bg, 0, 0);

		if (first) {
			first = false;
			Show_Double_Buffer();
			Fade_From(gPalette, &black, 3);
		}

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

	Stop_Music();

	if (menu_bg)
		Unload_File(gDjn, gCampaign->menubg_id);

	return next;
}
