/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <dos.h>
#include <conio.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* 320x200x256 */
#define VGA256    0x13

/* The default text mode */
#define TEXT_MODE 0x03

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Set_Video_Mode(int mode)
{
	/* Use video interrupt 10h to set the video mode to the sent value */
	union REGS inregs, outregs;

	inregs.h.ah = 0;                    /* Set the video mode subfunction. */
	inregs.h.al = (unsigned char)mode;  /* Video mode to which to change.  */

	int86(0x10, &inregs, &outregs);
}

/* /////////////////////////////////////////////////////////////////////// */

void main(void)
{
	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Wait for a key to be hit. */
	while (!kbhit()) {}

	/* Put the computer back into text mode. */
	Set_Video_Mode(TEXT_MODE);
}
