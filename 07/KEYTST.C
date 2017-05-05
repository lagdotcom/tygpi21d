/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "graph3.h"
#include "graph7.h"

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	/* Keyboard demo. */
	unsigned char key, buffer[100];
	int done = 0;

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Show some info. */
	Blit_String(10, 10, 15, "Press a key and look at", 0);
	Blit_String(10, 20, 15, "the scan code it generates.", 0);
	Blit_String(10, 30, 15, "To exit the program press the 'q' key.", 0);

	/* Wait until the user presses the Q key. */
	while (!done) {
		/* Has the user pressed a key? */
		key = Get_Scan_Code();
		if (key) {
			sprintf(buffer, "Scan Code = %d  ", key);
			Blit_String(10, 50, 15, buffer, 0);

			sprintf(buffer, "Scan code interpreted as ASCII = %c", key);
			Blit_String(10, 60, 15, buffer, 0);
		}

		/* Test for ctrl and alt keys */
		if (Get_Control_Keys(CTRL)) {
			Blit_String(10, 70, 15, "control key pressed", 0);
		} else {
			Blit_String(10, 70, 15, "                   ", 0);
		}

		if (Get_Control_Keys(ALT)) {
			Blit_String(10, 80, 15, "alt key pressed", 0);
		} else {
			Blit_String(10, 80, 15, "               ", 0);
		}

		if (key == SCAN_Q) done = 1;
	}

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);
}
