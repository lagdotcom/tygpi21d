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
	coPushFlag,
	coGetPCInSlot,
	coPushLiteral = 0x1F,
	coPopGlobal = 0x20,
	coPopLocal,
	coPopTemp,
	coPopFlag,
	/* Arithmetic */
	coAdd = 0x30,
	coSub,
	coMul,
	coDiv,
	/* Bitwise */
	coAnd,
	coOr,
	/* Unary */
	coInvert,
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
	coChoosePcName,
	coJoinParty,
	coLeaveParty,
	coChoosePcPortrait,
	coChoosePcPronouns,
	coSetAttitude,
	coGetAttitude,
	coGotoXY,
	coInParty,
	coHasBuff,
	coAddBuff,
	coRemoveBuff,
	coFade,
	coPlaceItem,
	coShowImage,
	coWait,
} code_op;

typedef enum code_internal {
	internalX,
	internalY,
	internalFacing,
	internalDanger,
	internalJustMoved,
	internalSuccess__Unused,

	internalReturn__Unused,
	internalPartyFull,	/* TODO */
	internalZone,
	internalEventAttacker,
	internalEventTarget,
	internalEventPC,
	internalEventItem,

	internalInvalid = 0xFF
} code_internal;

typedef struct script {
	bytecode *code;
} script;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Code(void);
void Free_Script(script *s);
void Initialise_Code(void);
int Run_Code(file_id id);
int Run_Event_Code(file_id id, event_data *data);
bool Read_Script(FILE *fp, script *s);

#endif
