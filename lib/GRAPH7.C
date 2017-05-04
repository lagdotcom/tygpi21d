/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <bios.h>
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
