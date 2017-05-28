/* I N C L U D E S /////////////////////////////////////////////////////// */

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define NAME_SIZE		24
#define PARTY_SIZE		6
#define ZONE_WIDTH		30
#define ZONE_HEIGHT		20
#define ZONE_TEXT_SIZE	10000

#define true 1
#define false 0
#ifndef bool
typedef unsigned char bool;
#endif

#ifndef null
#define null 0
#endif

typedef enum direction_typ {
	North = 0,
	East = 1,
	South = 2,
	West = 3
} direction;

typedef enum relative_typ {
	Ahead,
	Left,
	Right
} relative;

typedef unsigned char colour;
typedef unsigned char coord;
typedef unsigned short text_offset;
typedef unsigned char zone_id;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct character_typ {
	char name[NAME_SIZE];
	unsigned short hp, maxhp;
	unsigned short mp, maxmp;
} character;

typedef struct party_typ {
	character characters[PARTY_SIZE];
	zone_id zone;
	direction facing;
	coord x, y;
} party;

typedef struct wall_typ {
	colour texture;
} wall;

typedef struct tile_typ {
	colour floor, ceil;
	wall walls[4];
	text_offset description;
} tile;

typedef struct zone_typ {
	tile tiles[ZONE_WIDTH][ZONE_HEIGHT];
	char text[ZONE_TEXT_SIZE];
} zone;

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern party P;
extern zone Z;

extern bool
	redraw_description,
	redraw_fp,
	redraw_party;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Try_Move_Forward(void);
bool Valid_Coord(coord x, coord y);
char Offset_X(direction dir);
char Offset_Y(direction dir);
tile* Tile_Ahead(coord x, coord y, direction dir, char multiple);
void Bline_DB(int xo, int yo, int x1, int y1, colour color);
void Blit_String_Box(int sx, int sy, int w, int h, colour col, char *string, bool trans_flag);
void Dline_DB(int xo, int yo, int x1, int y1, colour col);
void Draw_Character_Status(int index, int x, int y);
void Draw_Party_Status(void);
void Zone_Load(char *filename);
void Zone_Save(char *filename);
wall* Wall_Offset(coord x, coord y, direction dir, relative rel);
