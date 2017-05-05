/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "graph6.h"
#include "graph8.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* global pixel position and color */
int g_x = 0, g_y = 0;
unsigned char g_color = 0;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Plot_Pixel_Global(void)
{
	/* lag: This function uses global variables to plot pixels; this results
	in less stack access and quicker speed overall, at the cost of a bit of
	stupid-looking code. The macro Plot_DB helps a bit. */
	double_buffer[(g_y << 8) + (g_y << 6) + g_x] = g_color;
}

fixed Assign_Integer(long integer)
{
	/* This function assigns an integer to a fixed-point type by shifting. */
	return ((fixed)(integer << FP_SHIFT));
}

fixed Assign_Float(float number)
{
	/* This function assigns a floating-point number to a fixed-point type by
	multiplication, because it makes no sense to shift a floating-point data
	type. */
	return ((fixed)(number * FP_SHIFT_2N));
}

fixed Mul_Fixed(fixed f1, fixed f2)
{
	/* This function multiplies two fixed-point numbers and returns the
	result. Notice how the final result is shifted back. */
	return (f1 * f2) >> FP_SHIFT;
}

fixed Div_Fixed(fixed f1, fixed f2)
{
	/* This function divides two fixed-point numbers and returns the result.
	Notice how the dividend is preshifted before the division. */
	return (f1 << FP_SHIFT) / f2;
}

fixed Add_Fixed(fixed f1, fixed f2)
{
	/* This function adds two fixed-point numbers and returns the result.
	Notice how no shifting is necessary. */
	return f1 + f2;
}

fixed Sub_Fixed(fixed f1, fixed f2)
{
	/* This function subtracts two fixed-point numbers and returns the
	result. Notice how no shifting is necessary. */
	return f1 - f2;
}

void Print_Fixed(fixed f1)
{
	/* This function prints out a fixed-point number. It does this by
	extracting the portion to the left of the imaginary decimal point and
	then extracting the portion to the right of the imaginary decimal
	point. */
	printf("%ld.%ld", f1 >> FP_SHIFT,
		100 * (unsigned long)(f1 & FP_FRAC_MASK) / FP_SHIFT_2N);
}
