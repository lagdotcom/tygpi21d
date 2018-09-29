#include "dosjun.h"

noexport file_id song_id;

void Initialise_Music(void)
{
	Initialise_SNG_Player();
}

void Stop_Music(void)
{
	if (song_id) {
		Log("Stop_Music: %d", song_id);

		Stop_SNG();
		Unload_File(gDjn, song_id);
		song_id = 0;
	}
}

void Start_Music(file_id ref)
{
	sng *s;

	Stop_Music();

	if (!ref) {
		return;
	}

	Log("Start_Music: %d", ref);
	s = Lookup_File(gDjn, ref, false);
	if (s) {
		song_id = ref;
		Start_SNG(s);
	}
}

void Free_Music(void)
{
	Stop_Music();
	Free_SNG_Player();
}
