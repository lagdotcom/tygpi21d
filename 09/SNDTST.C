/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define NUM_SOUNDS		5

#define DUCK_SOUND		0
#define BEE_SOUND		1
#define CAT_SOUND		2
#define FROG_SOUND		3
#define EXIT_SOUND		4

/* G L O B A L S ///////////////////////////////////////////////////////// */

char far *sounds[NUM_SOUNDS];
unsigned char lengths[NUM_SOUNDS];
unsigned ct_voice_status;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int done = 0;
	long index;

	/* Load the sound driver into memory. */
	printf("Voc_Load_Driver()\n");
	Voc_Load_Driver();
	if (driver_ptr == NULL) {
		printf("loading failed; exiting");
		return;
	}

	/* Initialize. */
	printf("Voc_Init_Driver()\n");
	Voc_Init_Driver();
	printf("Voc_Set_Port(0x220)\n");
	Voc_Set_Port(0x220);
	printf("Voc_Set_DMA(1)\n");
	Voc_Set_DMA(1);

	/* Print out the version. */
	printf("Voc_Get_Version()\n");
	Voc_Get_Version();
	printf("Voc_Set_Status_Addr()\n");
	Voc_Set_Status_Addr(&ct_voice_status);

	/* Load in the sounds. */
	sounds[DUCK_SOUND] = Voc_Load_Sound("duck.voc", &lengths[DUCK_SOUND]);
	sounds[BEE_SOUND]  = Voc_Load_Sound("bee.voc",  &lengths[BEE_SOUND]);
	sounds[CAT_SOUND]  = Voc_Load_Sound("cat.voc",  &lengths[CAT_SOUND]);
	sounds[FROG_SOUND] = Voc_Load_Sound("frog.voc", &lengths[FROG_SOUND]);
	sounds[EXIT_SOUND] = Voc_Load_Sound("exit.voc", &lengths[EXIT_SOUND]);

	Voc_Set_Speaker(1);

	while (!done) {
		if (kbhit()) {
			switch (getch()) {
				case '1':
					Voc_Stop_Sound();
					Voc_Play_Sound(sounds[DUCK_SOUND], lengths[DUCK_SOUND]);
					break;
				case '2':
					Voc_Stop_Sound();
					Voc_Play_Sound(sounds[BEE_SOUND], lengths[BEE_SOUND]);
					break;
				case '3':
					Voc_Stop_Sound();
					Voc_Play_Sound(sounds[CAT_SOUND], lengths[CAT_SOUND]);
					break;
				case '4':
					Voc_Stop_Sound();
					Voc_Play_Sound(sounds[FROG_SOUND], lengths[FROG_SOUND]);
					break;
				case 'q':
					done = 1;
					break;

				default: break;
			}
		}
	}

	/* Say goodbye. */
	Voc_Play_Sound(sounds[EXIT_SOUND], lengths[EXIT_SOUND]);

	/* Wait for the end of the sequence to stop. The status variable is -1
	when a sound is playing, and 0 otherwise. */
	while (ct_voice_status != 0) {}

	/* Turn the speaker off. */
	Voc_Set_Speaker(0);

	/* Unload sounds. */
	Voc_Unload_Sound(sounds[DUCK_SOUND]);
	Voc_Unload_Sound(sounds[BEE_SOUND]);
	Voc_Unload_Sound(sounds[CAT_SOUND]);
	Voc_Unload_Sound(sounds[FROG_SOUND]);
	Voc_Unload_Sound(sounds[EXIT_SOUND]);

	/* Unload the sound driver from memory. */
	Voc_Terminate_Driver();
}
