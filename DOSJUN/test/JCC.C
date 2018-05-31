/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "jc.h"
#include <stdio.h>

/* M A I N /////////////////////////////////////////////////////////////// */

int main(int argc, char **argv)
{
	int rvalue;
	jc_parser parser;

	printf("JunCode Compiler  Version 0.1  by Lag.Com <lagdotcom@gmail.com>\n");

	if (argc < 2) {
		printf("Syntax: jcc filename.jc\n");
		return 0;
	}

	Initialise_Parser(&parser);
	rvalue = Compile_JC(&parser, argv[1], true);

	Dump_Compiled_JC(&parser, "DUMP.JCC");
	Free_Parser(&parser);

	Stop_Memory_Tracking();
	return rvalue;
}
