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

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Behind_Sprite_DB(sprite_ptr sprite);
int Create_Double_Buffer(int num_lines);
void Delete_Double_Buffer(void);
void Draw_Sprite_Clipped_DB(sprite_ptr sprite, int min_x, int min_y,
	int max_x, int max_y);
void Draw_Sprite_DB(sprite_ptr sprite);
void Erase_Sprite_DB(sprite_ptr sprite);
void Fill_Double_Buffer(int color);
void Plot_Pixel_Fast_DB(int x, int y, unsigned char color);
void Show_Double_Buffer(void);
void Wait_For_Vsync(void);
