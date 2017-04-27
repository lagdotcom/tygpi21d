/* D E F I N E S ///////////////////////////////////////////////////////// */

#define PCX_VER_25      0       /* ver 2.5 Paintbrush */
#define PCX_VER_28_PAL  2       /* ver 2.8 with palette */
#define PCX_VER_28      3       /* ver 2.8 use default palette */
#define PCX_VER_30      5       /* ver 3.0 or higher */

#define MAX_SPRITE_FRAMES   20  /* TODO */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct sprite_typ {
	int x, y;           /* position */
	int x_old, y_old;   /* old position */
	int width, height;  /* dimensions */
	int anim_clock;
	int anim_speed;
	int motion_clock;
	int motion_speed;

	char far *frames[MAX_SPRITE_FRAMES];
	int curr_frame;
	int num_frames;
	int state;

	char far *background;
	void far* extra_data;
} sprite, *sprite_ptr;

typedef struct pcx_header_typ {
	char manufacturer;          /* always 10 */
	char version;               /* see PCX_VER_* */
	char encoding;              /* 1: RLE encoding */
	char bits_per_pixel;
	int x, y;                   /* upper left corner */
	int width, height;
	int horz_res, vert_res;
	char ega_palette[48];
	char reserved;
	char num_color_planes;
	int bytes_per_line;
	int palette_type;
	char padding[58];
} pcx_header, *pcx_header_ptr;

typedef struct pcx_picture_typ {
	pcx_header header;
	RGB_color palette[256];
	char far *buffer;
} pcx_picture, *pcx_picture_ptr;

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void PCX_Delete(pcx_picture_ptr image);
void PCX_Init(pcx_picture_ptr image);
void PCX_Load(char *filename, pcx_picture_ptr image, int enable_palette);
void PCX_Show_Buffer(pcx_picture_ptr image);
