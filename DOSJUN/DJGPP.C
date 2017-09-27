
#include "djgpp.h"

void Nearptr_Operation_Failed(void)
{
	/* TODO */
}

bool Enter_Nearptr_Mode(void)
{
	if (__djgpp_nearptr_enable() == 0) {
		Nearptr_Operation_Failed();
		return false;
	}

	return true;
}

char *Get_Nearptr(unsigned long offset)
{
	return (char *)(offset + __djgpp_conventional_base);
}
