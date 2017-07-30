
#include "common.h"
#include "music.h"

void main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Syntax: mustest <file.sop>\n");
		return;
	}

	Initialise_Music();

	printf("Loading music...\n");
	if (!Play_Music(argv[1])) {
		printf("Something went wrong. Aborting.\n");
		Free_Music();
		Stop_Memory_Tracking();
		return;
	}

	printf("Hit any key to exit.\n");
	while (!kbhit()) 
		Process_Music();

	Stop_Music();
	Free_Music();

	Stop_Memory_Tracking();
}
