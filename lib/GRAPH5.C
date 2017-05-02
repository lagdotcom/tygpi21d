/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <math.h>
#include "graph3.h"
#include "graph5.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* Look-up tables for sine and cosine. */
static float sin_look[361],
             cos_look[361];

/* Polygon clipping extents */
static int poly_clip_min_x,
           poly_clip_min_y,
           poly_clip_max_x,
           poly_clip_max_y;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Bline(int xo, int yo, int x1, int y1, unsigned char color)
{
	/* This function uses Bresenham's algorithm (IBM 1965) to draw a line
	from (xo,yo) - (x1,y1). */

	int dx,					/* x difference */
		dy,					/* y difference */
		x_inc,				/* x increment */
		y_inc,				/* y increment */
		error = 0,			/* discriminant */
		index;

	/* Access the video buffer directly for speed. */
	unsigned char far *vb_start = video_buffer;

/* // S E C T I O N  1 /////////////////////////////////////////////////// */

	/* Precompute the first pixel address in the video buffer. Use shifts for
	multiplication. */
	vb_start += ((unsigned int)yo << 6) + ((unsigned int)yo << 8) +
		(unsigned int)xo;

	/* Compute the deltas. */
	dx = x1 - xo;
	dy = y1 - yo;

/* // S E C T I O N  2 /////////////////////////////////////////////////// */

	/* Test which direction the line is going in; i.e. the slope angle. */
	if (dx >= 0) {
		x_inc = 1;
	} else {
		x_inc = -1;
		dx = -dx;
	}

/* // S E C T I O N  3 /////////////////////////////////////////////////// */

	/* Test the y component of the slope. */
	if (dy >= 0) {
		y_inc = SCREEN_WIDTH;
	} else {
		y_inc = -SCREEN_WIDTH;
		dy = -dy;
	}

/* // S E C T I O N  4 /////////////////////////////////////////////////// */

	/* Now, based on which delta is greater, we can draw the line. */
	if (dx > dy) {
		/* Draw the line. */
		for (index = 0; index <= dx; index++) {
			/* Set the pixel */
			*vb_start = color;

			/* Adjust the disciminant. */
			error += dy;

			/* Test if error overflowed. */
			if (error > dx) {
				error -= dx;

				/* Move to the next line. */
				vb_start += y_inc;
			}

			vb_start += x_inc;
		}
	} else {
		for (index = 0; index <= dy; index++) {
			*vb_start = color;

			error += dx;

			if (error > dy) {
				error -= dy;

				vb_start += x_inc;
			}

			vb_start += y_inc;
		}
	}
}

void Draw_Polygon(polygon_ptr poly)
{
	/* This function draws a polygon on the screen without clipping. The
	caller should make sure that vertices are within bounds of the clipping
	rectangle. The polygon will always be unfilled regardless of the fill
	flag. */
	int index, xo, yo;

	/* Extract the local origin. */
	xo = poly->lxo;
	yo = poly->lyo;

	/* Draw the polygon. */
	for (index = 0; index < poly->num_vertices - 1; index++) {
		Bline(xo + (int)poly->vertices[index].x,
			yo + (int)poly->vertices[index].y,
			xo + (int)poly->vertices[index + 1].x,
			yo + (int)poly->vertices[index + 1].y,
			poly->b_color);
	}

	/* Close the polygon? */
	if (!poly->closed) {
		return;
	}

	Bline(xo + (int)poly->vertices[index].x,
		yo + (int)poly->vertices[index].y,
		xo + (int)poly->vertices[0].x,
		yo + (int)poly->vertices[0].y,
		poly->b_color);
}

void Translate_Polygon(polygon_ptr poly, int dx, int dy)
{
	/* Translate the origin of the polygon. */

	poly->lxo += dx;
	poly->lyo += dy;
}

void Scale_Polygon(polygon_ptr poly, float scale)
{
	/* Scale each vertex of the polygon. */
	int index;

	for (index = 0; index < poly->num_vertices; index++) {
		/* Multiply by the scaling factor. */
		poly->vertices[index].x *= scale;
		poly->vertices[index].y *= scale;
	}
}

void Create_Tables(void)
{
	/* This function creates the sine and cosine look-up tables. */
	int index;

	/* Create the tables. */
	for (index = 0; index <= 360; index++) {
		cos_look[index] = (float)cos((double)(index * M_PI / 180));
		sin_look[index] = (float)sin((double)(index * M_PI / 180));
	}
}

void Rotate_Polygon(polygon_ptr poly, int angle)
{
	/* Rotate each point of the polygon around its local origin.
	Note: angle is an integer and ranges from -360 to +360. */
	int index;			/* loop index */
	float si, cs,		/* values of sine and cosine */
	      rx, ry;		/* rotated points */

	/* Compute the sine and cosine of the angle to be rotated. */
	if (angle >= 0) {
		/* Extract the sine and cosine from the look-up table. */
		si = sin_look[angle];
		cs = cos_look[angle];
	} else {
		/* Angle is negative; convert to positive */
		si = sin_look[angle + 360];
		cs = cos_look[angle + 360];
	}

	/* Using values for sine and cosine, rotate the point. */
	for (index = 0; index < poly->num_vertices; index++) {
		/* Compute rotated values using rotation equations. */
		rx = poly->vertices[index].x * cs -
		     poly->vertices[index].y * si;
		ry = poly->vertices[index].y * cs +
		     poly->vertices[index].x * si;

		/* Store the rotated vertex back in the structure. */
			poly->vertices[index].x = rx;
			poly->vertices[index].y = ry;
	}
}

int Clip_Line(int *x1, int *y1, int *x2, int *y2)
{
	/* This function clips the sent line using the globally defined clipping
	region. */

	/* Track whether each endpoint is visible or invisible. */
	int point_1 = 0, point_2 = 0;

	int clip_always = 0;	/* Used for clipping ovveride. */
	int xi, yi;				/* Point of intersection. */
	int right_edge = 0,		/* Which edges are the endpoints beyond? */
		left_edge = 0,
		top_edge = 0,
		bottom_edge = 0;
	int success = 0;		/* Was clipping successful? */
	float dx, dy;			/* Used to hold slope deltas. */

/* // S E C T I O N  1 /////////////////////////////////////////////////// */

	/* Test whether line is completely visible. */
	if ((*x1 >= poly_clip_min_x) && (*x1 <= poly_clip_max_x) &&
		(*y1 >= poly_clip_min_y) && (*y1 <= poly_clip_max_y)) {
		point_1 = 1;
	}

	if ((*x2 >= poly_clip_min_x) && (*x2 <= poly_clip_max_x) &&
		(*y2 >= poly_clip_min_y) && (*y2 <= poly_clip_max_y)) {
		point_2 = 1;
	}

/* // S E C T I O N  2 /////////////////////////////////////////////////// */

	/* Test endpoints. */
	if (point_1 && point_2) {
		return 1;
	}

/* // S E C T I O N  3 /////////////////////////////////////////////////// */

	/* Test whether the line is completely invisible. */
	if (!point_1 && !point_2) {
		/* Must test to see whether each endpoint is on the same side of one
		of the bounding planes created by each clipping-region boundary. */

		/* To the left */
		if (((*x1 < poly_clip_min_x) && (*x2 < poly_clip_min_x)) ||
		/* To the right */
			((*x1 > poly_clip_max_x) && (*x2 > poly_clip_max_x)) ||
		/* Above */
			((*y1 < poly_clip_min_y) && (*y2 < poly_clip_min_y)) ||
		/* Below */
			((*y1 > poly_clip_max_y) && (*y2 > poly_clip_max_y))) {
			return 0;
		}

		/* If we got here, we have the special case where the line cuts into
		and out of the clipping region. */
		clip_always = 1;
	}

/* // S E C T I O N  4 /////////////////////////////////////////////////// */

	/* Take care of the case where either endpoint is in the clipping
	region. */
	if (point_1 || clip_always) {
		/* Compute the deltas. */
		dx = *x2 - *x1;
		dy = *y2 - *y1;

		/* Compute what boundary line must be clipped against. */
		if (*x2 > poly_clip_max_x) {
			/* Flag the right edge. */
			right_edge = 1;

			/* Compute intersection with the right edge. */
			if (dx != 0) {
				yi = (int)(.5 + (dy / dx) * (poly_clip_max_x - *x1) + *y1);
			} else {
				yi = -1;	/* Invalidate the intersection. */
			}
		} else if (*x2 < poly_clip_min_x) {
			/* Flag the left edge. */
			left_edge = 1;

			/* Compute intersection with the left edge. */
			if (dx != 0) {
				yi = (int)(.5 + (dy / dx) * (poly_clip_min_x - *x1) + *y1);
			} else {
				yi = -1;	/* Invalidate the intersection. */
			}
		}

		/* Horizontal intersections */
		if (*y2 > poly_clip_max_y) {
			/* Flag the bottom edge. */
			bottom_edge = 1;

			/* Compute intersection with the bottom edge. */
			if (dy != 0) {
				xi = (int)(.5 + (dx / dy) * (poly_clip_max_y - *y1) + *x1);
			} else {
				xi = -1;	/* Invalidate the intersection. */
			}
		} else if (*y2 < poly_clip_min_y) {
			/* Flag the top edge. */
			top_edge = 1;

			/* Compute intersection with the top edge. */
			if (dy != 0) {
				xi = (int)(.5 + (dx / dy) * (poly_clip_min_y - *y1) + *x1);
			} else {
				xi = -1;	/* Invalidate the intersection. */
			}
		}
	}

/* // S E C T I O N  5 /////////////////////////////////////////////////// */

	/* Now that we know where the line passed through, compute which edge is
	the proper intersection. */
	if (right_edge &&
		(yi >= poly_clip_min_y && yi <= poly_clip_max_y)) {
		*x2 = poly_clip_max_x;
		*y2 = yi;

		success = 1;
	} else if (left_edge &&
		(yi >= poly_clip_min_y && yi <= poly_clip_max_y)) {
		*x2 = poly_clip_min_x;
		*y2 = yi;

		success = 1;
	}

	if (bottom_edge &&
		(xi >= poly_clip_min_x && xi <= poly_clip_max_x)) {
		*x2 = xi;
		*y2 = poly_clip_max_y;

		success = 1;
	} else if (top_edge &&
		(xi >= poly_clip_min_x && xi <= poly_clip_max_x)) {
		*x2 = xi;
		*y2 = poly_clip_min_y;

		success = 1;
	}

/* // S E C T I O N  6 /////////////////////////////////////////////////// */

	/* Reset the edge flags. */
	right_edge = left_edge = top_edge = bottom_edge = 0;

	/* Test the second endpoint. */
	if (point_2 || clip_always) {
		/* Compute the deltas. */
		dx = *x1 - *x2;
		dy = *y1 - *y2;

		/* Compute what boundary line must be clipped against. */
		if (*x1 > poly_clip_max_x) {
			/* Flag the right edge. */
			right_edge = 1;

			/* Compute intersection with the right edge. */
			if (dx != 0) {
				yi = (int)(.5 + (dy / dx) * (poly_clip_max_x - *x2) + *y2);
			} else {
				yi = -1;	/* Invalidate the intersection. */
			}
		} else if (*x1 < poly_clip_min_x) {
			/* Flag the left edge. */
			left_edge = 1;

			/* Compute intersection with the left edge. */
			if (dx != 0) {
				yi = (int)(.5 + (dy / dx) * (poly_clip_min_x - *x2) + *y2);
			} else {
				yi = -1;	/* Invalidate the intersection. */
			}
		}

		/* Horizontal intersections */
		if (*y1 > poly_clip_max_y) {
			/* Flag the bottom edge. */
			bottom_edge = 1;

			/* Compute intersection with the bottom edge. */
			if (dy != 0) {
				xi = (int)(.5 + (dx / dy) * (poly_clip_max_y - *y2) + *x2);
			} else {
				xi = -1;	/* Invalidate the intersection. */
			}
		} else if (*y1 < poly_clip_min_y) {
			/* Flag the top edge. */
			top_edge = 1;

			/* Compute intersection with the top edge. */
			if (dy != 0) {
				xi = (int)(.5 + (dx / dy) * (poly_clip_min_y - *y2) + *x2);
			} else {
				xi = -1;	/* Invalidate the intersection. */
			}
		}
	}

/* // S E C T I O N  7 /////////////////////////////////////////////////// */

	/* Now that we know where the line passed through, compute which edge is
	the proper intersection. */
	if (right_edge &&
		(yi >= poly_clip_min_y && yi <= poly_clip_max_y)) {
		*x1 = poly_clip_max_x;
		*y1 = yi;

		success = 1;
	} else if (left_edge &&
		(yi >= poly_clip_min_y && yi <= poly_clip_max_y)) {
		*x1 = poly_clip_min_x;
		*y1 = yi;

		success = 1;
	}

	if (bottom_edge &&
		(xi >= poly_clip_min_x && xi <= poly_clip_max_x)) {
		*x1 = xi;
		*y1 = poly_clip_max_y;

		success = 1;
	} else if (top_edge &&
		(xi >= poly_clip_min_x && xi <= poly_clip_max_x)) {
		*x1 = xi;
		*y1 = poly_clip_min_y;

		success = 1;
	}

/* // S E C T I O N  8 /////////////////////////////////////////////////// */
	return success;
}

void Draw_Polygon_Clip(polygon_ptr poly)
{
	/* This function draws a polygon on the screen with clipping.
	The polygon will always be unfilled regardless of the fill flag. */
	int index,
		xo, yo,		/* local origin */
		x1, y1,		/* endpoints of the line being processed */
		x2, y2;

	/* Extract the local origin. */
	xo = poly->lxo;
	yo = poly->lyo;

	/* Draw the polygon. */
	for (index = 0; index < poly->num_vertices - 1; index++) {
		/* Extract the line. */
		x1 = (int)poly->vertices[index].x + xo;
		y1 = (int)poly->vertices[index].y + yo;

		x2 = (int)poly->vertices[index + 1].x + xo;
		y2 = (int)poly->vertices[index + 1].y + yo;

		/* Clip the line to the viewing screen and draw it, unless the line
		is totally invisible. */
		if (Clip_Line(&x1, &y1, &x2, &y2)) {
			Bline(x1, y1, x2, y2, poly->b_color);
		}
	}

	/* Close the polygon? */
	if (!poly->closed) {
		return;
	}

	/* Extract the line. */
	x1 = (int)poly->vertices[index].x + xo;
	y1 = (int)poly->vertices[index].y + yo;

	x2 = (int)poly->vertices[0].x + xo;
	y2 = (int)poly->vertices[0].y + yo;

	/* Clip the line to the viewing screen and draw it, unless the line
	is totally invisible. */
	if (Clip_Line(&x1, &y1, &x2, &y2)) {
		Bline(x1, y1, x2, y2, poly->b_color);
	}
}

void Set_Clipping_Region(int minx, int miny, int maxx, int maxy)
{
	/* lag: utility function */
	poly_clip_min_x = minx;
	poly_clip_min_y = miny;
	poly_clip_max_x = maxx;
	poly_clip_max_y = maxy;
}

