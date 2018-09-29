/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "options.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

options *gOptions;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Read_Options(FILE *fp, options *o)
{
	fread(o, sizeof(options), 1, fp);
}

void Write_Options(FILE *fp, options *o)
{
	fwrite(o, sizeof(options), 1, fp);
}
