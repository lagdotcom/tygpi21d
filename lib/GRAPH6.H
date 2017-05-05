#ifndef GAMELIB_GRAPH_6
#define GAMELIB_GRAPH_6

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>
#include "graph4.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern unsigned char far *double_buffer;
extern unsigned int buffer_height;
extern unsigned int buffer_size;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

/* Double Buffer management */
int Create_Double_Buffer(int num_lines);
void Delete_Double_Buffer(void);
unsigned char far *Get_Double_Buffer(void);
void Show_Double_Buffer(void);

/* Double Buffer drawing */
void Behind_Sprite_DB(sprite_ptr sprite);
void Blit_Char_DB(int cx, int yc, char c, int color, int trans_flag);
void Blit_String_DB(int x, int y, int color, char *string, int trans_flag);
void Draw_Sprite_Clipped_DB(sprite_ptr sprite, int min_x, int min_y,
	int max_x, int max_y);
void Draw_Sprite_DB(sprite_ptr sprite);
void Erase_Sprite_DB(sprite_ptr sprite);
void Fill_Double_Buffer(int color);
void Plot_Pixel_Fast_DB(int x, int y, unsigned char color);

void Wait_For_Vsync(void);

/* Transitions */
void Dissolve(void);
void Fade_Lights(void);
void Melt(void);
void Shear(void);

#endif
