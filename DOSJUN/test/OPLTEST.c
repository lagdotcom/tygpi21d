/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <dos.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef unsigned int WORD;
typedef unsigned char BYTE;

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
#define BASEDRUM		0x10
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

/* G L O B A L S ///////////////////////////////////////////////////////// */

WORD detected = 0;
WORD FMport = 0;
WORD OPL3 = 0;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

/* Thanks, http://www.fit.vutbr.cz/~arnost/opl/opl3.html */

/*
 * Direct write to any Adlib/SB Pro II FM synthetiser register.
 *   reg - register number (range 0x001-0x0F5 and 0x101-0x1F5). When high byte
 *         of reg is zero, data go to port FMport, otherwise to FMport+2
 *   data - register value to be written
 */
BYTE FMwriteReg(WORD reg, BYTE data)
{
	asm mov  dx,FMport;
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

BYTE FMreadReg(WORD reg)
{
	return inportb(FMport + reg);
}

/*
 * Write to an operator pair. To be used for register bases of 0x20, 0x40,
 * 0x60, 0x80 and 0xE0.
 */
void FMwriteChannel(BYTE regbase, BYTE channel, BYTE data1, BYTE data2)
{
	static BYTE adlib_op[] = {0, 1, 2, 8, 9, 10, 16, 17, 18};
	static BYTE sbpro_op[] = {0, 1, 2, 6, 7, 8, 12, 13, 14, 18, 19, 20, 24, 25, 26, 30, 31, 32};
	static WORD rg[] = {0x000, 0x001, 0x002, 0x003, 0x004, 0x005,
		0x008, 0x009, 0x00A, 0x00B, 0x00C, 0x00D,
		0x010, 0x011, 0x012, 0x013, 0x014, 0x015,
		0x100, 0x101, 0x102, 0x103, 0x104, 0x105,
		0x108, 0x109, 0x10A, 0x10B, 0x10C, 0x10D,
		0x110, 0x111, 0x112, 0x113, 0x114, 0x115};
	register WORD reg;

	if (OPL3) {
		reg = sbpro_op[channel];
		FMwriteReg(regbase + rg[reg], data1);
		FMwriteReg(regbase + rg[reg + 3], data2);
	} else {
		reg = regbase + adlib_op[channel];
		FMwriteReg(reg, data1);
		FMwriteReg(reg + 3, data2);
	}
}

/*
 * Write to channel a single value. To be used for register bases of
 * 0xA0, 0xB0 and 0xC0.
 */
void FMwriteValue(BYTE regbase, BYTE channel, BYTE value)
{
	static WORD ch[] = {0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 0x008,
		0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108};
	register WORD chan;

	if (OPL3) {
		chan = ch[channel];
	} else {
		chan = channel;
	}
	FMwriteReg(regbase + chan, value);
}

/* M A I N /////////////////////////////////////////////////////////////// */

int DetectOPL2()
{
	BYTE status, newstatus;
	int delay;

	/* Reset Timer 1/2 */
	FMwriteReg(REG_IRQ, IRQ_MASK1|IRQ_MASK2);

	/* Reset IRQ */
	FMwriteReg(REG_IRQ, IRQ_RESET);

	/* Read status */
	status = FMreadReg(REG_STATUS);

	/* Set timer 1 max */
	FMwriteReg(REG_TIMER1, 0xFF);

	/* Unmask/start Timer 1 */
	FMwriteReg(REG_IRQ, IRQ_START1|IRQ_MASK2);

	for (delay = 0; delay < 9999; delay++)
		; /* wait for at least 80us */

	/* Read status again */
	newstatus = FMreadReg(REG_STATUS);

	/* Reset Timer 1/2 */
	FMwriteReg(REG_IRQ, IRQ_MASK1|IRQ_MASK2);

	/* Reset IRQ */
	FMwriteReg(REG_IRQ, IRQ_RESET);

	return status == 0 && newstatus == (STAT_IRQ|STAT_TIMER1);
}

int DetectOPL3()
{
	BYTE status = FMreadReg(0);

	return (status & STAT_OPL3) == 0;
}

void DetectAtPort(WORD port)
{
	FMport = port;
	if (DetectOPL2()) {
		detected = 1;
		printf("OPL2 detected at port 0x%x.\n", port);
		if (DetectOPL3()) {
			OPL3 = 1;
			printf("OPL3 detected at port 0x%x.\n", port);
		}
	}
}

void main(void)
{
	detected = 0;
	if (!detected) DetectAtPort(0x220);
	if (!detected) DetectAtPort(0x388);

	if (!detected) {
		printf("Could not find OPL2/3 card. Aborting.\n");
		return;
	}

	/* Try to play a note... */
	FMwriteChannel(REG_SUSTAINED, 0, SUSTAIN_ON|MULTI_1, 0);
	FMwriteChannel(REG_KEYSCALE, 0, 0, 0);
	FMwriteChannel(REG_ATTACK, 0, ATTACK(15), 0);
	FMwriteChannel(REG_SUSTAIN, 0, SUSTAIN(15), 0);
	FMwriteValue(REG_FREQ, 0, 0xF0);
	FMwriteValue(REG_KEYON, 0, KEYON|BLOCK(4));
	FMwriteValue(REG_SYNTHESIS, 0, SYNTH_ADD|OUT_L|OUT_R);
}
