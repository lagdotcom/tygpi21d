/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef enum opt {
	oMusicVol,
	oSoundVol,
	MAX_OPTION,
} opt;

/* G L O B A L S ///////////////////////////////////////////////////////// */

options *gOptions;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

#define REDUCE(o) o = max(o - 1, 0)
#define INCREASE(o, _max) o = min(o + 1, _max);

void Read_Options(FILE *fp, options *o)
{
	fread(o, sizeof(options), 1, fp);
}

void Write_Options(FILE *fp, options *o)
{
	fwrite(o, sizeof(options), 1, fp);
}

noexport void Show_Option(int y, char *name, char *value, bool active)
{
	Draw_Font(8, y, WHITE, name, gFont, 0);
	Draw_Font(100, y, active ? YELLOW : WHITE, value, gFont, 0);
}

noexport void Show_Numeric_Option(int y, char *name, int value, bool active)
{
	char v[6];

	itoa(value, v, 10);
	Show_Option(y, name, v, active);
}

noexport void Draw_Options_Screen(int oindex)
{
	Fill_Double_Buffer(0);

	Draw_Font(8, 8, WHITE, "Options", gFont, 0);

	Show_Numeric_Option(24, "Music Volume", gOptions->music_vol, oindex == 0);
	Show_Numeric_Option(32, "Sound Volume", gOptions->sound_vol, oindex == 1);
}

void Show_Options_Screen(void)
{
	bool running = true;
	int fp_effect = current_fp_effect,
		oindex = 0;

	current_fp_effect = 0;
	redraw_everything = true;

	while (running) {
		Draw_Options_Screen(oindex);
		Show_Double_Buffer();

		switch (Get_Next_Scan_Code()) {
			case SCAN_ESC:
			case SCAN_Q:
				running = false;
				return;

			case SCAN_UP:
				oindex--;
				if (oindex < 0) oindex = MAX_OPTION;
				break;

			case SCAN_DOWN:
				oindex++;
				if (oindex >= MAX_OPTION) oindex = 0;
				break;

			case SCAN_MINUS:
			case SCAN_LEFT:
				switch (oindex) {
					case oMusicVol:
						REDUCE(gOptions->music_vol);
						break;

					case oSoundVol:
						REDUCE(gOptions->sound_vol);
						break;
				}
				break;

			case SCAN_EQUALS:
			case SCAN_RIGHT:
				switch (oindex) {
					case oMusicVol:
						INCREASE(gOptions->music_vol, MAX_VOLUME);
						break;

					case oSoundVol:
						INCREASE(gOptions->sound_vol, MAX_VOLUME);
						break;
				}
				break;
		}
	}

	current_fp_effect = fp_effect;
}
