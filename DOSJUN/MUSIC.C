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

bool Play_Music(char *track)
{
	char filename[13];

	if (strcmp(track, active_track) != 0) {
		if (loaded) {
			Stop_Music();
		}

		sprintf(filename, "%s.SNG", track);

		Log("Play_Music: Loading %s", track);
		if (!Load_SNG(filename, &song)) {
			return false;
		}

		loaded = true;
		strcpy(active_track, track);
	}

	if (!playing) {
		Log("%s", "Play_Music: Playing");
		Start_SNG(&song);
		playing = true;
	}

	return true;
}

void Free_Music(void)
{
	Stop_Music();
}
