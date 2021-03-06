#include "types.h"

#define _BLOCK(n) ((n) << 10)

UINT16 note_frequencies[] = {
	/*  C0 */ _BLOCK(0) | 344,
	/* C#0 */ _BLOCK(0) | 365,
	/*  D0 */ _BLOCK(0) | 387,
	/* Eb0 */ _BLOCK(0) | 410,
	/*  E0 */ _BLOCK(0) | 434,
	/*  F0 */ _BLOCK(0) | 460,
	/* F#0 */ _BLOCK(0) | 487,
	/*  G0 */ _BLOCK(0) | 516,
	/* Ab0 */ _BLOCK(0) | 547,
	/*  A0 */ _BLOCK(0) | 580,
	/* Bb0 */ _BLOCK(0) | 614,
	/*  B0 */ _BLOCK(0) | 651,
	/*  C1 */ _BLOCK(0) | 689,
	/* C#1 */ _BLOCK(0) | 730,
	/*  D1 */ _BLOCK(0) | 774,
	/* Eb1 */ _BLOCK(0) | 820,
	/*  E1 */ _BLOCK(0) | 869,
	/*  F1 */ _BLOCK(0) | 920,
	/* F#1 */ _BLOCK(0) | 975,
	/*  G1 */ _BLOCK(1) | 516,
	/* Ab1 */ _BLOCK(1) | 547,
	/*  A1 */ _BLOCK(1) | 580,
	/* Bb1 */ _BLOCK(1) | 614,
	/*  B1 */ _BLOCK(1) | 651,
	/*  C2 */ _BLOCK(1) | 689,
	/* C#2 */ _BLOCK(1) | 730,
	/*  D2 */ _BLOCK(1) | 774,
	/* Eb2 */ _BLOCK(1) | 820,
	/*  E2 */ _BLOCK(1) | 869,
	/*  F2 */ _BLOCK(1) | 920,
	/* F#2 */ _BLOCK(1) | 975,
	/*  G2 */ _BLOCK(2) | 516,
	/* Ab2 */ _BLOCK(2) | 547,
	/*  A2 */ _BLOCK(2) | 580,
	/* Bb2 */ _BLOCK(2) | 614,
	/*  B2 */ _BLOCK(2) | 651,
	/*  C3 */ _BLOCK(2) | 689,
	/* C#3 */ _BLOCK(2) | 730,
	/*  D3 */ _BLOCK(2) | 774,
	/* Eb3 */ _BLOCK(2) | 820,
	/*  E3 */ _BLOCK(2) | 869,
	/*  F3 */ _BLOCK(2) | 920,
	/* F#3 */ _BLOCK(2) | 975,
	/*  G3 */ _BLOCK(3) | 516,
	/* Ab3 */ _BLOCK(3) | 547,
	/*  A3 */ _BLOCK(3) | 580,
	/* Bb3 */ _BLOCK(3) | 614,
	/*  B3 */ _BLOCK(3) | 651,
	/*  C4 */ _BLOCK(3) | 689,
	/* C#4 */ _BLOCK(3) | 730,
	/*  D4 */ _BLOCK(3) | 774,
	/* Eb4 */ _BLOCK(3) | 820,
	/*  E4 */ _BLOCK(3) | 869,
	/*  F4 */ _BLOCK(3) | 920,
	/* F#4 */ _BLOCK(3) | 975,
	/*  G4 */ _BLOCK(4) | 516,
	/* Ab4 */ _BLOCK(4) | 547,
	/*  A4 */ _BLOCK(4) | 580,
	/* Bb4 */ _BLOCK(4) | 614,
	/*  B4 */ _BLOCK(4) | 651,
	/*  C5 */ _BLOCK(4) | 689,
	/* C#5 */ _BLOCK(4) | 730,
	/*  D5 */ _BLOCK(4) | 774,
	/* Eb5 */ _BLOCK(4) | 820,
	/*  E5 */ _BLOCK(4) | 869,
	/*  F5 */ _BLOCK(4) | 920,
	/* F#5 */ _BLOCK(4) | 975,
	/*  G5 */ _BLOCK(5) | 516,
	/* Ab5 */ _BLOCK(5) | 547,
	/*  A5 */ _BLOCK(5) | 580,
	/* Bb5 */ _BLOCK(5) | 614,
	/*  B5 */ _BLOCK(5) | 651,
	/*  C6 */ _BLOCK(5) | 689,
	/* C#6 */ _BLOCK(5) | 730,
	/*  D6 */ _BLOCK(5) | 774,
	/* Eb6 */ _BLOCK(5) | 820,
	/*  E6 */ _BLOCK(5) | 869,
	/*  F6 */ _BLOCK(5) | 920,
	/* F#6 */ _BLOCK(5) | 975,
	/*  G6 */ _BLOCK(6) | 516,
	/* Ab6 */ _BLOCK(6) | 547,
	/*  A6 */ _BLOCK(6) | 580,
	/* Bb6 */ _BLOCK(6) | 614,
	/*  B6 */ _BLOCK(6) | 651,
	/*  C7 */ _BLOCK(6) | 689,
	/* C#7 */ _BLOCK(6) | 730,
	/*  D7 */ _BLOCK(6) | 774,
	/* Eb7 */ _BLOCK(6) | 820,
	/*  E7 */ _BLOCK(6) | 869,
	/*  F7 */ _BLOCK(6) | 920,
	/* F#7 */ _BLOCK(6) | 975,
	/*  G7 */ _BLOCK(7) | 516,
	/* Ab7 */ _BLOCK(7) | 547,
	/*  A7 */ _BLOCK(7) | 580,
	/* Bb7 */ _BLOCK(7) | 614,
};

#undef _BLOCK
