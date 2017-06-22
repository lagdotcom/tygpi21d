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

bool redraw_description;
bool trigger_on_enter;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

char Get_X_Offset(direction dir)
{
	switch (dir) {
		case East: return 1;
		case West: return -1;
		default: return 0;
	}
}

char Get_Y_Offset(direction dir)
{
	switch (dir) {
		case South: return 1;
		case North: return -1;
		default: return 0;
	}
}

bool Is_Coord_Valid(coord x, coord y)
{
	if (x >= gZone.header.width) return false;
	if (y >= gZone.header.height) return false;
	return true;
}

tile* Get_Adjacent_Tile(coord x, coord y, direction dir, char multiple)
{
	coord ax = x + Get_X_Offset(dir) * multiple;
	coord ay = y + Get_Y_Offset(dir) * multiple;
	
	if (Is_Coord_Valid(ax, ay)) return TILE(gZone, ax, ay);
	return (tile*)null;
}

wall* Get_Wall(coord x, coord y, direction dir, relative rel)
{
	tile* under = TILE(gZone, x, y);

	switch (dir) {
		case North:
			switch (rel) {
				case rLeft:  return &under->walls[West];
				case rAhead: return &under->walls[North];
				case rRight: return &under->walls[East];
				default: return null;
			}

		case East:
			switch (rel) {
				case rLeft:  return &under->walls[North];
				case rAhead: return &under->walls[East];
				case rRight: return &under->walls[South];
				default: return null;
			}

		case South:
			switch (rel) {
				case rLeft:  return &under->walls[East];
				case rAhead: return &under->walls[South];
				case rRight: return &under->walls[West];
				default: return null;
			}

		case West:
			switch (rel) {
				case rLeft:  return &under->walls[South];
				case rAhead: return &under->walls[West];
				case rRight: return &under->walls[North];
				default: return null;
			}
	}

	return null;
}

void Show_Game_String(char *string, bool wait_for_key)
{
	Draw_Square_DB(0, 12, 148, 12 + 37*8, 148 + 5*8, 1);
	Draw_Bounded_String(12, 148, 37, 5, 15, string, 0);

	if (wait_for_key) {
		Show_Double_Buffer();
		Delay(5);
		Get_Next_Scan_Code();
	}

	redraw_description = true;
}

void Draw_Description(void)
{
	tile* under = TILE(gZone, gSave.header.x, gSave.header.y);

	Draw_Square_DB(0, 12, 148, 12 + 37*8, 148 + 5*8, 1);
	if (under->description > 0) {
		Draw_Bounded_String(12, 148, 37, 5, 15, gZone.strings[under->description - 1], 0);
	}

	redraw_description = false;
}

bool Try_Move_Forward(void)
{
	wall *centre;
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

	ax = gSave.header.x + Get_X_Offset(gSave.header.facing);
	ay = gSave.header.y + Get_Y_Offset(gSave.header.facing);

	gSave.header.x = ax;
	gSave.header.y = ay;
	redraw_description = true;
	redraw_fp = true;
	trigger_on_enter = true;

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

/* M A I N /////////////////////////////////////////////////////////////// */

gamestate Show_Dungeon_Screen(void)
{
	int done = 0;
	gamestate new;

	/* Get background image and palette */
	PCX_Init(&explore_bg);
	PCX_Load("BACK.PCX", &explore_bg, 1);
	memcpy(double_buffer, explore_bg.buffer, SCREEN_WIDTH * SCREEN_HEIGHT);

	redraw_description = true;
	redraw_fp = true;
	redraw_party = true;

	while (!done) {
		if (redraw_fp) Draw_FP();
		if (redraw_party) Draw_Party_Status();
		if (trigger_on_enter) Trigger_Enter_Script();

		if (redraw_description) Draw_Description();
		Show_Double_Buffer();
		Delay(1);

		switch (Get_Next_Scan_Code()) {
			case SCAN_Q:
				new = gsQuit;
				done = 1;
				break;

			case SCAN_LEFT:
				if (gSave.header.facing == North) gSave.header.facing = West;
				else gSave.header.facing--;
				trigger_on_enter = true;
				redraw_fp = true;
				break;

			case SCAN_RIGHT:
				if (gSave.header.facing == West) gSave.header.facing = North;
				else gSave.header.facing++;
				trigger_on_enter = true;
				redraw_fp = true;
				break;

			case SCAN_UP:
				Try_Move_Forward();
				break;
		}
	}

	/* Cleanup */
	PCX_Delete(&explore_bg);
	return new;
}

void main(void)
{
	printf("Initialising DOSJUN...");
	Initialise_Campaign(&gCampaign);
	Initialise_Items(&gItems);
	Initialise_Monsters(&gMonsters);
	Initialise_Savefile(&gSave);
	Initialise_Zone(&gZone);
	Initialise_Combat();

	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	printf("OK\n");

	Set_Video_Mode(VGA256);

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

	printf("Cleaning up after DOSJUN...");

	Free_Campaign(&gCampaign);
	Free_Items(&gItems);
	Free_Monsters(&gMonsters);
	Free_Savefile(&gSave);
	Free_Zone(&gZone);
	Free_Combat();
	Delete_Picture();

	Delete_Double_Buffer();

	printf("OK!\n");
	Stop_Memory_Tracking();
}
