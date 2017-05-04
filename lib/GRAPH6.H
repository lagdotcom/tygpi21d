/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "graph4.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* VGA status register 1 */
#define VGA_INPUT_STATUS_1	0x3DA
	/* bit 3: vsync -
		1: retrace in progress
		0: no retrace */

#define VGA_VSYNC_MASK		0x08
	/* bit 3 */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

int Create_Double_Buffer(int num_lines);
void Behind_Sprite_DB(sprite_ptr sprite);
void Delete_Double_Buffer(void);
void Draw_Sprite_DB(sprite_ptr sprite);
void Erase_Sprite_DB(sprite_ptr sprite);
void Fill_Double_Buffer(int color);
void Plot_Pixel_Fast_DB(int x, int y, unsigned char color);
void Show_Double_Buffer(void);
void Wait_For_Vsync(void);
