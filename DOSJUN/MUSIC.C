#include "common.h"
#include "mus_sng.h"
#include <stdio.h>
#include <string.h>

noexport bool playing, loaded;
noexport char active_track[9];
noexport sng song;

void Initialise_Music(void)
{
	playing = false;
	loaded = false;
	active_track[0] = 0;
}

void Stop_Music(void)
{
	if (playing) {
		Log("%s", "Stop_Music: Stopping");
		Stop_SNG();
		playing = false;
	}

	if (loaded) {
		Log("%s", "Stop_Music: Freeing");
		Free_SNG(&song);
		loaded = false;
	}
}

void Free_Music(void)
{
	Stop_Music();
}
