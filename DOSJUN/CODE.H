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
	coNOP = 0,
	/* Stack */
	coPushGlobal = 0x10,
	coPushLocal,
	coPushTemp,
	coPushInternal,
	coPushLiteral = 0x1F,
	coPopGlobal = 0x20,
	coPopLocal,
	coPopTemp,
	/* Arithmetic */
	coAdd = 0x30,
	coSub,
	coMul,
	coDiv,
	/* Bitwise */
	coAnd,
	coOr,
	/* Comparison */
	coEQ = 0x40,
	coNEQ,
	coLT,
	coLTE,
	coGT,
	coGTE,
	/* Branching */
	coJump = 0x50,
	coJumpFalse,
	coReturn,
	coCall,
	/* Builtins */
	coCombat = 0xA0,
	coPcSpeak,
	coText,
	coUnlock,
	coGiveItem,
	coEquipItem,
	coSetTileDescription,
	coSetTileColour,
	coTeleport,
	coSetTileThing,
	coSetDanger,
	coSafe,
	coRemoveWall,
	coRefresh,
	coAddItem,
	coMusic,
	coConverse,
	coEndConverse,
	coChangeState,
	coPcAction,
	coNpcSpeak,
	coNpcAction,
	coOption,
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

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Code(void);
void Initialise_Code(void);
int Run_Code(script_id id);

#endif
