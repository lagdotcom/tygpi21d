/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include <dos.h>
#include "features.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define TIMER_INT		0x1c
#define PIT_FREQ		1193181.66666666666666666
#define TIMER_TOCK		((int)(PIT_FREQ / TIMER_TICK_COUNTER))

#define PIT_DATA0		0x40
#define PIT_CMD			0x43

#define CMD_BINARY		0
#define CMD_BCD			0x01
#define CMD_MODE0		0
#define CMD_MODE1		0x02
#define CMD_MODE2		0x04
#define CMD_MODE3		0x06
#define CMD_MODE4		0x08
#define CMD_MODE5		0x0A
#define CMD_LATCH		0
#define CMD_ACCESS_LO	0x10
#define CMD_ACCESS_HI	0x20
#define CMD_CHAN0		0
#define CMD_CHAN1		0x40
#define CMD_CHAN2		0x80
#define CMD_READBACK	0xC0

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport void interrupt(*oldtimer)(void);
bool clock_enabled = false;
noexport int clock_count;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Continue_Clock(void)
{
	if (++clock_count >= TIMER_TOCK) {
		gParty->seconds_elapsed++;
		clock_count = 0;
	}
}

noexport void interrupt Timer_Tick(void)
{
	if (sng_playing)
		Continue_SNG();

	if (current_fp_effect)
		Continue_Effect();

	if (clock_enabled)
		Continue_Clock();
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Initialise_Timer(void)
{
	Log("Initialise_Timer: c=%d, ~%dHz", TIMER_TICK_COUNTER, TIMER_TOCK);

	clock_count = 0;

	disable();
	oldtimer = getvect(TIMER_INT);
	setvect(TIMER_INT, Timer_Tick);

	outportb(PIT_CMD, CMD_BINARY | CMD_MODE3 | CMD_ACCESS_LO | CMD_ACCESS_HI | CMD_CHAN0);
	outportb(PIT_DATA0, TIMER_TICK_COUNTER % 256);
	outportb(PIT_DATA0, TIMER_TICK_COUNTER / 256);
	enable();
}

void Free_Timer(void)
{
	Log("Free_Timer: %p", oldtimer);

	disable();
	setvect(TIMER_INT, oldtimer);

	outportb(PIT_CMD, CMD_BINARY | CMD_MODE3 | CMD_ACCESS_LO | CMD_ACCESS_HI | CMD_CHAN0);
	outportb(PIT_DATA0, 0);
	outportb(PIT_DATA0, 0);
	enable();
}
