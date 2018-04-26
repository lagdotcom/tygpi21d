/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include <dos.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define TIMER_INT		0x1c
#define TIMER_TICK_FREQ	(0x427d / 5)

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport void interrupt(*oldtimer)(void);

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void interrupt Timer_Tick(void)
{
	if (sng_playing)
		Continue_SNG();
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Initialise_Timer(void)
{
	Log("Initialise_Timer: %dHz", TIMER_TICK_FREQ);

	disable();
	oldtimer = getvect(TIMER_INT);
	setvect(TIMER_INT, Timer_Tick);

	outportb(0x43, 0x36);
	outportb(0x40, TIMER_TICK_FREQ & 0xff);
	outportb(0x40, (TIMER_TICK_FREQ & 0xff00) >> 8);
	enable();
}

void Free_Timer(void)
{
	Log("Free_Timer: %p", oldtimer);

	disable();
	setvect(TIMER_INT, oldtimer);

	outportb(0x43, 0x36);
	outportb(0x40, 0);
	outportb(0x40, 0);
	enable();
}

