/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "gamelib.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport grf *explore_bg;

djn *gDjn;
campaign *gCampaign;
grf *gFont;
gamestate gState;
party *gParty;
djn *gSave;
strings *gStrings;
zone *gZone;
overlay *gOverlay;
globals *gGlobals;
palette *gPalette;

bool redraw_everything,
	redraw_description;
bool trigger_on_enter,
	trigger_zone_enter,
	just_moved,
	can_save;

point2d gTopLeft = { 0, 0 };
noexport box2d gameStringBox = {
	{ 12, 148 },
	{ 308, 188 },
};

noexport char strbuf[DOSJUN_BUFFER_SIZE];

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void dief(char *format, ...)
{
	va_list vargs;

	va_start(vargs, format);
	vsprintf(strbuf, format, vargs);
	va_end(vargs);

	die(strbuf);
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
	if (x >= gZone->header.width) return false;
	if (y >= gZone->header.height) return false;
	return true;
}

tile *Get_Adjacent_Tile(coord x, coord y, dir dir, char multiple)
{
	coord ax = x + Get_X_Offset(dir) * multiple;
	coord ay = y + Get_Y_Offset(dir) * multiple;
	
	if (Is_Coord_Valid(ax, ay)) return TILE(gZone, ax, ay);
	return (tile*)null;
}

wall *Get_Wall(coord x, coord y, dir dir, relative rel)
{
	tile *under = TILE(gZone, x, y);

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

void Show_Game_String(const char *string, bool wait_for_key)
{
	Show_Game_String_Context(string, wait_for_key, 0, 0);
}

void Show_Game_String_Context(const char *string, bool wait_for_key, file_id speaker, file_id target)
{
	Show_Formatted_String(string, speaker, target, &gameStringBox, gFont, 0, false);

	if (wait_for_key) {
		Show_Double_Buffer();
		Delay(5);
		Get_Next_Scan_Code();

		redraw_description = true;
	}
}

void Draw_Description(void)
{
	tile *under = TILE(gZone, gParty->x, gParty->y);

	if (under->description > 0) {
		Show_Game_String(Resolve_String(under->description), false);
	}

	redraw_description = false;
}

bool Try_Move_Forward(void)
{
	wall *centre;
	tile *ahead;
	coord ax, ay;

#if EXPLORE_DEBUG
	Log("Try_Move_Forward: at %d,%d facing %d", gParty->x, gParty->y, gParty->facing);
#endif

	centre = Get_Wall(gParty->x, gParty->y, gParty->facing, rAhead);
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

	ahead = Get_Adjacent_Tile(gParty->x, gParty->y, gParty->facing, 1);
	if (ahead == null) {
		Show_Game_String("There's nothing to walk onto.", true);
		return false;
	}
	if (ahead->flags & tiImpassable) {
		Show_Game_String("That way is blocked.", true);
		return false;
	}

	ax = gParty->x + Get_X_Offset(gParty->facing);
	ay = gParty->y + Get_Y_Offset(gParty->facing);
	gParty->steps_taken++;

	gParty->x = ax;
	gParty->y = ay;
	redraw_description = true;
	redraw_fp = true;
	trigger_on_enter = true;
	just_moved = true;
	can_save = false;

#if EXPLORE_DEBUG
	Log("Try_Move_Forward: moving to %d,%d", ax, ay);
#endif

	return true;
}

void Trigger_Enter_Script(void)
{
	tile *under = TILE(gZone, gParty->x, gParty->y);

	if (under->on_enter) {
#if EXPLORE_DEBUG
		Log("%s", "Trigger_Enter_Script");
#endif
		Run_Code(under->on_enter);
	}

	trigger_on_enter = false;
}

void Trigger_Use_Script(void)
{
	tile *under = TILE(gZone, gParty->x, gParty->y);

	if (under->on_use) {
#if EXPLORE_DEBUG
		Log("%s", "Trigger_Use_Script");
#endif
		Run_Code(under->on_use);
	}
}

void Trigger_Zone_Enter_Script(void)
{
	if (gZone->header.on_enter) {
#if EXPLORE_DEBUG
		Log("%s", "Trigger_Zone_Enter_Script");
#endif
		Run_Code(gZone->header.on_enter);
	}

	trigger_zone_enter = false;
}

void Trigger_Zone_Move_Script(void)
{
	if (gZone->header.on_move) {
#if EXPLORE_DEBUG
		Log("%s", "Trigger_Zone_Move_Script");
#endif
		Run_Code(gZone->header.on_move);
	}
}

void Random_Encounter(encounter_id eid)
{
	assert(eid < gZone->header.num_encounters, "Random_Encounter: encounter number too high");

	gParty->encounter_chance = 0;
	Start_Combat(eid);
}

bool Check_Party_Level(int min, int max)
{
	pc *p;
	int pl = 0;
	int i;

	if (min == 0 && max == 0)
		return true;

	for (i = 0; i < PARTY_SIZE; i++) {
		p = Get_PC(i);
		if (p) {
			pl += p->header.total_level;
		}
	}

	if (min && min > pl) return false;
	if (max && max < pl) return false;

	return true;
}

void Check_Random_Encounter(void)
{
	int i;
	tile *under = TILE(gZone, gParty->x, gParty->y);
	etable *et;
	encounter *ec;
	if (under->etable == 0) return;

	if (gParty->encounter_chance < 200)
		gParty->encounter_chance += under->danger + gParty->danger;

	if (randint(0, 100) < gParty->encounter_chance) {
		et = &gZone->etables[under->etable - 1];
		for (i = 0; i < et->possibilities; i++) {
			ec = &gZone->encounters[et->encounters[i]];
			if (!Check_Party_Level(ec->min_level, ec->max_level)) {
				continue;
			}

			if (randint(0, 100) < et->percentages[i]) {
				Random_Encounter(et->encounters[i]);
				return;
			}
		}
	}
}

void Redraw_Dungeon_Screen(bool script)
{
	if (redraw_everything) {
		Fill_Double_Buffer(0);
		if (explore_bg)
			Draw_GRF(&gTopLeft, explore_bg, 0, 0);
	}

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

noexport void Try_Get_Items(void)
{
	pcnum num;
	pc *pc;
	unsigned int i;
	itempos *ip;
	item *it;
	int remove_indices[DOSJUN_REMOVAL_LIST_SIZE];

	bool success = true;
	int j = 0;

	for (i = 0; i < gOverlay->items->size; i++)
	{
		ip = List_At(gOverlay->items, i);
		if (ip->x == gParty->x && ip->y == gParty->y) {
			if (Add_Item_to_Party(ip->item, 1, &num)) {
				pc = Lookup_File(gSave, gParty->members[num], true);
				it = Lookup_File(gDjn, ip->item, true);
				sprintf(strbuf, "%s gets %s.", pc->name, Resolve_String(it->name_id));

				/* null items are not drawn */
				ip->item = 0;
				Draw_FP();

				Show_Game_String(strbuf, true);
				remove_indices[j++] = i;

				/* don't break memory */
				if (j == DOSJUN_REMOVAL_LIST_SIZE)
					break;
			} else {
				success = false;
				break;
			}
		}
	}

	while (j > 0) {
		ip = List_At(gOverlay->items, remove_indices[--j]);
		Remove_from_List(gOverlay->items, ip);
	}

	if (!success) {
		Show_Game_String("You couldn't carry everything.", true);
	}
}

/* M A I N /////////////////////////////////////////////////////////////// */

gamestate Show_Dungeon_Screen(void)
{
	int done = 0;
	gamestate new;

	/* Get background image and palette */
	explore_bg = Lookup_File_Chained(gDjn, gCampaign->dungeonbg_id);

	redraw_everything = true;
	redraw_description = true;
	redraw_fp = true;
	redraw_party = true;

	while (!done) {
		if (just_moved) {
			Log("Show_Dungeon_Screen: at %d,%d", gParty->x, gParty->y);
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
					if (Save_Savefile("ETR.SAV", gSave)) {
						Show_Game_String("Game saved.", true);
					} else {
						Show_Game_String("Error while saving.", true);
					}
				} else {
					Show_Game_String("It's not safe here.", true);
				}
				break;

			case SCAN_LEFT:
				gParty->facing = Turn_Left(gParty->facing);
				trigger_on_enter = true;
				redraw_fp = true;
				break;

			case SCAN_RIGHT:
				gParty->facing = Turn_Right(gParty->facing);
				trigger_on_enter = true;
				redraw_fp = true;
				break;

			case SCAN_UP:
				Try_Move_Forward();
				break;

			case SCAN_1:
				Show_Pc_Screen(0);
				break;

			case SCAN_2:
				Show_Pc_Screen(1);
				break;

			case SCAN_3:
				Show_Pc_Screen(2);
				break;

			case SCAN_4:
				Show_Pc_Screen(3);
				break;

			case SCAN_5:
				Show_Pc_Screen(4);
				break;

			case SCAN_6:
				Show_Pc_Screen(5);
				break;

			case SCAN_SPACE:
				Trigger_Use_Script();
				break;

			case SCAN_G:
				Try_Get_Items();
				break;
		}
	}

	if (explore_bg)
		Unload_File(gDjn, gCampaign->dungeonbg_id);

	return new;
}

noexport bool Initialise_DB(void)
{
	double_buffer = Allocate(SCREEN_WIDTH, SCREEN_HEIGHT, "Initialise_DB");
	memset(double_buffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);

	Log("Initialise_DB: %p", double_buffer);

	return true;
}

noexport void Free_DB(void)
{
	Log("Free_DB: %p", double_buffer);

	Free(double_buffer);
}

noexport void Ready_Djn_Chain(int count, char **filenames)
{
	int i;
	djn *chain,
		*temp;

	gDjn = null;
	for (i = 1; i < count; i++) {
		temp = SzAlloc(1, djn, "Ready_Djn_Chain");
		
		if (Load_Djn(filenames[i], temp)) {
			if (gDjn == null) {
				gDjn = temp;
				chain = gDjn;
			} else {
				chain->next = temp;
				chain = chain->next;
			}
		}
	}
}

noexport void Free_Djn_Chain(void)
{
	djn *old,
		*d = gDjn;

	while (d) {
		Free_Djn(d);

		old = d;
		d = d->next;
		Free(old);
	}

	gDjn = null;
}

char *Resolve_String(int id)
{
	if (gStrings == null) {
		gStrings = Find_File_Type(gDjn, ftStrings);
		if (gStrings == null)
			die("Resolve_String: no STRINGS resource in DJN chain");
	}

	return Get_String(gStrings, id);
}

noexport void Load_Djn_Palette(void)
{
	gPalette = Find_File_Type(gDjn, ftPalette);
	if (gPalette) {
		Apply_Palette(gPalette);
		Log("%s", "Load_Djn_Palette: found");
	} else {
		Log("%s", "Load_Djn_Palette: not present");
	}
}

void main(int argc, char **argv)
{
	Start_Memory_Tracking();
	Clear_Log();
	Log("%s", "main: Init");

	printf("%s", "Initialising DOSJUN...\n");
	if (!Initialise_DB()) {
		dief("%s", "main: Not enough memory to create double buffer.");
		return;
	}

	Ready_Djn_Chain(argc, argv);
	if (gDjn == null) {
		printf("%s", "Syntax: DOSJUN <file.djn> [file.djn]...\n");
		Free_DB();
		Stop_Memory_Tracking();
		return;
	}

	gCampaign = Find_File_Type(gDjn, ftCampaign);
	if (gCampaign == null) {
		printf("%s", "main: No campaign data found in DJN files.\n");
		Free_Djn_Chain();
		Stop_Memory_Tracking();
		return;
	}

	gFont = Lookup_File_Chained(gDjn, gCampaign->font_id);
	if (gFont == null) {
		printf("%s", "main: No font data found in DJN files.\n");
		Free_Djn_Chain();
		Stop_Memory_Tracking();
		return;
	}

	Initialise_Timer();
	Initialise_Combat();
	Initialise_Buffs();
	Initialise_Jobs();
	Initialise_Sound();
	Initialise_Music();
	Initialise_Code();
	Initialise_Formatter();

	printf("OK\n");

	Set_Video_Mode(VGA256);
	Load_Djn_Palette();

	Log("%s", "main: Menu");
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

	Log("%s", "main: Cleanup");
	printf("%s", "Cleaning up after DOSJUN...");

	if (gSave)
		Free_Savefile(gSave);

	Free_Buffs();
	Free_Combat();
	Free_Jobs();
	Free_Sound();
	Free_Music();
	Free_Code();
	Free_Formatter();
	Free_Timer();

	Free_DB();
	Free_Djn_Chain();

	printf("%s", "OK!\n");
	Stop_Memory_Tracking();

	Log("%s", "main: Done");
}
