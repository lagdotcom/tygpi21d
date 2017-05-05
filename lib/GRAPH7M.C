/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <dos.h>
#include "graph7m.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MOUSE_INT				0x33	/* mouse interrupt number */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

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
