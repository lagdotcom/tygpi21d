/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "graph6.h"
#include "graph8.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* global pixel position and color */
int g_x = 0, g_y = 0;
unsigned char g_color = 0;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Plot_Pixel_Global(void)
{
	/* lag: This function uses global variables to plot pixels; this results
	in less stack access and quicker speed overall, at the cost of a bit of
	stupid-looking code. The macro Plot_DB helps a bit. */
	double_buffer[(g_y << 8) + (g_y << 6) + g_x] = g_color;
}
