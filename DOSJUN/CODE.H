#ifndef DOSJUN_CODE_H
#define DOSJUN_CODE_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MAX_GLOBALS		20
#define MAX_LOCALS		20
#define MAX_TEMPS		20
#define MAX_CONSTS		200
#define MAX_STACK		20

typedef enum code_op {
	coNOP,
	/* Stack */
	coPushGlobal = 0x10,
	coPushLocal = 0x11,
	coPushTemp = 0x12,
	coPushInternal = 0x13,
	coPushLiteral = 0x1F,
	coPopGlobal = 0x20,
	coPopLocal = 0x21,
	coPopTemp = 0x22,
	/* Arithmetic */
	coAdd = 0x30,
	coSub = 0x31,
	coMul = 0x32,
	coDiv = 0x33,
	/* Bitwise */
	coAnd = 0x34,
	coOr = 0x35,
	/* Comparison */
	coEQ = 0x40,
	coNEQ = 0x41,
	coLT = 0x42,
	coLTE = 0x43,
	coGT = 0x44,
	coGTE = 0x45,
	/* Branching */
	coJump = 0x50,
	coJumpFalse = 0x51,
	coReturn = 0x52,
	/* Builtins */
	coCombat = 0xA0,
	coPcSpeak = 0xA1,
	coText = 0xA2,
	coUnlock = 0xA3,
	coGiveItem = 0xA4,
	coEquipItem = 0xA5,
	coSetTileDescription = 0xA6,
	coSetTileColour = 0xA7,
	coTeleport = 0xA8,
	coSetTileThing = 0xA9,
	coSetDanger = 0xAA,
	coSafe = 0xAB,
	coRemoveWall = 0xAC,
	coRefresh = 0xAD,
	coAddItem = 0xAE,
	coMusic = 0xAF,
} code_op;

typedef enum code_internal {
	internalX,
	internalY,
	internalFacing,
	internalDanger,
	internalJustMoved,
	internalSuccess,

	internalInvalid = 0xFF
} code_internal;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct code_host {
	bytecode* code;
	int* globals;
	int* locals;
	int* temps;
	int* stack;

	int pc, next_pc;
	int sp;
	bool running;
	int result;
} code_host;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Run_Code(script_id id);

#endif
