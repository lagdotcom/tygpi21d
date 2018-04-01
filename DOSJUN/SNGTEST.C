#include "mus_sng.h"

void main(void)
{
	sng s;

	puts("Trying to read ANTICAR...");
	if (!Load_SNG("ANTICAR.SNG", &s)) {
		puts("Could not read SNG file.");
		return;
	}
	puts("OK");

	puts("Playing...");
	Start_Music(&s);

	while (!kbhit());

	puts("Stopping...");
	Stop_Music();
	Free_SNG(&s);
}
