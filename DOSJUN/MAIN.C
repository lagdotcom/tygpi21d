/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture menu_bg;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Init_Character(character *c, job_id job, unsigned short str,
	unsigned short intelligence, unsigned short dex, unsigned short hp,
	unsigned short mp)
{
	c->job = job;
	c->level = 1;
	c->stats[Strength] = str;
	c->stats[Dexterity] = dex;
	c->stats[Intelligence] = intelligence;
	c->stats[HP] = c->stats[MaxHP] = hp;
	c->stats[MP] = c->stats[MaxMP] = mp;
}

void New_Game()
{
	char buffer[13];
	Fill_Double_Buffer(0);

	Blit_String_DB(0,  0, 15, "You've always wanted to play one of", 0);
	Blit_String_DB(0,  8, 15, "those 'Escape the Room' games, so you", 0);
	Blit_String_DB(0, 16, 15, "get together a group of friends and go", 0);
	Blit_String_DB(0, 24, 15, "to a local one.", 0);

	Blit_String_DB(0, 40, 15, "Who are you?", 0);
	Input_String(104, 40, &S.characters[0].name, NAME_SIZE);
	Init_Character(&S.characters[0], Bard,     8, 13, 13, 10, 0);

	Blit_String_DB(0, 56, 15, "Who's the strong one?", 0);
	Input_String(176, 56, &S.characters[1].name, NAME_SIZE);
	Init_Character(&S.characters[1], Fighter, 14,  9, 11, 20, 0);

	Blit_String_DB(0, 72, 15, "Who's the nerd?", 0);
	Input_String(128, 72, &S.characters[2].name, NAME_SIZE);
	Init_Character(&S.characters[2], Mage,     9, 14, 11,  8, 8);

	Blit_String_DB(0, 88, 15, "Who's kinda shifty?", 0);
	Input_String(160, 88, &S.characters[3].name, NAME_SIZE);
	Init_Character(&S.characters[3], Rogue,    9, 11, 14, 12, 0);

	Blit_String_DB(0, 104, 15, "Who cares a lot?", 0);
	Input_String(136, 104, &S.characters[4].name, NAME_SIZE);
	Init_Character(&S.characters[4], Cleric,  13, 13,  8, 14, 6);

	Blit_String_DB(0, 120, 15, "Who likes guns?", 0);
	Input_String(128, 120, &S.characters[5].name, NAME_SIZE);
	Init_Character(&S.characters[5], Ranger,  13,  8, 13, 13, 0);

	G = State_Dungeon;

	Campaign_Load("DEMO.CMP", &C);
	S.header.num_characters = 6;

	strncpy(S.header.campaign_name, "DEMO", 8);
	S.header.zone = C.header.start_zone;
	S.header.x = C.header.start_x;
	S.header.y = C.header.start_y;
	S.header.facing = C.header.start_facing;

	strcpy(buffer, C.zones[C.header.start_zone]);
	strcat(buffer, ".ZON");

	Zone_Load(buffer, &Z);
	Savefile_Save("DEMO.SAV", &S);
}

bool Load_Game()
{
	return false;
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Main_Menu(void)
{
	int option;
	bool done = false;
	char *menu[3];
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
				New_Game();
				done = true;
				break;

			case 1:
				if (Load_Game()) done = true;
				break;

			case 2:
				G = State_Quit;
				done = true;
				break;
		}
	}

	PCX_Delete(&menu_bg);
	free(menu);
}
