/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <bios.h>
#include <dos.h>
#include "graph7j.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define JOYPORT				0x201

/* F U N C T I O N S ///////////////////////////////////////////////////// */

unsigned char Buttons(unsigned char button)
{
	/* Read the joystick buttons by peeking the port to which the switches
	are attached. */
	outp(JOYPORT, 0);

	/* Invert buttons, then mask with request. */
	return (~inp(JOYPORT)) & button;
}

unsigned char Buttons_Bios(unsigned char button)
{
	/* BIOS version of buttons read. */
	union REGS inregs, outregs;

	inregs.h.ah = 0x84;	/* joystick function */
	inregs.x.dx = 0x00;	/* read buttons subfunction */

	/* Call DOS */
	int86(0x15, &inregs, &outregs);

	/* Invert buttons, then mask with request. */
	return (~outregs.h.al) & button;
}

unsigned int Joystick(unsigned char stick)
{
	/* This function reads the joystick values manually by counting how long
	the capacitors take to charge/discharge. Let's use the in-line assembler.
	It's cool! */

	asm cli;					/* disable interrupts. */
	asm mov ah, byte ptr stick;	/* get mask into ah to select joystick */
	asm xor al, al;				/* zero out al */
	asm xor cx, cx;				/* same with cx, will be used as counter */
	asm mov dx, JOYPORT;		/* dx is used by inp and outp */
	asm out dx, al;				/* write 0 to port */

discharge:
	asm in al, dx;				/* read data from port */
	asm test al, ah;			/* has the bit changed? */
	asm loopne discharge;		/* if not ready, loop */

	asm sti;					/* re-enable interrupts */
	asm xor ax, ax;				/* zero out ax */
	asm sub ax, cx;				/* ax now holds axis switch position */
}

unsigned int Joystick_Bios(unsigned char stick)
{
	/* BIOS version of joystick read. */
	union REGS inregs, outregs;

	inregs.h.ah = 0x84;	/* joystick function */
	inregs.x.dx = 0x00;	/* read buttons subfunction */

	/* Call DOS */
	int86(0x15, &inregs, &outregs);

	/* Return the proper value depending on the sent command. */
	switch (stick)
	{
		case JOYSTICK_1_X: return outregs.x.ax;
		case JOYSTICK_1_Y: return outregs.x.bx;
		case JOYSTICK_2_X: return outregs.x.cx;
		case JOYSTICK_2_Y: return outregs.x.dx;
		default: return 0;
	}
}

int Joystick_Available(int stick_num)
{
	/* This function tests whether the joystick the user is requesting be
	tested is plugged in. */

	if (stick_num == JOYSTICK_1) {
		/* Test whether joystick 1 is plugged in by testing the port values.
		If there's no stick, they're 0,0. */
		return Joystick_Bios(JOYSTICK_1_X) + Joystick_Bios(JOYSTICK_1_Y);
	} else {
		return Joystick_Bios(JOYSTICK_2_X) + Joystick_Bios(JOYSTICK_2_Y);
	}
}
