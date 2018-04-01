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
	Start_SNG(&s);

	while (!kbhit());

	puts("Stopping...");
	Stop_SNG();
	Free_SNG(&s);
}
