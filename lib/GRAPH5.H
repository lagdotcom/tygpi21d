/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MAX_VERTICES	16

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct vertex_typ {
	float x,y;
} vertex, *vertex_ptr;

typedef struct polygon_typ {
	int b_color;	/* border */
	int i_color;	/* interior */
	int closed;
	int filled;
	int lxo, lyo;	/* local origin */
	int num_vertices;
	vertex vertices[MAX_VERTICES];
} polygon, *polygon_ptr;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Bline(int xo, int yo, int x1, int y1, unsigned char color);
int Clip_Line(int *x1, int *y1, int *x2, int *y2);
void Create_Tables(void);
void Draw_Polygon(polygon_ptr poly);
void Draw_Polygon_Clip(polygon_ptr poly);
void Rotate_Polygon(polygon_ptr poly, int angle);
void Scale_Polygon(polygon_ptr poly, float scale);
void Set_Clipping_Region(int minx, int miny, int maxx, int maxy);
void Translate_Polygon(polygon_ptr poly, int dx, int dy);
