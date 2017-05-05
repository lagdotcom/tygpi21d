#ifndef GAMELIB_GRAPH_3
#define GAMELIB_GRAPH_3

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct RGB_color_typ {
	unsigned char red;      /* Red component of color 0-63 */
	unsigned char green;    /* Green component of color 0-63 */
	unsigned char blue;     /* Blue component of color 0-63 */
} RGB_color, *RGB_color_ptr;

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* vram byte ptr */
extern unsigned char far *video_buffer;
extern unsigned short far *video_buffer_w;

/* 8x8 ROM characters */
extern unsigned char far *rom_char_set;

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define VGA256    0x13          /* 320x200x256 */
#define TEXT_MODE 0x03          /* The default text mode */

#define PALETTE_MASK        0x3c6
#define PALETTE_REGISTER_RD 0x3c7
#define PALETTE_REGISTER_WR 0x3c8
#define PALETTE_DATA        0x3c9

#define CHAR_WIDTH  8
#define CHAR_HEIGHT 8

#define SCREEN_WIDTH        (unsigned int)320
#define SCREEN_HEIGHT       (unsigned int)200

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Blit_Char(int cx, int yc, char c, int color, int trans_flag);
void Blit_String(int x, int y, int color, char *string, int trans_flag);
void Create_Cool_Palette(void);
void Delay(int clicks);
void Fill_Screen(int value);
void Get_Palette_Register(int index, RGB_color_ptr color);
unsigned char Get_Pixel(int x, int y);
void H_Line(int x1, int x2, int y, unsigned int color);
void H_Line_Fast(int x1, int x2, int y, unsigned int color);
void Plot_Pixel(int x, int y, char color);
void Plot_Pixel_Fast(int x, int y, unsigned char color);
void Set_Palette_Register(int index, RGB_color_ptr color);
void Set_Video_Mode(int mode);
void V_Line(int y1, int y2, int x, unsigned int color);

#endif
