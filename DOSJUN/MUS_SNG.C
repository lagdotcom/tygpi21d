/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "DOSJUN.H"
#include "MUS_SNG.h"
#include <stdio.h>
#include <dos.h>
#include <string.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SNG_MAGIC		"FMC!"
#define SNG_SEP			0x666
#define SNG_HEADER_LEN	0x71c

#define PATTERN_SIZE	64

#define ORDER_HALT		0xfe
#define ORDER_REPEAT	0xff

#define MAX_FREQ		1535

typedef enum effect {
	FX_ARPEGGIO = 0,
	FX_SLIDE_UP = 1,
	FX_SLIDE_DOWN = 2,
	FX_PORTAMENTO = 3,
	FX_VIBRATO = 4,
	FX_NOTE_OFF = 5,
	FX_VOLUME_SLIDE = 0xA,
	FX_ORDER_JUMP = 0xB,
	FX_VOLUME = 0xC,
	FX_BREAK = 0xD,
	FX_SPECIAL = 0xE,
	FX_SPEED = 0xF
} effect;

typedef enum special {
	SP_RETRIGGER = 0x9,
	SP_DELAY = 0xD,
} special;

typedef enum notestate {
	S_OFF = 0,
	S_PLAYED = 1,
	S_TRIGGER = 2,
	S_SOON = 3,
	S_RELEASE = 4
} notestate;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct player {
	sng *s;
	int stride;
	bool playing;
	UINT8 note_volume;
	UINT8 time_counter;
	UINT8 song_position;
	UINT8 patt_position;
	UINT8 tempo;
	UINT8 counter;
} player;

typedef struct chan {
	INT8 op1_ctrl;
	INT8 op1_volume;
	INT8 op1_attack;
	INT8 op1_sustain;
	INT8 op1_wave;
	INT8 op2_ctrl;
	INT8 op2_volume;
	INT8 op2_attack;
	INT8 op2_sustain;
	INT8 op2_wave;
	INT8 connect;
	INT8 lsb;
	INT8 msb;
	INT8 addvalue;
	INT8 volume;
	INT8 instr;
	INT8 effect1;
	INT8 effect2;
	INT8 effect3;
	INT8 note;
	INT8 noteslot;
	INT8 soundon;
	INT8 arpeggio;
	INT8 tonedir;
	INT8 tonespeed;
	INT8 vibdepth;
	INT8 vibspeed;
	INT8 vibdir;
	INT8 vibcounter;
	INT16 vibrato;
	INT16 target;
	INT16 freq;
	INT16 basefreq;
} chan;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport UINT16 note_frequencies[] = {
	344,345,346,348,349,350,351,352,354,355,356,357,358,359,361,362,
	363,364,366,367,369,370,371,373,374,375,377,378,380,381,382,384,
	385,386,388,389,391,392,394,395,397,398,399,401,402,404,405,407,
	408,410,411,413,414,416,417,419,420,422,423,425,426,428,429,431,
	432,434,435,437,439,440,442,443,445,447,448,450,452,453,455,456,
	458,460,461,463,465,466,468,470,472,473,475,477,478,480,482,483,
	485,487,489,490,492,494,496,498,500,501,503,505,507,509,510,512,
	514,516,518,520,522,523,525,527,529,531,533,535,537,538,540,542,
	544,546,548,550,552,554,556,558,561,563,565,567,569,571,573,575,
	577,579,581,583,586,588,590,592,594,596,598,600,603,605,607,609,
	611,613,616,618,620,622,625,627,629,631,634,636,638,640,643,645,
	647,649,652,654,657,659,662,664,667,669,671,674,676,679,681,684
};

noexport UINT8 adlib_regs[] = {
	0x20,0x40,0x60,0x80,0xe0,0x23,0x43,0x63,0x83,0xe3,0xc0,0xa0,0xb0,
	0x21,0x41,0x61,0x81,0xe1,0x24,0x44,0x64,0x84,0xe4,0xc1,0xa1,0xb1,
	0x22,0x42,0x62,0x82,0xe2,0x25,0x45,0x65,0x85,0xe5,0xc2,0xa2,0xb2,
	0x28,0x48,0x68,0x88,0xe8,0x2b,0x4b,0x6b,0x8b,0xeb,0xc3,0xa3,0xb3,
	0x29,0x49,0x69,0x89,0xe9,0x2c,0x4c,0x6c,0x8c,0xec,0xc4,0xa4,0xb4,
	0x2a,0x4a,0x6a,0x8a,0xea,0x2d,0x4d,0x6d,0x8d,0xed,0xc5,0xa5,0xb5,
	0x30,0x50,0x70,0x90,0xf0,0x33,0x53,0x73,0x93,0xf3,0xc6,0xa6,0xb6,
	0x31,0x51,0x71,0x91,0xf1,0x34,0x54,0x74,0x94,0xf4,0xc7,0xa7,0xb7,
	0x32,0x52,0x72,0x92,0xf2,0x35,0x55,0x75,0x95,0xf5,0xc8,0xa8,0xb8
};

noexport player p;
noexport chan *channels;
noexport char *ghost_regs;
noexport char *instrument_data;

bool sng_playing = false;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport long filesize(FILE *fp)
{
	long size, pos;

	pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	return size;
}

bool Read_SNG(FILE *fp, sng *s)
{
	long pattern_data;

	fread(s, SNG_HEADER_LEN, 1, fp);
	if (strncmp(SNG_MAGIC, s->magic, 4) != 0) {
		fclose(fp);
		die("Not a FMC/SNG file");
		return false;
	}

	pattern_data = filesize(fp) - SNG_HEADER_LEN;
	s->patterns = Allocate(1, pattern_data, "Load_SNG");

	/* TODO: fix warning (fread takes size_t, not long) */
	fread(s->patterns, pattern_data, 1, fp);

	return true;
}

bool Load_SNG(char *filename, sng *s)
{
	bool result;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		dief("Load_SNG: Could not open for reading: %s\n", filename);
		return false;
	}

	result = Read_SNG(fp, s);
	fclose(fp);
	return result;
}

void Free_SNG(sng *s)
{
	Free(s->patterns);
}

noexport void Convert_Instruments(sng_inst *x)
{
	int i;
	UINT8 temp;
	char *idata = instrument_data;
	for (i = 0; i < NUM_INSTRUMENTS; i++) {
		temp = 1;
		if (x[i].freqmod) temp--;
		temp |= x[i].feedback << 1;
		idata[10] = temp;
		idata[13] = x[i].pitch_shift;

		/* modulator data */
		idata[2] = (x[i].mod_attack << 4) | x[i].mod_decay;
		idata[3] = ((15 - x[i].mod_sustain) << 4) | x[i].mod_release;
		idata[1] = (63 - x[i].mod_volume) | (x[i].mod_level_scale);
		idata[4] = x[i].mod_waveform;

		temp = x[i].mod_multiplier;
		if (x[i].mod_sustain_sound) temp |= 0x20;
		if (x[i].mod_scale) temp |= 0x10;
		if (x[i].mod_pitch_vibrato) temp |= 0x40;
		if (x[i].mod_volume_vibrato) temp |= 0x80;
		idata[0] = temp;

		/* carrier data */
		idata[7] = (x[i].car_attack << 4) | x[i].car_decay;
		idata[8] = ((15 - x[i].car_sustain) << 4) | x[i].car_release;
		idata[6] = (63 - x[i].car_volume) | (x[i].car_level_scale);
		idata[9] = x[i].car_waveform;

		temp = x[i].car_multiplier;
		if (x[i].car_sustain_sound) temp |= 0x20;
		if (x[i].car_scale) temp |= 0x10;
		if (x[i].car_pitch_vibrato) temp |= 0x40;
		if (x[i].car_volume_vibrato) temp |= 0x80;
		idata[5] = temp;

		idata += 16;
	}
}

noexport void Write(UINT8 reg, UINT8 value)
{
	int loop;
	ghost_regs[reg] = value;

	outportb(0x388, reg);
	for (loop = 0; loop < 6; loop++) inportb(0x388);

	outportb(0x389, value);
	for (loop = 0; loop < 35; loop++) inportb(0x388);
}

noexport void Smart_Write(UINT8 reg, UINT8 value)
{
	if (ghost_regs[reg] == 0 || ghost_regs[reg] != value) {
		Write(reg, value);
	}
}

noexport void Note_Off(UINT8 i)
{
	if (i >= NUM_CHANNELS) return;

	channels[i].soundon = S_OFF;
	Smart_Write(0xb0 + i, channels[i].msb & 0xdf);
}

noexport void All_Note_Off(void)
{
	int i;
	for (i = 0; i < NUM_CHANNELS; i++) Note_Off(i);
}

noexport void Reset_Adlib(void)
{
	UINT8 i;
	/* clear all notes */
	for (i = 0; i < NUM_CHANNELS; i++) {
		Write(0xb0 + i, 0);
	}

	/* TODO: wait for note-offs */

	/* clear other regs */
	for (i = 0; i < 0xf6; i++) {
		Write(i, 0);
	}

	Write(0x01, 0x20);
	Write(0xBD, 0xC0);
}

void Start_SNG(sng *s)
{
	p.s = s;
	p.time_counter = 240;
	p.song_position = -1;
	p.patt_position = PATTERN_SIZE - 1;
	p.stride = s->channels * PATTERN_SIZE * 3;
	p.tempo = 6;
	p.counter = 5;

	channels = SzAlloc(NUM_CHANNELS, chan, "Start_SNG.channels");
	ghost_regs = SzAlloc(256, UINT8, "Start_SNG.ghost_regs");
	instrument_data = Allocate(NUM_INSTRUMENTS, 16, "Start_SNG.instrument_data");

	Convert_Instruments(s->instruments);
	All_Note_Off();

	Reset_Adlib();

	p.playing = true;
	sng_playing = true;
}

void Stop_SNG(void)
{
	int i;
	if (!p.playing) return;

	p.playing = false;
	sng_playing = false;

	All_Note_Off();

	for (i = 0; i < 17; i++)
		Write(0x40 + i, 0x3f);
	Write(0x53, 0x3f);
	Write(0x54, 0x3f);

	Free(instrument_data);
	Free(ghost_regs);
	Free(channels);
}

noexport void Play_Note(UINT8 channel, UINT8 inst, UINT16 freq, UINT8 volume)
{
	int reg, i;
	char *cdata;
	UINT8 ah, al, cl;
	chan *ch;
	if (channel >= NUM_CHANNELS) return;

	p.note_volume = volume;

	ch = &channels[channel];
	cdata = (char *)ch;
	memcpy(ch, &instrument_data[inst*16], 14);

	ah = 63 - volume;
	al = (ah + (ch->op1_volume & 0x3f)) >> 1;
	if (al > 63) al = 63;
	cl = ch->op1_volume & 0xc0;
	ch->op1_volume = cl | al;

	al = (ah + (ch->op2_volume & 0x3f)) >> 1;
	if (al > 63) al = 63;
	cl = ch->op2_volume & 0xc0;
	ch->op2_volume = cl | al;

	reg = channel * 13;
	for (i = 0; i < 11; i++) Smart_Write(adlib_regs[reg + i], cdata[i]);

	ch->freq = freq;
	ch->soundon = S_PLAYED;
}

noexport void Play_Notes_Off(void)
{
	int i;
	chan *ch = channels;

	for (i = 0; i < NUM_CHANNELS; i++, ch++) {
		if (ch->soundon == S_SOON) {
			ch->soundon = S_TRIGGER;
			Smart_Write(0xb0 + i, ch->msb & 0xdf);
		}
	}
}

noexport void Play_Notes_On(void)
{
	int i;
	chan *ch = channels;

	for (i = 0; i < NUM_CHANNELS; i++, ch++) {
		if (ch->soundon == S_TRIGGER) {
			Play_Note(i, ch->instr, ch->freq, ch->volume);
		}
	}
}

noexport void Set_Note(chan *ch)
{
	INT16 freq;

	freq = ((ch->noteslot & 0xff) - 1) << 4;
	ch->freq = freq;
	ch->basefreq = freq;
	ch->volume = 63;
	ch->soundon = S_SOON;
	ch->arpeggio = 2;
	ch->vibdir = 0;
	ch->vibcounter = 0;
	ch->vibrato = 0;
}

noexport void Play_Notes_Update(void)
{
	int i;
	chan *ch = channels;
	UINT8 bp, al;
	UINT8 octave, note, temp, patt, arg;
	UINT16 freq;
	int offset;
	char *data;
	bool portafinish;

	p.time_counter = 0;
	bp = 0;
	for (i = 0; i < NUM_CHANNELS; i++, ch++) {
		if (ch->soundon != S_OFF) {
			ch->freq += ch->addvalue;
			if (ch->freq < 0) ch->freq = 0;

			octave = ch->freq / 192;
			note = ch->freq % 192;

			octave = (octave & 0x07) << 2;
			freq = note_frequencies[note] >> 8;
			al = octave | freq;

			ch->lsb = note_frequencies[note] & 0xff;
			Smart_Write(adlib_regs[bp+11], ch->lsb);

			temp = al;
			if (ch->soundon == S_PLAYED) {
				temp |= 0x20;
			}
			ch->msb = temp;
			Smart_Write(adlib_regs[bp+11] + 16, ch->msb);

			bp += 13;
		}
	}

	if (p.playing) {
		p.counter++;
		if (p.counter >= p.tempo) {
			p.counter = 0;
			p.patt_position++;
			if (p.patt_position >= PATTERN_SIZE) {
				p.patt_position = 0;
				p.song_position++;

				patt = p.s->order[p.song_position];
				switch (patt) {
					case ORDER_HALT:
						p.playing = false;
						sng_playing = false;
						return;
					case ORDER_REPEAT:
						p.song_position = 0;
						break;
				}
			}

			/* read note data */
			patt = p.s->order[p.song_position];
			offset = patt * p.stride + p.patt_position * 3;
			data = (char *)&p.s->patterns[offset];
			for (i = 0, ch = channels; i < p.s->channels; i++, ch++) {
				ch->noteslot = data[0] & 0x7f;
				if (ch->noteslot) ch->note = ch->noteslot;

				ch->instr = data[1] >> 4;
				if (data[0] & 0x80) ch->instr |= 0x10;

				ch->effect1 = data[1] & 0x0f;
				ch->effect2 = (data[2] & 0xf0) >> 4;
				ch->effect3 = data[2] & 0x0f;

				if (ch->noteslot) {
					if (ch->effect1 == FX_PORTAMENTO) {
						arg = (ch->effect2 << 4) | ch->effect3;
						ch->tonespeed = arg;
						freq = ((ch->note & 0xff) - 1) << 4;
						ch->target = freq;
						if (ch->freq < ch->target) ch->tonedir = 1;
						else ch->tonedir = -1;
					} else if (ch->effect1 != FX_SPECIAL || ch->effect2 != SP_DELAY) {
						Set_Note(ch);
					}
				}

				data += PATTERN_SIZE * 3;
			}
		}

		/* channel effects */
		for (i = 0, ch = channels; i < p.s->channels; i++, ch++) {
			arg = (ch->effect2 << 4) | ch->effect3;
			switch (ch->effect1) {
				case FX_SPEED:
					if (arg) {
						p.tempo = arg;
					} else {
						p.playing = 0;
					}
					break;
				case FX_BREAK:
					p.patt_position = PATTERN_SIZE - 1;
					break;
				case FX_ORDER_JUMP:
					p.patt_position = PATTERN_SIZE - 1;
					p.song_position = arg - 1;
					break;
				case FX_SLIDE_UP:
					ch->freq += arg;
					ch->basefreq += arg;
					if (ch->basefreq > MAX_FREQ) {
						ch->basefreq = MAX_FREQ;
						ch->freq = MAX_FREQ;
					}
					break;
				case FX_SLIDE_DOWN:
					ch->freq -= arg;
					ch->basefreq -= arg;
					if (ch->basefreq < 0) {
						ch->basefreq = 0;
						ch->freq = 0;
					}
					break;
				case FX_PORTAMENTO:
					if (ch->tonedir) {
						if (arg) ch->tonespeed = arg;

						portafinish = false;
						if (ch->tonedir == -1) {
							ch->freq -= ch->tonespeed;
							ch->basefreq -= ch->tonespeed;
							portafinish = ch->basefreq <= ch->target;
						} else {
							ch->freq += ch->tonespeed;
							ch->basefreq += ch->tonespeed;
							portafinish = ch->basefreq >= ch->target;
						}

						if (portafinish) {
							ch->tonedir = 0;
							ch->freq = ch->target;
							ch->basefreq = ch->target;
						}
					}
					break;
				case FX_VIBRATO:
					if (ch->soundon == S_SOON || p.counter == 0) {
						if (ch->effect2 != 0) {
							ch->vibspeed = ch->effect2;
							ch->vibrato = 0;
							ch->vibcounter = ch->effect2 >> 1;
						}

						if (ch->effect3 != 0) {
							ch->vibdepth = ch->effect3;
						}
					}

					ch->vibcounter--;
					if (ch->vibcounter < 0) {
						ch->vibcounter = ch->vibspeed;
						ch->vibdir = 1;
					}

					if (ch->vibdir == 0) {
						ch->vibrato += ch->vibdepth;
					} else {
						ch->vibrato -= ch->vibdepth;
					}
					ch->freq = ch->basefreq + ch->vibrato;
					break;
				case FX_NOTE_OFF:
					if (ch->soundon != S_SOON) ch->soundon = S_RELEASE;
					break;
				case FX_SPECIAL:
					switch (ch->effect2) {
						case SP_DELAY:
						case SP_RETRIGGER:
							if (ch->effect3 == p.counter) {
								Set_Note(ch);
							}
							break;
					}
					break;
				case FX_VOLUME:
					if (arg > 63) arg = 63;
					ch->volume = arg;
					if (ch->soundon != S_SOON) ch->soundon = S_TRIGGER;
					break;
				case FX_ARPEGGIO:
					if (ch->effect2 || ch->effect3) {
						ch->arpeggio++;
						if (ch->arpeggio == 3) {
							ch->arpeggio = 0;
							ch->freq = ch->basefreq;
						} else if (ch->arpeggio == 2) {
							temp = ch->note + ch->effect3;
							if (temp > 96) temp = 96;
							freq = (temp - 1) << 4;
							ch->freq = freq;
							ch->basefreq = freq;
						} else {
							temp = ch->note + ch->effect2;
							if (temp > 96) temp = 96;
							freq = (temp - 1) << 4;
							ch->freq = freq;
							ch->basefreq = freq;
						}
					}
					break;
				case FX_VOLUME_SLIDE:
					ch->volume += ch->effect2;
					if (ch->volume > 63) ch->volume = 63;
					ch->volume -= ch->effect3;
					if (ch->volume < 0) ch->volume = 0;
					if (ch->soundon != S_SOON) ch->soundon = S_TRIGGER;
					break;
			}
		}
	}
}

void Continue_SNG(void)
{
	/* TIMER ROUTINE */
	p.time_counter++;

	switch (p.time_counter) {
		case 1:
			Play_Notes_Off();
		break;

		case 6:
			Play_Notes_On();
		break;

		case 7:
			Play_Notes_Update();
		break;
	}
}
