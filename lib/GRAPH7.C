/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <bios.h>
#include <dos.h>
#include "graph7.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define _KEYBRD_READ			0
#define _KEYBRD_READY			1
#define _KEYBRD_SHIFTSTATUS		2

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

unsigned int Get_Control_Keys(unsigned int mask)
{
	/* Return the status of all requested control keys. */
	return (mask & bioskey(_KEYBRD_SHIFTSTATUS));
}

unsigned char Get_Ascii_Key(void)
{
	/* If there's a normal ASCII key waiting, return it;
	otherwise, return 0. */
	if (bioskey(_KEYBRD_READY)) {
		return bioskey(_KEYBRD_READ);
	} else {
		return 0;
	}
}

unsigned char Get_Scan_Code(void)
{
	/* Get the scan code of a key press. Because we have to look at status
	bits, let's use the in-line assembler. */

	/* Is a key ready? */
	asm mov ah, _KEYBRD_READY;	/* function 1: is key ready? */
	asm int 16h;				/* call interrupt */
	asm jz empty;				/* there was no key, so exit */
	asm mov ah, _KEYBRD_READ;	/* function 0: get scan code */
	asm int 16h;				/* call interrupt */
	asm mov al, ah;				/* result is in ah, only use al */
	asm xor ah, ah;				/* zero ah */
	asm jmp done;				/* data's in ax, return */

empty:
	asm xor ax, ax;				/* clear out ax */
done:
	; /* data is returned, as it is in ax */
}

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

int Squeeze_Mouse(int command, int *x, int *y, int *buttons)
{
	/* Mouse interface. We use int86 instead of the in-line asm.
	(Why? No real reason...) */
	union REGS inregs, outregs;

	/* What function is the caller requesting? */
	inregs.x.ax = command;
	switch(command) {
		case MOUSE_RESET:
			int86(MOUSE_INT, &inregs, &outregs);

			/* return the number of buttons */
			*buttons = outregs.x.bx;
			return outregs.x.ax;

		case MOUSE_SHOW:
			/* This function increments the internal show-mouse counter.
			When it's equal to 0, the mouse is displayed. */
			int86(MOUSE_INT, &inregs, &outregs);
			return 1;

		case MOUSE_HIDE:
			/* This function decrements the internal show-mouse counter.
			When it's equal to -1, the mouse is hidden. */
			int86(MOUSE_INT, &inregs, &outregs);
			return 1;

		case MOUSE_BUTT_POS:
			/* This function gets the buttons and returns the absolute
			mouse positions in the vars x, y and buttons. */
			int86(MOUSE_INT, &inregs, &outregs);

			/* Extract the info and send it back. */
			*x       = outregs.x.cx;
			*y       = outregs.x.dx;
			*buttons = outregs.x.bx;
			return 1;

		case MOUSE_MOTION_REL:
			/* This function gets the relative mouse motions from the last
			call and puts them in the vars x, y. */
			int86(MOUSE_INT, &inregs, &outregs);

			/* Extract the info and send it back. */
			*x       = outregs.x.cx;
			*y       = outregs.x.dx;
			return 1;

		case MOUSE_SET_SENSITIVITY:
			/* This function sets the overall 'sensitivity' of the mouse.
			Each axis can have a sensitivity of 1-100, so the caller should
			put 1-100 in both x and y before calling. Also, 'buttons' is used
			to send in the doublespeed value, which also ranges from 1-100 */
			inregs.x.bx = *x;
			inregs.x.cx = *y;
			inregs.x.dx = *buttons;

			int86(MOUSE_INT, &inregs, &outregs);
			return 1;

		default: return 0;
	}
}
