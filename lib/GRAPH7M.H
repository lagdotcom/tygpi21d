/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MOUSE_RESET				0x00	/* reset mouse */
#define MOUSE_SHOW				0x01	/* show mouse */
#define MOUSE_HIDE				0x02	/* hide mouse */
#define MOUSE_BUTT_POS			0x03	/* get buttons/position */
#define MOUSE_SET_SENSITIVITY	0x1A	/* set sensitivity 0-100 */
#define MOUSE_MOTION_REL		0x0B	/* get relative motion counters */

#define MOUSE_LEFT_BUTTON		0x01	/* left button mask */
#define MOUSE_RIGHT_BUTTON		0x02	/* right button mask */
#define MOUSE_CENTER_BUTTON		0x04	/* center button mask */

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

int Squeeze_Mouse(int command, int *x, int *y, int *buttons);
