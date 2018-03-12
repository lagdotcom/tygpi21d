/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <dos.h>
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define DMA1_C1_ADDR	0x02
#define DMA1_C1_SIZE	0x03
#define DMA1_MASK		0x0A
#define DMA1_MODE		0x0B
#define DMA1_FLIP		0x0C
#define DMA1_C1_PAGE	0x83

#define MASK_C0			0x00
#define MASK_C1			0x01
#define MASK_C2			0x02
#define MASK_C3			0x03
#define MASK_CLEAR		0x00
#define MASK_SET		0x04

#define MODE_C0			0x00
#define MODE_C1			0x01
#define MODE_C2			0x02
#define MODE_C3			0x03
#define MODE_VERIFY		0x00
#define MODE_WRITE		0x04
#define MODE_READ		0x08
#define MODE_SINGLE_DMA	0x00
#define MODE_INIT_DMA	0x10
#define MODE_ADDR_INC	0x00
#define MODE_ADDR_DEC	0x20
#define MODE_DEMAND		0x00
#define MODE_SINGLE		0x40
#define MODE_BLOCK		0x80
#define MODE_CASCADE	0xC0

#define DSP_PLAY_SINGLE_CYCLE		0x14
#define DSP_SET_OUTPUT_FREQUENCY	0x41
#define DSP_GET_VERSION				0xE1

/* G L O B A L S ///////////////////////////////////////////////////////// */

bool sb_found;
UINT16 sb_base_addr;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct wave {
	UINT16 length;
	UINT16 frequency;
	char *sample;
} wave;

typedef struct wav_header {
	UINT32 riff;
	char ignore_1[18];
	UINT16 channels;
	UINT32 frequency;
	char ignore_2[6];
	char bit_resolution;
	char ignore_3[12];
} wav_header;

typedef struct riff_header {
	char riff[4];
	UINT32 size;
	char format[4];
} riff_header;

typedef struct wav_fmt_chunk {
	char fmt[4];			/* "fmt " */
	UINT32 size;
	UINT16 format;			/* 1 = pcm */
	UINT16 channels;		/* 1 = mono */
	UINT32 frequency;
	UINT32 byte_rate;		/* frequency * channels * width/8 */
	UINT16 block_align;		/* channels * width/8 */
	UINT16 width;
} wav_fmt_chunk;

typedef struct wav_data_chunk {
	char fmt[4];			/* "data" */
	UINT32 size;
	/* samples */
} wav_data_chunk;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

/* Code from: http://c.happycodings.com/small-programs/code37.html */

/* Checks to see if a Sound Blaster exists at a given address. */
noexport bool Check_For_SB(UINT16 base)
{
	/* Reset */
	outportb(base + 0x6, 1);
	delay(10);
	outportb(base + 0x6, 0);
	delay(10);

	/* Check for known values */
	if (inportb(base + 0xE) & 0x80 && inportb(base + 0xA) == 0xAA) {
		sb_found = true;
		sb_base_addr = base;
		return true;
	}

	return false;
}

/* Send a byte to the DSP(Digital Signal Processor) on the Sound Blaster */
noexport void DSP_Write(UINT8 value)
{
	/* Wait for DSP to be ready */
	while (inportb(sb_base_addr + 0xC) & 0x80);

	/* Send byte */
	outportb(sb_base_addr + 0xC, value);
}

noexport UINT8 DSP_Read(void)
{
	/* Wait for DSP to be ready */
	while (!(inportb(sb_base_addr + 0xE) & 0x80));

	/* Read byte */
	return inportb(sb_base_addr + 0xA);
}

/* Play back some wave data */
noexport void Play_WAV(wave *data)
{
	UINT32 linear_address;
	UINT16 page, offset, length;

	DSP_Write(DSP_SET_OUTPUT_FREQUENCY);
	DSP_Write(data->frequency >> 8);
	DSP_Write(data->frequency & 0xFF);

	/* convert pointer to linear address */
	linear_address = FP_SEG(data->sample);
	linear_address = (linear_address << 4) + FP_OFF(data->sample);
	page = linear_address >> 16;
	offset = linear_address & 0xFFFF;

	outportb(DMA1_MASK, MASK_C1 | MASK_SET);
	outportb(DMA1_FLIP, 0);				/* Clear byte pointer */
	outportb(DMA1_MODE, MODE_SINGLE_DMA | MODE_C1 | MODE_READ | MODE_SINGLE);

	/* Write address offset */
	outportb(DMA1_C1_ADDR, offset & 0xFF);
	outportb(DMA1_C1_ADDR, offset >> 8);

	/* Write address page and length */
	outportb(DMA1_C1_PAGE, page);

	length = data->length - 1;	/* yes, minus one */
	outportb(DMA1_C1_SIZE, length & 0xFF);	/* yes, minus one*/
	outportb(DMA1_C1_SIZE, length >> 8);

	outportb(DMA1_MASK, MASK_C1 | MASK_CLEAR);

	DSP_Write(DSP_PLAY_SINGLE_CYCLE);
	DSP_Write(data->length & 0xFF);
	DSP_Write(data->length >> 8);
}

/* Load a WAV file into memory. */
noexport bool Load_WAV(wave *data, char *filename)
{
	char *err;
	FILE *f;
	riff_header header;
	wav_fmt_chunk wavefmt;
	wav_data_chunk wavedata;

	f = fopen(filename, "rb");
	if (!f)
	{
		die("Load_WAV: couldn't open file");
		return false;
	}

	fread(&header, sizeof(riff_header), 1, f);
	if (strncmp(header.riff, "RIFF", 4) != 0) {
		err = "Load_WAV: not a RIFF file";
		goto _dead;
	}
	if (strncmp(header.format, "WAVE", 4) != 0) {
		err = "Load_WAV: not a WAVE format file";
		goto _dead;
	}

	fread(&wavefmt, sizeof(wav_fmt_chunk), 1, f);
	if (strncmp(wavefmt.fmt, "fmt ", 4) != 0) {
		err = "Load_WAV: fmt chunk missing";
		goto _dead;
	}
	if (wavefmt.format != 1) {
		err = "Load_WAV: not a PCM file";
		goto _dead;
	}
	if (wavefmt.channels != 1) {
		err = "Load_WAV: not a mono file";
		goto _dead;
	}
	if (wavefmt.width != 8) {
		err = "Load_WAV: not an 8 bit file";
		goto _dead;
	}

	fread(&wavedata, sizeof(wav_data_chunk), 1, f);
	if (strncmp(wavedata.fmt, "data", 4) != 0) {
		err = "Load_WAV: data chunk missing";
		goto _dead;
	}

	data->frequency = wavefmt.frequency & 0xFFFF;
	data->length = wavedata.size & 0xFFFF;
	data->sample = SzAlloc(data->length, char *, "Load_WAV");
	if (data->sample == null)
	{
		err = "Load_WAV: out of memory";
		goto _dead;
	}
	fread(data->sample, data->length, 1, f);

	fclose(f);
	return true;

_dead:
	fclose(f);
	die(err);
	return false;
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Initialise_Sound(void)
{
	UINT8 major, minor;
	sb_found = false;

	Check_For_SB(0x220);
	if (!sb_found) Check_For_SB(0x240);

	if (sb_found) {
		DSP_Write(DSP_GET_VERSION);
		major = DSP_Read();
		minor = DSP_Read();

		printf("Initialise_Sound: SoundBlaster found @%x: v%u.%u\n", sb_base_addr, major, minor);
	}
}

void Free_Sound(void)
{
	/* No teardown yet. */
	Log("Free_Sound: %p", null);
}

void Play_Sound(char *name)
{
	wave data;
	char filename[20];

	if (!sb_found) return;

	sprintf(filename, "SOUND\\%s.WAV", name);
	if (Load_WAV(&data, filename)) {
		Play_WAV(&data);

		/* TODO: this is wrong! card is still reading sample data at this point, should use interrupts instead */
		Free(data.sample);
	}
}
