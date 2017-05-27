/* I N C L U D E S /////////////////////////////////////////////////////// */

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define NAME_SIZE		24
#define PARTY_SIZE		6
#define ZONE_WIDTH		30
#define ZONE_HEIGHT		20

#define DIR_N			0
#define DIR_E			1
#define DIR_S			2
#define DIR_W			3

#define SEL_LEFT		-1
#define SEL_AHEAD		0
#define SEL_RIGHT		1

#define true 1
#define false 0
#ifndef bool
#define bool unsigned char
#endif

#ifndef null
#define null 0
#endif

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct character_typ {
	char name[NAME_SIZE];
} character;

typedef struct party_typ {
	character characters[PARTY_SIZE];
	char zone, x, y, facing;
} party;

typedef struct wall_typ {
	char texture;
} wall;

typedef struct tile_typ {
	char floor, ceil;
	wall walls[4];
} tile;

typedef struct zone_typ {
	tile tiles[ZONE_WIDTH][ZONE_HEIGHT];
} zone;

/* G L O B A L S ///////////////////////////////////////////////////////// */

party P;
zone Z;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */
