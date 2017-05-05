/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	fixed f1, f2, f3;

	f1 = Assign_Float((float)15.00);
	f2 = Assign_Float((float)233.45);

	printf("\nf1:=");
	Print_Fixed(f1);

	printf("\nf2:=");
	Print_Fixed(f2);

	printf("\nf1+f2:=");
	f3 = Add_Fixed(f1, f2);
	Print_Fixed(f3);

	printf("\nf1-f2:=");
	f3 = Sub_Fixed(f1, f2);
	Print_Fixed(f3);

	printf("\nf1*f2:=");
	f3 = Mul_Fixed(f1, f2);
	Print_Fixed(f3);

	printf("\nf1/f2:=");
	f3 = Div_Fixed(f1, f2);
	Print_Fixed(f3);
}
