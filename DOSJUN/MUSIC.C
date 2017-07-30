/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include "common.h"
#include "types.h"

#define DEBUG 1

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SOP_SIGNATURE	"sopepos"

#define CHAN_UNUSED		0
#define CHAN_4OP		1
#define CHAN_2OP		2

/* two sets of data */
#define INST_4OP		0

/* one set of data */
#define INST_2OP		1
#define INST_BASSDRUM	6
#define INST_SNAREDRUM	7
#define INST_TOMTOM		8
#define INST_CYMBAL		9
#define INST_HIHAT		10

/* no data */
#define INST_UNUSED		12

/* one argument */
#define EVT_SPECIAL		1
#define EVT_VOLUME		4
#define EVT_PITCH		5
#define EVT_INSTRUMENT	6
#define EVT_PANNING		7

/* one argument - control track only */
#define EVT_TEMPO		3
#define EVT_GVOLUME		8

/* two arguments */
#define EVT_NOTE		2

#define PITCH_OFFSET	100

#define PAN_RIGHT		0
#define PAN_CENTRE		1
#define PAN_LEFT		2

#define REG_STATUS		0
#define STAT_OPL3		0x06
#define STAT_TIMER2		0x20
#define STAT_TIMER1		0x40
#define STAT_IRQ		0x80

#define REG_WSE			1
#define REG_TEST		1
#define WSE_ENABLE		0x20

#define REG_TIMER1		2
#define REG_TIMER2		3

#define REG_IRQ			4
#define IRQ_START1		0x01
#define IRQ_START2		0x02
#define IRQ_MASK2		0x20
#define IRQ_MASK1		0x40
#define IRQ_RESET		0x80

#define REG_FOUROP		0x104
#define FOUR_0_3		0x01
#define FOUR_1_4		0x02
#define FOUR_2_5		0x04
#define FOUR_9_12		0x08
#define FOUR_10_13		0x10
#define FOUR_11_14		0x20

#define	REG_OPL3		0x105
#define OPL3_OFF		0
#define OPL3_ON			1

#define REG_CSW			8
#define REG_NOTESEL		8
#define NOTESEL_ON		0x40
#define CSW_ON			0x80

#define REG_CHARACTER	0x20
#define REG_TREMOLO		0x20
#define REG_VIBRATO		0x20
#define REG_SUSTAINED	0x20
#define REG_KSR			0x20
#define REG_MULTI		0x20
#define MULTI_1			1
#define KSR_ON			0x10
#define SUSTAIN_ON		0x20
#define VIBRATO_ON		0x40
#define TREMOLO_ON		0x80

#define REG_KEYSCALE	0x40
#define REG_OUTPUT		0x40
#define KEYSCALE_0		0
#define KEYSCALE_1_5	0x40
#define KEYSCALE_3		0x80
#define KEYSCALE_6		0xC0

#define REG_ATTACK		0x60
#define REG_DECAY		0x60
#define ATTACK(n)		(n << 4)

#define REG_SUSTAIN		0x80
#define REG_RELEASE		0x80
#define SUSTAIN(n)		(n << 4)

#define REG_FREQ		0xA0

#define REG_KEYON		0xB0
#define REG_BLOCK		0xB0
#define REG_FREQ_HI		0xB0
#define BLOCK(n)		(n << 2)
#define KEYON			0x20

#define REG_TREMOLO_D	0xBD
#define REG_VIBRATO_D	0xBD
#define REG_PERC		0xBD
#define HIHAT			0x01
#define CYMBAL			0x02
#define TOMTOM			0x04
#define SNAREDRUM		0x08
#define BASSDRUM		0x10
#define PERCUSSION_ON	0x20
#define VIBRATO_7		0
#define VIBRATO_14		0x40
#define TREMOLO_1		0
#define TREMOLO_4_8		0x80

#define REG_SPEAKERS	0xC0
#define REG_MODULATION	0xC0
#define REG_SYNTHESIS	0xC0
#define SYNTH_MOD		0
#define SYNTH_ADD		1
#define FEEDBACK(n)		(n << 1)
#define OUT_L			0x10
#define OUT_R			0x20

#define REG_WAVEFORM	0xE0
#define WAVE_SINE		0
#define WAVE_HALFSINE	1
#define WAVE_ABSSINE	2
#define WAVE_PULSESINE	3
#define WAVE_SINE_E		4
#define WAVE_ABSSINE_E	5
#define WAVE_SQUARE		6
#define WAVE_SQUARE_DV	7

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	char signature[7];
	UINT8 majorVersion;
	UINT8 minorVersion;
	BYTE _pad0;
	char fileName[13];
	char title[31];
	UINT8 percussive;	/* 1 = percussive */
	BYTE _pad1;
	UINT8 tickBeat;		/* ticks per beat */
	BYTE _pad2;
	UINT8 beatMeasure;	/* beats per measure */
	UINT8 basicTempo;	/* beats per minute */
	char comment[13];
	UINT8 nTracks;		/* number of tracks */
	UINT8 nInsts;		/* number of instruments */
	BYTE _pad3;
} sop_header;

typedef struct {
	UINT8 mod_char;
	UINT8 mod_scale;
	UINT8 mod_attack;
	UINT8 mod_sustain;
	UINT8 mod_wave;
	UINT8 feedback;
	UINT8 car_char;
	UINT8 car_scale;
	UINT8 car_attack;
	UINT8 car_sustain;
	UINT8 car_wave;
} sop_instrument_data;

typedef struct {
	UINT8 instType;		/* see INST_* */
	char shortName[8];
	char longName[19];
	sop_instrument_data *data;
} sop_instrument;
#define INST_SZ		28

typedef struct {
	UINT16 ticks;
	UINT8 event;		/* see EVT_* */
	UINT8 arg0;
	UINT16 arg1;
} sop_event;
#define EVT_SZ		4

typedef struct {
	UINT16 numEvents;
	UINT32 dataSize;
	BYTE *data;
} sop_track;
#define TRACK_SZ	6

typedef struct {
	UINT16 on;
	UINT16 wait;
	UINT16 off;
	UINT8 instrument;
	UINT8 volume;
	UINT8 pitch;
	UINT8 panning;
	bool updated;
} channel;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport bool detected = false;
noexport UINT16 opl_port;
noexport bool opl3_supported;
noexport bool opl3_enabled = false;

noexport bool loaded = false;
noexport bool playing = false;
noexport UINT32 clicks, clicks_max;
noexport sop_header header;
noexport BYTE *channelTypes;
noexport sop_instrument *instruments;
noexport sop_track *tracks;
noexport channel channels[20];
noexport UINT8 global_volume;
noexport bool update_all_channels;
noexport UINT8 global_tempo;
noexport UINT8 percussion_reg;

#include "notefreq.c"

/* O P L 2/3  C O D E //////////////////////////////////////////////////// */
/* Thanks, http://www.fit.vutbr.cz/~arnost/opl/opl3.html */

/*
 * Direct write to any Adlib/SB Pro II FM synthetiser register.
 *   reg - register number (range 0x001-0x0F5 and 0x101-0x1F5). When high byte
 *         of reg is zero, data go to port opl_port, otherwise to opl_port+2
 *   data - register value to be written
 */
noexport BYTE OPL_Write_Register(UINT16 reg, BYTE data)
{
	asm mov  dx,opl_port;
	asm mov  ax,reg;
	asm or   ah,ah;		/* high byte is nonzero -- write to port base+2 */
	asm jz   out1;
	asm inc  dx;
	asm inc  dx;

out1:
	asm out  dx,al;

	asm mov  cx,6;
loop1:					/* delay between writes */
	asm in   al,dx;
	asm loop loop1;

	asm inc  dx;
	asm mov  al,data;
	asm out  dx,al;
	asm dec  dx;

	asm mov  cx,36;
loop2:					/* delay after data write */
	asm in   al,dx;
	asm loop loop2;

	return _AL;
}

noexport BYTE OPL_Read_Register(UINT16 reg)
{
	return inportb(opl_port + reg);
}

/*
 * Write to an operator pair. To be used for register bases of 0x20, 0x40,
 * 0x60, 0x80 and 0xE0.
 */
noexport void OPL_Write_Channel(BYTE regbase, BYTE channel, BYTE data1, BYTE data2)
{
	static BYTE adlib_op[] = {0, 1, 2, 8, 9, 10, 16, 17, 18};
	static BYTE sbpro_op[] = {0, 1, 2, 6, 7, 8, 12, 13, 14, 18, 19, 20, 24, 25, 26, 30, 31, 32};
	static UINT16 rg[] = {0x000, 0x001, 0x002, 0x003, 0x004, 0x005,
		0x008, 0x009, 0x00A, 0x00B, 0x00C, 0x00D,
		0x010, 0x011, 0x012, 0x013, 0x014, 0x015,
		0x100, 0x101, 0x102, 0x103, 0x104, 0x105,
		0x108, 0x109, 0x10A, 0x10B, 0x10C, 0x10D,
		0x110, 0x111, 0x112, 0x113, 0x114, 0x115};
	register UINT16 reg;

	if (opl3_enabled) {
		reg = sbpro_op[channel];
		OPL_Write_Register(regbase + rg[reg], data1);
		OPL_Write_Register(regbase + rg[reg + 3], data2);
	} else {
		reg = regbase + adlib_op[channel];
		OPL_Write_Register(reg, data1);
		OPL_Write_Register(reg + 3, data2);
	}
}

/*
 * Write to channel a single value. To be used for register bases of
 * 0xA0, 0xB0 and 0xC0.
 */
noexport void OPL_Write_Value(BYTE regbase, BYTE channel, BYTE value)
{
	static UINT16 ch[] = {0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 0x008,
		0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108};
	register UINT16 chan;

	if (opl3_enabled) {
		chan = ch[channel];
	} else {
		chan = channel;
	}
	OPL_Write_Register(regbase + chan, value);
}

noexport bool Detect_OPL2()
{
	BYTE status, newstatus;
	int delay;

	/* Reset Timer 1/2 */
	OPL_Write_Register(REG_IRQ, IRQ_MASK1|IRQ_MASK2);

	/* Reset IRQ */
	OPL_Write_Register(REG_IRQ, IRQ_RESET);

	/* Read status */
	status = OPL_Read_Register(REG_STATUS);

	/* Set timer 1 max */
	OPL_Write_Register(REG_TIMER1, 0xFF);

	/* Unmask/start Timer 1 */
	OPL_Write_Register(REG_IRQ, IRQ_START1|IRQ_MASK2);

	for (delay = 0; delay < 9999; delay++)
		; /* wait for at least 80us */

	/* Read status again */
	newstatus = OPL_Read_Register(REG_STATUS);

	/* Reset Timer 1/2 */
	OPL_Write_Register(REG_IRQ, IRQ_MASK1|IRQ_MASK2);

	/* Reset IRQ */
	OPL_Write_Register(REG_IRQ, IRQ_RESET);

	return status == 0 && newstatus == (STAT_IRQ|STAT_TIMER1);
}

noexport bool Detect_OPL3()
{
	BYTE status = OPL_Read_Register(REG_STATUS);

	return (status & STAT_OPL3) == 0;
}

noexport void Detect_OPL_At(UINT16 port)
{
	opl_port = port;
	if (Detect_OPL2()) {
		detected = true;
		printf("OPL2 detected at port 0x%x.\n", port);
		if (Detect_OPL3()) {
			opl3_supported = true;
			printf("OPL3 detected at port 0x%x.\n", port);
		} else {
			opl3_supported = false;
		}
	}
}

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport int Instrument_Data_Count(UINT8 type)
{
	switch (type) {
		case INST_UNUSED: return 0;
		case INST_4OP: return 2;

		case INST_2OP:
		case INST_BASSDRUM:
		case INST_SNAREDRUM:
		case INST_TOMTOM:
		case INST_CYMBAL:
		case INST_HIHAT:
			return 1;

		default:
			printf("Warning: Instrument_Data_Count(%d) is invalid\n", type);
			abort();
			return 0;
	}
}

noexport bool Has_Second_Argument(UINT8 event)
{
	switch (event) {
		case EVT_NOTE: return true;

		case EVT_SPECIAL:
		case EVT_VOLUME:
		case EVT_PITCH:
		case EVT_INSTRUMENT:
		case EVT_PANNING:
		case EVT_TEMPO:
		case EVT_GVOLUME:
			return false;

		default:
			printf("Warning: Has_Second_Argument(%d) is invalid\n", event);
			abort();
			return false;
	}
}

noexport bool Load_SOP(char *filename)
{
	int i, j;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Could not open for reading: %s\n", filename);
		return false;
	}

	fread(&header, sizeof(sop_header), 1, fp);
	if (strncmp(header.signature, SOP_SIGNATURE, 7) != 0) {
		printf("Not a SOPEPOS: %s\n", filename);
		fclose(fp);
		return false;
	}

	channelTypes = SzAlloc(header.nTracks, BYTE, "Load_SOP.channelTypes");
	fread(channelTypes, sizeof(BYTE), header.nTracks, fp);

	instruments = SzAlloc(header.nInsts, sop_instrument, "Load_SOP.instruments");
	for (i = 0; i < header.nInsts; i++) {
		fread(&instruments[i], 1, INST_SZ, fp);
		j = Instrument_Data_Count(instruments[i].instType);
		if (j > 0) {
			instruments[i].data = SzAlloc(j, sop_instrument_data, "Load_SOP.instruments[i]");
			fread(instruments[i].data, sizeof(sop_instrument_data), j, fp);
		} else {
			instruments[i].data = null;
		}
	}

	/* +1 for the Control track */
	tracks = SzAlloc(header.nTracks + 1, sop_track, "Load_SOP.tracks");
	for (i = 0; i <= header.nTracks; i++) {
		fread(&tracks[i], 1, TRACK_SZ, fp);
		tracks[i].data = SzAlloc(tracks[i].dataSize, 1, "Load_SOP.tracks[i]");
		fread(tracks[i].data, tracks[i].dataSize, 1, fp);
	}

	fclose(fp);
	loaded = true;
	return true;
}

noexport void Free_SOP()
{
	int i;

	Free(channelTypes);

	for (i = 0; i < header.nInsts; i++) Free(instruments[i].data);
	Free(instruments);

	/* +1 for the Control track */
	for (i = 0; i <= header.nTracks; i++) Free(tracks[i].data);
	Free(tracks);

	loaded = false;
}

noexport UINT16 Frequency_From_Note(UINT8 note)
{
	return note_frequencies[note - 12];
} 

noexport UINT32 Bpm_To_Clicks(UINT8 bpm)
{
	/*
	timer1 fires every 80us
	 - we set it up to fire 10 times for one click

	clicks per beat = 60 * 1000000 / 80 / bpm / ticksPerBeat / 10
	*/

	return 75000 / bpm / header.tickBeat;
}

noexport void Do_Event(sop_event *e, int track_num)
{
	UINT16 freq;
	channel *c = &channels[track_num];
	sop_instrument *i;

	switch (e->event) {
		case EVT_VOLUME:
			#if DEBUG
			printf("%dv%d ", track_num, e->arg0);
			#endif
			c->volume = e->arg0;
			c->updated = true;
			break;

		case EVT_PITCH:
			#if DEBUG
			printf("%dp%d ", track_num, e->arg0);
			#endif
			c->pitch = e->arg0;
			c->updated = true;
			break;

		case EVT_INSTRUMENT:
			i = &instruments[e->arg0];
			OPL_Write_Channel(REG_CHARACTER, track_num, i->data[0].mod_char,    i->data[0].car_char);
			OPL_Write_Channel(REG_KEYSCALE,  track_num, i->data[0].mod_scale,   i->data[0].car_scale);
			OPL_Write_Channel(REG_ATTACK,    track_num, i->data[0].mod_attack,  i->data[0].car_attack);
			OPL_Write_Channel(REG_SUSTAIN,   track_num, i->data[0].mod_sustain, i->data[0].car_sustain);
			OPL_Write_Channel(REG_WAVEFORM,  track_num, i->data[0].mod_wave,    i->data[0].car_wave);
			OPL_Write_Value(REG_MODULATION, track_num, i->data[0].feedback);

			if (i->instType == INST_4OP) {
				OPL_Write_Channel(REG_CHARACTER+8, track_num, i->data[1].mod_char,    i->data[1].car_char);
				OPL_Write_Channel(REG_KEYSCALE+8,  track_num, i->data[1].mod_scale,   i->data[1].car_scale);
				OPL_Write_Channel(REG_ATTACK+8,    track_num, i->data[1].mod_attack,  i->data[1].car_attack);
				OPL_Write_Channel(REG_SUSTAIN+8,   track_num, i->data[1].mod_sustain, i->data[1].car_sustain);
				OPL_Write_Channel(REG_WAVEFORM+8,  track_num, i->data[1].mod_wave,    i->data[1].car_wave);
				OPL_Write_Value(REG_MODULATION+8, track_num, i->data[1].feedback);
			}
			#if DEBUG
			printf("%di%d ", track_num, e->arg0);
			#endif
			c->instrument = e->arg0;
			c->updated = true;
			break;

		case EVT_PANNING:
			#if DEBUG
			printf("%da%d ", track_num, e->arg0);
			#endif
			c->panning = e->arg0;
			c->updated = true;
			break;

		case EVT_TEMPO:
			#if DEBUG
			printf("!t%d ", e->arg0);
			#endif
			global_tempo = e->arg0;
			clicks_max = Bpm_To_Clicks(global_tempo);
			break;

		case EVT_GVOLUME:
			#if DEBUG
			printf("!v%d ", e->arg0);
			#endif
			global_volume = e->arg0;
			update_all_channels = true;
			break;

		case EVT_NOTE:
			i = &instruments[c->instrument];

			#if DEBUG
			printf("%dn%u,%u ", track_num, e->arg0, e->arg1);
			#endif

			switch (i->instType) {
				case INST_2OP:
				case INST_4OP:
					freq = Frequency_From_Note(e->arg0);
					OPL_Write_Value(REG_FREQ,  track_num, freq & 0xFF);
					OPL_Write_Value(REG_KEYON, track_num, KEYON | (freq >> 8));
					break;

				/* TODO: preserve depth/other perc values */
				case INST_BASSDRUM:
					percussion_reg |= BASSDRUM;
					break;
				case INST_SNAREDRUM:
					percussion_reg |= SNAREDRUM;
					break;
				case INST_TOMTOM:
					percussion_reg |= TOMTOM;
					break;
				case INST_CYMBAL:
					percussion_reg |= CYMBAL;
					break;
				case INST_HIHAT:
					percussion_reg |= HIHAT;
					break;
			}

			c->off = e->arg1;
			break;

		case EVT_SPECIAL:
			/* TODO: Notify_Engine(e->arg0) */
			printf("!s%d\n", e->arg0);
			break;
	}
}

noexport void Do_Next_Tick()
{
	int i;
	channel *c;
	sop_track *t;
	sop_instrument *in;
	sop_event *e;

	/* TODO: SOMETHING IS UNWELL */
	for (i = 0; i <= header.nTracks; i++) {
		c = &channels[i];
		t = &tracks[i];
		in = &instruments[c->instrument];
		e = &t->data[c->on];

		if (c->off == 1) {
			c->off = 0;
			switch (in->instType) {
				case INST_2OP:
				case INST_4OP:
					OPL_Write_Value(REG_KEYON, i, 0);
					break;

				case INST_BASSDRUM:
					percussion_reg &= ~BASSDRUM;
					break;
				case INST_SNAREDRUM:
					percussion_reg &= ~SNAREDRUM;
					break;
				case INST_TOMTOM:
					percussion_reg &= ~TOMTOM;
					break;
				case INST_CYMBAL:
					percussion_reg &= ~CYMBAL;
					break;
				case INST_HIHAT:
					percussion_reg &= ~HIHAT;
					break;
			}
			#if DEBUG
			printf("%d. ", i);
			#endif
		} else if (c->off > 1) {
			c->off--;
		}

		if (c->on >= t->numEvents) continue;
		c->wait++;

		while (e->ticks <= c->wait) {
			c->wait = 0;
			Do_Event(e, i);
			channels[i].on += Has_Second_Argument(e->event) ? (EVT_SZ + 2) : (EVT_SZ);
		}
	}

	OPL_Write_Register(REG_PERC, percussion_reg);
}

noexport void Setup_OPL_Channels()
{
	UINT8 four = 0;

	if (!opl3_supported) return;

	if (channelTypes[0] == CHAN_4OP)  four |= FOUR_0_3;
	if (channelTypes[1] == CHAN_4OP)  four |= FOUR_1_4;
	if (channelTypes[2] == CHAN_4OP)  four |= FOUR_2_5;
	if (channelTypes[9] == CHAN_4OP)  four |= FOUR_9_12;
	if (channelTypes[10] == CHAN_4OP) four |= FOUR_10_13;
	if (channelTypes[11] == CHAN_4OP) four |= FOUR_11_14;

	if (four) {
		opl3_enabled = true;
		OPL_Write_Register(REG_FOUROP, four);
	} else {
		opl3_enabled = false;
		OPL_Write_Register(REG_FOUROP, 0);
	}
}

noexport void Start_Channels()
{
	int i;

	for (i = 0; i < header.nTracks; i++) {
		channels[i].on = 0;
		channels[i].wait = 0;
		channels[i].off = 0;
		channels[i].instrument = 0;
		channels[i].volume = 0x7F;
		channels[i].pitch = 100;
		channels[i].panning = PAN_CENTRE;
		channels[i].updated = true;
	}
}

noexport void Update_Channels()
{
	int i;
	UINT8 temp;

	for (i = 0; i < header.nTracks; i++) {
		if (channels[i].updated || update_all_channels) {
			channels[i].updated = false;

			/* update modulation value */
			switch (channels[i].panning) {
				case PAN_LEFT:
					temp = OUT_L;
					break;

				case PAN_RIGHT:
					temp = OUT_R;
					break;

				case PAN_CENTRE:
					temp = OUT_L | OUT_R;
					break;
			}
			temp |= instruments[channels[i].instrument].data[0].feedback;
			OPL_Write_Value(REG_MODULATION, i, temp);

			/* update KSL/output value */
			temp = (channels[i].volume + global_volume) >> 2;
			OPL_Write_Value(REG_OUTPUT, i, temp);
		}
	}

	update_all_channels = false;
}

noexport void Silence_Channels()
{
	/* TODO: this isn't quite working */
	int i;

	for (i = 0; i < 18; i++) {
		OPL_Write_Channel(REG_KEYON, i, 0, 0);
	}
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Initialise_Music()
{
	detected = false;
	if (!detected) Detect_OPL_At(0x220);
	if (!detected) Detect_OPL_At(0x388);

	if (!detected) {
		printf("Could not find OPL2/3 card. Aborting.\n");
		return;
	}

	loaded = false;
	channelTypes = null;
	instruments = null;
	tracks = null;

	/* TODO */
}

bool Play_Music(char *filename)
{
	if (!detected) return false;
	if (loaded) Free_SOP();

	if (!Load_SOP(filename)) {
		return false;
	}

	opl3_enabled = false;
	Setup_OPL_Channels();
	Start_Channels();

	if (opl3_enabled) {
		OPL_Write_Register(REG_OPL3, OPL3_ON);
		OPL_Write_Register(REG_WSE, WSE_ENABLE);
	} else {
		OPL_Write_Register(REG_OPL3, OPL3_OFF);
		OPL_Write_Register(REG_WSE, 0);
	}

	/* start our timer */
	OPL_Write_Register(REG_TIMER1, 0xF6);
	OPL_Write_Register(REG_IRQ, IRQ_START1|IRQ_MASK2);

	global_volume = 0x7F;
	global_tempo = header.basicTempo;
	percussion_reg = header.percussive ? PERCUSSION_ON : 0;

	clicks = 0;
	clicks_max = Bpm_To_Clicks(global_tempo);
	playing = true;

	printf("Playing %s at %dbpm (%dcl).\n", filename, global_tempo, clicks_max);

	return true;
}

void Stop_Music()
{
	if (!detected) return;
	playing = false;

	Silence_Channels();
	/* TODO: clear channelTypes? */
}

void Free_Music()
{
	if (!detected) return;
	if (playing) Stop_Music();
	if (loaded) Free_SOP();
}

void Process_Music()
{
	UINT8 status;

	if (!detected) return;
	if (!playing) return;

	status = OPL_Read_Register(REG_STATUS);
	if (status & STAT_TIMER1) {
		OPL_Write_Register(REG_TIMER1, 0xF6);
		OPL_Write_Register(REG_IRQ, IRQ_START1|IRQ_MASK2);

		clicks++;
		if (clicks > clicks_max) {
			#if DEBUG
			printf("\nC:");
			#endif
			clicks = 0;
			Do_Next_Tick();
			Update_Channels();
		}
	}
}
