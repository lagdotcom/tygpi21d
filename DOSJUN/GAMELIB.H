#ifndef GAMELIB_GRAPH
#define GAMELIB_GRAPH

#include "types.h"

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct RGB_color_typ {
	unsigned char red;      /* Red component of color 0-63 */
	unsigned char green;    /* Green component of color 0-63 */
	unsigned char blue;     /* Blue component of color 0-63 */
} RGB_color, *RGB_color_ptr;

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

#ifndef GAMELIB_GRAPH_4
#define GAMELIB_GRAPH_4

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define PCX_VER_25      0       /* ver 2.5 Paintbrush */
#define PCX_VER_28_PAL  2       /* ver 2.8 with palette */
#define PCX_VER_28      3       /* ver 2.8 use default palette */
#define PCX_VER_30      5       /* ver 3.0 or higher */

#define MAX_SPRITE_FRAMES   20  /* TODO */

#define SPRITE_DEAD		0

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct sprite_typ {
	int x, y;           /* position */
	int x_old, y_old;   /* old position */
	int width, height;  /* dimensions */
	int anim_clock;
	int anim_speed;
	int motion_clock;
	int motion_speed;

	char *frames[MAX_SPRITE_FRAMES];
	int curr_frame;
	int num_frames;
	int state;

	char *background;
	void *extra_data;
} sprite, *sprite_ptr;

typedef struct pcx_header_typ {
	INT8 manufacturer;          /* always 10 */
	INT8 version;               /* see PCX_VER_* */
	INT8 encoding;              /* 1: RLE encoding */
	INT8 bits_per_pixel;
	INT16 x, y;                   /* upper left corner */
	INT16 width, height;
	INT16 horz_res, vert_res;
	INT8 ega_palette[48];
	INT8 reserved;
	INT8 num_color_planes;
	INT16 bytes_per_line;
	INT16 palette_type;
	INT8 padding[58];
} pcx_header, *pcx_header_ptr;

typedef struct pcx_picture_typ {
	pcx_header header;
	RGB_color palette[256];
	char *buffer;
} pcx_picture, *pcx_picture_ptr;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void PCX_Delete(pcx_picture_ptr image);
void PCX_Grab_Bitmap(pcx_picture_ptr image, sprite_ptr sprite,
	int sprite_frame, int grab_x, int grab_y);
void PCX_Init(pcx_picture_ptr image);
void PCX_Load(char *filename, pcx_picture_ptr image, int enable_palette);
void PCX_Show_Buffer(pcx_picture_ptr image);

void Behind_Sprite(sprite_ptr sprite);
void Draw_Sprite(sprite_ptr sprite);
void Erase_Sprite(sprite_ptr sprite);
int Sprite_Collide(sprite_ptr sprite_1, sprite_ptr sprite_2);
void Sprite_Delete(sprite_ptr sprite);
void Sprite_Init(sprite_ptr sprite, int x, int y, int ac, int as,
	int mc, int ms, int width, int height);

#endif

#ifndef GAMELIB_GRAPH_6
#define GAMELIB_GRAPH_6

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern unsigned char *double_buffer;
extern unsigned int buffer_height;
extern unsigned int buffer_size;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

/* Double Buffer management */
int Create_Double_Buffer(int num_lines);
void Delete_Double_Buffer(void);
void Show_Double_Buffer(void);

/* Double Buffer drawing */
void Behind_Sprite_DB(sprite_ptr sprite);
void Blit_Char_DB(int xc, int yc, char c, int color, int trans_flag);
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

#ifndef GAMELIB_GRAPH_7K
#define GAMELIB_GRAPH_7K

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* bitmasks for control keys/shift status */
#define SHIFT_R				0x0001
#define SHIFT_L				0x0002
#define CTRL				0x0004
#define ALT					0x0008
#define SCROLL_LOCK_ON		0x0010
#define NUM_LOCK_ON			0x0020
#define CAPS_LOCK_ON		0x0040
#define INSERT_MODE			0x0080
#define CTRL_L				0x0100
#define ALT_L				0x0200
#define CTRL_R				0x0400
#define ALT_R				0x0800
#define SCROLL_LOCK_DWN		0x1000
#define NUM_LOCK_DWN		0x2000
#define CAPS_LOCK_DWN		0x4000
#define SYS_REQ_DWN			0x8000

/* Scan code values. Note: keys with two symbols on them are the same, so I
use the lower symbol. For example, the 1 key also has a ! above it, but we
would just call it the SCAN_1 key. */
#define SCAN_ESC			1
#define SCAN_1				2
#define SCAN_2				3
#define SCAN_3				4
#define SCAN_4				5
#define SCAN_5				6
#define SCAN_6				7
#define SCAN_7				8
#define SCAN_8				9
#define SCAN_9				10
#define SCAN_0				11
#define SCAN_MINUS			12
#define SCAN_EQUALS			13
#define SCAN_BKSP			14

#define SCAN_TAB			15
#define SCAN_Q				16
#define SCAN_W				17
#define SCAN_E				18
#define SCAN_R				19
#define SCAN_T				20
#define SCAN_Y				21
#define SCAN_U				22
#define SCAN_I				23
#define SCAN_O				24
#define SCAN_P				25
#define SCAN_LFT_BRACKET	26
#define SCAN_RGT_BRACKET	27
#define SCAN_ENTER			28
#define SCAN_CTRL			29

#define SCAN_A				30
#define SCAN_S				31
#define SCAN_D				32
#define SCAN_F				33
#define SCAN_G				34
#define SCAN_H				35
#define SCAN_J				36
#define SCAN_K				37
#define SCAN_L				38
#define SCAN_SEMI			39
#define SCAN_APOS			40
#define SCAN_TILDE			41

#define SCAN_LEFT_SHIFT		42
#define SCAN_BACK_SLASH		43
#define SCAN_Z				44
#define SCAN_X				45
#define SCAN_C				46
#define SCAN_V				47
#define SCAN_B				48
#define SCAN_N				49
#define SCAN_M				50
#define SCAN_COMMA			51
#define SCAN_PERIOD			52
#define SCAN_FORWARD_SLASH	53
#define SCAN_RIGHT_SHIFT	54

#define SCAN_PRT_SCRN		55
#define SCAN_ALT			56
#define SCAN_SPACE			57
#define SCAN_CAPS_LOCK		58
#define SCAN_F1				59
#define SCAN_F2				60
#define SCAN_F3				61
#define SCAN_F4				62
#define SCAN_F5				63
#define SCAN_F6				64
#define SCAN_F7				65
#define SCAN_F8				66
#define SCAN_F9				67
#define SCAN_F10			68
#define SCAN_F11			133
#define SCAN_F12			134
#define SCAN_NUM_LOCK		69
#define SCAN_SCROLL_LOCK	70
#define SCAN_HOME			71
#define SCAN_UP				72
#define SCAN_PGUP			73
#define SCAN_NUM_MINUS		74
#define SCAN_LEFT			75
#define SCAN_CENTER			76
#define SCAN_RIGHT			77
#define SCAN_NUM_PLUS		78
#define SCAN_END			79
#define SCAN_DOWN			80
#define SCAN_PGDWN			81
#define SCAN_INS			82
#define SCAN_DEL			83

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

unsigned char Get_Ascii_Key(void);
unsigned int Get_Control_Keys(unsigned int mask);
unsigned char Get_Scan_Code(void);

#endif
