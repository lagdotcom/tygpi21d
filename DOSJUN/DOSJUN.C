/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "gamelib.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture explore_bg;

campaign gCampaign;
gamestate gState;
items gItems;
monsters gMonsters;
save gSave;
zone gZone;

bool redraw_everything,
	redraw_description;
bool trigger_on_enter,
	trigger_zone_enter,
	just_moved,
	can_save;

font font6x8;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void dief(char *format, ...)
{
	va_list vargs;
	char buf[500];

	va_start(vargs, format);
	vsprintf(buf, format, vargs);
	va_end(vargs);

	die(buf);
}

char Get_X_Offset(dir dir)
{
	switch (dir) {
		case dEast: return 1;
		case dWest: return -1;
		default: return 0;
	}
}

char Get_Y_Offset(dir dir)
{
	switch (dir) {
		case dSouth: return 1;
		case dNorth: return -1;
		default: return 0;
	}
}

dir Turn_Left(dir dir)
{
	if (dir == dNorth) return dWest;
	return dir - 1;
}

dir Turn_Right(dir dir)
{
	if (dir == dWest) return dNorth;
	return dir + 1;
}

bool Is_Coord_Valid(coord x, coord y)
{
	if (x >= gZone.header.width) return false;
	if (y >= gZone.header.height) return false;
	return true;
}

tile* Get_Adjacent_Tile(coord x, coord y, dir dir, char multiple)
{
	coord ax = x + Get_X_Offset(dir) * multiple;
	coord ay = y + Get_Y_Offset(dir) * multiple;
	
	if (Is_Coord_Valid(ax, ay)) return TILE(gZone, ax, ay);
	return (tile*)null;
}

wall* Get_Wall(coord x, coord y, dir dir, relative rel)
{
	tile* under = TILE(gZone, x, y);

	switch (dir) {
		case dNorth:
			switch (rel) {
				case rLeft:  return &under->walls[dWest];
				case rAhead: return &under->walls[dNorth];
				case rRight: return &under->walls[dEast];
				default: return null;
			}

		case dEast:
			switch (rel) {
				case rLeft:  return &under->walls[dNorth];
				case rAhead: return &under->walls[dEast];
				case rRight: return &under->walls[dSouth];
				default: return null;
			}

		case dSouth:
			switch (rel) {
				case rLeft:  return &under->walls[dEast];
				case rAhead: return &under->walls[dSouth];
				case rRight: return &under->walls[dWest];
				default: return null;
			}

		case dWest:
			switch (rel) {
				case rLeft:  return &under->walls[dSouth];
				case rAhead: return &under->walls[dWest];
				case rRight: return &under->walls[dNorth];
				default: return null;
			}

		default: return null;
	}
}

void Show_Game_String(char *string, bool wait_for_key)
{
	Draw_Wrapped_Font(8, 144, 304, 48, 15, string, FNT, true);

	if (wait_for_key) {
		Show_Double_Buffer();
		Delay(5);
		Get_Next_Scan_Code();

		redraw_description = true;
	}
}

void Draw_Description(void)
{
	tile* under = TILE(gZone, gSave.header.x, gSave.header.y);

	if (under->description > 0) {
		Show_Game_String(gZone.strings[under->description - 1], false);
	}

	redraw_description = false;
}

bool Try_Move_Forward(void)
{
	wall *centre;
	tile *ahead;
	coord ax, ay;

	centre = Get_Wall(gSave.header.x, gSave.header.y, gSave.header.facing, rAhead);
	if (centre == null) {
		Show_Game_String("There's nothing to walk onto.", true);
		return false;
	}

	if (centre->texture) {
		switch (centre->type) {
			case wtNormal:
				Show_Game_String("Ow!", true);
				return false;

			case wtLockedDoor:
				Show_Game_String("The door is locked.", true);
				return false;
		}
	}

	ahead = Get_Adjacent_Tile(gSave.header.x, gSave.header.y, gSave.header.facing, 1);
	if (ahead == null) {
		Show_Game_String("There's nothing to walk onto.", true);
		return false;
	}
	if (ahead->flags & tiImpassable) {
		Show_Game_String("That way is blocked.", true);
		return false;
	}

	ax = gSave.header.x + Get_X_Offset(gSave.header.facing);
	ay = gSave.header.y + Get_Y_Offset(gSave.header.facing);

	gSave.header.x = ax;
	gSave.header.y = ay;
	redraw_description = true;
	redraw_fp = true;
	trigger_on_enter = true;
	just_moved = true;
	can_save = false;

	return true;
}

void Trigger_Enter_Script(void)
{
	tile* under = TILE(gZone, gSave.header.x, gSave.header.y);

	if (under->on_enter) {
		Run_Code(under->on_enter - 1);
	}

	trigger_on_enter = false;
}

void Trigger_Zone_Enter_Script(void)
{
	if (gZone.header.on_enter) {
		Run_Code(gZone.header.on_enter - 1);
	}

	trigger_zone_enter = false;
}

void Trigger_Zone_Move_Script(void)
{
	if (gZone.header.on_move) {
		Run_Code(gZone.header.on_move - 1);
	}
}

void Random_Encounter(encounter_id eid)
{
	gSave.header.encounter_chance = 0;
	Start_Combat(eid);
}

void Check_Random_Encounter(void)
{
	int i;
	tile* under = TILE(gZone, gSave.header.x, gSave.header.y);
	etable* et;
	if (under->etable == 0) return;

	if (gSave.header.encounter_chance < 200)
		gSave.header.encounter_chance += under->danger + gSave.header.danger;

	if (randint(0, 100) < gSave.header.encounter_chance) {
		et = &gZone.etables[under->etable - 1];
		for (i = 0; i < et->possibilities; i++) {
			if (randint(0, 100) < et->percentages[i]) {
				Random_Encounter(et->encounters[i]);
				return;
			}
		}
	}
}

void Redraw_Dungeon_Screen(bool script)
{
	if (redraw_everything) memcpy(double_buffer, explore_bg.buffer, SCREEN_WIDTH * SCREEN_HEIGHT);
	if (redraw_fp || redraw_everything) Draw_FP();
	if (redraw_party || redraw_everything) Draw_Party_Status();
	if (script && just_moved) {
		Check_Random_Encounter();
		Trigger_Zone_Move_Script();
	}
	if (script && trigger_zone_enter) Trigger_Zone_Enter_Script();
	if (script && trigger_on_enter) Trigger_Enter_Script();

	/* script might have set these */
	if (redraw_fp || redraw_everything) Draw_FP();
	if (redraw_party || redraw_everything) Draw_Party_Status();

	if (redraw_description || redraw_everything) Draw_Description();
	redraw_everything = false;

	Show_Double_Buffer();

	just_moved = false;
}

/* M A I N /////////////////////////////////////////////////////////////// */

gamestate Show_Dungeon_Screen(void)
{
	int done = 0;
	gamestate new;

	/* Get background image and palette */
	PCX_Init(&explore_bg);
	PCX_Load("BACK.PCX", &explore_bg, 1);

	redraw_everything = true;
	redraw_description = true;
	redraw_fp = true;
	redraw_party = true;

	while (!done) {
		if (just_moved) {
			Log("Show_Dungeon_Screen: at %d,%d", gSave.header.x, gSave.header.y);
		}

		Redraw_Dungeon_Screen(true);
		Delay(1);

		switch (Get_Next_Scan_Code()) {
			case SCAN_Q:
				new = gsQuit;
				done = 1;
				break;

			case SCAN_S:
				if (can_save) {
					if (Save_Savefile("ETR.SAV", &gSave)) {
						Show_Game_String("Game saved.", true);
					} else {
						Show_Game_String("Error while saving.", true);
					}
				} else {
					Show_Game_String("It's not safe here.", true);
				}
				break;

			case SCAN_LEFT:
				gSave.header.facing = Turn_Left(gSave.header.facing);
				trigger_on_enter = true;
				redraw_fp = true;
				break;

			case SCAN_RIGHT:
				gSave.header.facing = Turn_Right(gSave.header.facing);
				trigger_on_enter = true;
				redraw_fp = true;
				break;

			case SCAN_UP:
				Try_Move_Forward();
				break;

			/* TESTING ONLY */
			case SCAN_L:
				Level_Up(&gSave.characters[3]);
				break;
		}
	}

	/* Cleanup */
	PCX_Delete(&explore_bg);
	return new;
}

void main(void)
{
	Start_Memory_Tracking();
	Clear_Log();
	Log("main: Init");

	printf("Initialising DOSJUN...\n");
	Load_Font("6x8.FNT", &font6x8);
	Initialise_Campaign(&gCampaign);
	Initialise_Items(&gItems);
	Initialise_Monsters(&gMonsters);
	Initialise_Savefile(&gSave);
	Initialise_Zone(&gZone);
	Initialise_Combat();
	Initialise_Jobs();
	Initialise_Sound();

	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		dief("main: Not enough memory to create double buffer.");
		return;
	}

	printf("OK\n");

	Set_Video_Mode(VGA256);

	Log("main: Menu");
	gState = gsMainMenu;

	while (gState != gsQuit) {
		switch (gState) {
			case gsMainMenu:
				gState = Show_Main_Menu();
				break;

			case gsDungeon:
				gState = Show_Dungeon_Screen();
				break;
		}
	}

	Set_Video_Mode(TEXT_MODE);

	Log("main: Cleanup");
	printf("Cleaning up after DOSJUN...");

	Free_Campaign(&gCampaign);
	Free_Items(&gItems);
	Free_Monsters(&gMonsters);
	Free_Savefile(&gSave);
	Free_Zone(&gZone);
	Free_Textures();
	Free_Combat();
	Free_Jobs();
	Free_Sound();
	Delete_Picture();
	Free_Font(&font6x8);

	Delete_Double_Buffer();

	printf("OK!\n");
	Stop_Memory_Tracking();

	Log("main: Done");
}
