#ifndef DOSJUN_MUS_SNG_H
#define DOSJUN_MUS_SNG_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define NUM_INSTRUMENTS	32
#define NUM_CHANNELS	9

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct sng_inst {
	UINT8 freqmod;
	UINT8 feedback;
	UINT8 mod_attack;
	UINT8 mod_decay;
	UINT8 mod_sustain;
	UINT8 mod_release;
	UINT8 mod_volume;
	UINT8 mod_level_scale;
	UINT8 mod_multiplier;
	UINT8 mod_waveform;
	UINT8 mod_sustain_sound;
	UINT8 mod_scale;
	UINT8 mod_pitch_vibrato;
	UINT8 mod_volume_vibrato;
	UINT8 car_attack;
	UINT8 car_decay;
	UINT8 car_sustain;
	UINT8 car_release;
	UINT8 car_volume;
	UINT8 car_level_scale;
	UINT8 car_multiplier;
	UINT8 car_waveform;
	UINT8 car_sustain_sound;
	UINT8 car_scale;
	UINT8 car_pitch_vibrato;
	UINT8 car_volume_vibrato;
	UINT8 pitch_shift;
	char name[21];
} sng_inst;

typedef struct sng {
	char magic[4];
	char name[21];
	UINT8 channels;
	UINT8 order[256];
	UINT16 sep;
	sng_inst instruments[NUM_INSTRUMENTS];
	char *patterns;
} sng;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_SNG(sng *s);
bool Load_SNG(char *filename, sng *s);
void Start_SNG(sng *s);
void Stop_SNG(void);

#endif
