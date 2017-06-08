/* I N C L U D E S /////////////////////////////////////////////////////// */

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

	Parser_Init(&parser);
	rvalue = JC_Compile(&parser, argv[1], true);

	JCC_Dump(&parser, "DUMP.JCC");
	Parser_Free(&parser);

	return rvalue;
}
