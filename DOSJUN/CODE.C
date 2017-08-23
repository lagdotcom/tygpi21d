/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/*#define TRACE_CODE*/

/* G L O B A L S ///////////////////////////////////////////////////////// */

#ifdef TRACE_CODE
#include <stdio.h>
FILE *trace;

noexport void Code_Error(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	fputs("[ERROR] ", trace);
	vfprintf(trace, format, args);
	fputs("\n", trace);

	va_end(args);
}

#else

#define Code_Error printf

#endif

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport bytecode Next_Op(host *h)
{
	h->pc = h->next_pc;
	h->next_pc++;
	return h->code[h->pc];
}

noexport int Next_Literal(host *h)
{
	bytecode little = Next_Op(h);
	bytecode big = Next_Op(h);
	return little | (big << 8);
}

noexport void Push_Stack(host *h, int value)
{
	h->stack[h->sp++] = value;
#ifdef TRACE_CODE
	fprintf(trace, " (%d pushed) ", value);
#endif
}

noexport int Pop_Stack(host *h)
{
	int value = h->stack[--h->sp];
#ifdef TRACE_CODE
	fprintf(trace, " (%d popped) ", value);
#endif
	return value;
}

noexport int Bool(int result)
{
	return result ? 1 : 0;
}

/* O P C O D E  F U N C T I O N S //////////////////////////////////////// */

noexport void Push_Global(host *h)
{
	bytecode index = Next_Op(h);
#ifdef TRACE_CODE
	fprintf(trace, "push global.%d", index);
#endif
	Push_Stack(h, h->globals[index]);
}

noexport void Push_Local(host *h)
{
	bytecode index = Next_Op(h);
#ifdef TRACE_CODE
	fprintf(trace, "push local.%d", index);
#endif
	Push_Stack(h, h->locals[index]);
}

noexport void Push_Temp(host *h)
{
	bytecode index = Next_Op(h);
#ifdef TRACE_CODE
	fprintf(trace, "push temp.%d", index);
#endif
	Push_Stack(h, h->temps[index]);
}

noexport void Push_Internal(host *h)
{
	internal_id index = Next_Op(h);
#ifdef TRACE_CODE
	fprintf(trace, "push internal.%d", index);
#endif

	switch (index) {
		case internalX:
			Push_Stack(h, gSave.header.x);
			return;

		case internalY:
			Push_Stack(h, gSave.header.y);
			return;

		case internalFacing:
			Push_Stack(h, gSave.header.facing);
			return;
	}

	Code_Error("Unknown internal #%d", index);
}

noexport void Push_Literal(host *h)
{
	int value = Next_Literal(h);
#ifdef TRACE_CODE
	fprintf(trace, "push literal");
#endif
	Push_Stack(h, value);
}

noexport void Pop_Global(host *h)
{
	bytecode index = Next_Op(h);
#ifdef TRACE_CODE
	fprintf(trace, "pop global.%d", index);
#endif
	h->globals[index] = Pop_Stack(h);
}

noexport void Pop_Local(host *h)
{
	bytecode index = Next_Op(h);
#ifdef TRACE_CODE
	fprintf(trace, "pop local.%d", index);
#endif
	h->locals[index] = Pop_Stack(h);
}

noexport void Pop_Temp(host *h)
{
	bytecode index = Next_Op(h);
#ifdef TRACE_CODE
	fprintf(trace, "pop temp.%d", index);
#endif
	h->temps[index] = Pop_Stack(h);
}

noexport void Add(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
#ifdef TRACE_CODE
	fprintf(trace, "add");
#endif
	Push_Stack(h, left + right);
}

noexport void Sub(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
#ifdef TRACE_CODE
	fprintf(trace, "sub");
#endif
	Push_Stack(h, left - right);
}

noexport void Mul(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
#ifdef TRACE_CODE
	fprintf(trace, "mul");
#endif
	Push_Stack(h, left * right);
}

noexport void Div(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
#ifdef TRACE_CODE
	fprintf(trace, "div");
#endif
	Push_Stack(h, left / right);
}

noexport void Eq(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
#ifdef TRACE_CODE
	fprintf(trace, "eq");
#endif
	Push_Stack(h, Bool(left == right));
}

noexport void Neq(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
#ifdef TRACE_CODE
	fprintf(trace, "neq");
#endif
	Push_Stack(h, Bool(left != right));
}

noexport void Lt(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
#ifdef TRACE_CODE
	fprintf(trace, "lt");
#endif
	Push_Stack(h, Bool(left < right));
}

noexport void Lte(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
#ifdef TRACE_CODE
	fprintf(trace, "lte");
#endif
	Push_Stack(h, Bool(left <= right));
}

noexport void Gt(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
#ifdef TRACE_CODE
	fprintf(trace, "gt");
#endif
	Push_Stack(h, Bool(left > right));
}

noexport void Gte(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
#ifdef TRACE_CODE
	fprintf(trace, "gte");
#endif
	Push_Stack(h, Bool(left >= right));
}

noexport void Jump(host *h)
{
	h->next_pc = Next_Literal(h);
#ifdef TRACE_CODE
	fprintf(trace, "jump %04x", h->next_pc);
#endif
}

noexport void JumpFalse(host *h)
{
	int offset = Next_Literal(h);
#ifdef TRACE_CODE
	fprintf(trace, "jnz %04x", offset);
#endif
	if (!Pop_Stack(h)) h->next_pc = offset;
}

noexport void Return(host *h)
{
	h->running = false;
	h->result = 0;
#ifdef TRACE_CODE
	fprintf(trace, "return");
#endif
}

noexport void Combat(host *h)
{
	encounter_id combat = Pop_Stack(h);

#ifdef TRACE_CODE
	fprintf(trace, "combat #%d", combat);
#endif

	Start_Combat(combat);
}

noexport void PcSpeak(host *h)
{
	char buffer[300];
	int pc = Pop_Stack(h);
	string_id string = Pop_Stack(h);

#ifdef TRACE_CODE
	fprintf(trace, "pcspeak %d, #%d", pc, string);
#endif
	
	sprintf(buffer, "%s:\n%s", gSave.characters[pc].name, gZone.code_strings[string]);
	Show_Game_String(buffer, true);
}

noexport void Text(host *h)
{
	string_id string = Pop_Stack(h);

#ifdef TRACE_CODE
	fprintf(trace, "text #%d", string);
#endif

	Show_Game_String(gZone.code_strings[string], true);
}

noexport void Unlock(host *h)
{
	tile *tile;
	coord x = Pop_Stack(h);
	coord y = Pop_Stack(h);
	direction dir = Pop_Stack(h);

#ifdef TRACE_CODE
	fprintf(trace, "unlock %d, %d, %d", x, y, dir);
#endif

	tile = TILE(gZone, x, y);
	tile->walls[dir].type = wtDoor;

	/* TODO: unlock wall on other side too? */
}

noexport void GiveItem(host *h)
{
	bool result;
	int pc = Pop_Stack(h);
	item_id item = Pop_Stack(h);
	int qty = Pop_Stack(h);

#ifdef TRACE_CODE
	fprintf(trace, "giveitem %d, %d, %d", pc, item, qty);
#endif

	result = Add_To_Inventory(pc, item, qty);
	Push_Stack(h, result);
}

noexport void EquipItem(host *h)
{
	bool result;
	int pc = Pop_Stack(h);
	item_id item = Pop_Stack(h);

#ifdef TRACE_CODE
	fprintf(trace, "equipitem %d, %d", pc, item);
#endif

	result = Equip_Item(pc, item);
	Push_Stack(h, result);
}

noexport void SetTileDescription(host *h)
{
	coord x = Pop_Stack(h);
	coord y = Pop_Stack(h);
	string_id string = Pop_Stack(h);

#ifdef TRACE_CODE
	fprintf(trace, "settiledescription %d, %d, #%d", x, y, string);
#endif

	TILE(gZone, x, y)->description = string;
}

noexport void SetTileColour(host *h)
{
	coord x = Pop_Stack(h);
	coord y = Pop_Stack(h);
	int f = Pop_Stack(h);
	colour c = Pop_Stack(h);

#ifdef TRACE_CODE
	fprintf(trace, "settilecolour %d, %d, %d, %d", x, y, facing, colour);
#endif

	switch (f) {
		case Up:
			TILE(gZone, x, y)->ceil = c;
			break;

		case Down:
			TILE(gZone, x, y)->floor = c;
			break;

		default:
			TILE(gZone, x, y)->walls[f].texture = c;
			break;
	}

	redraw_fp = true;
}

noexport void Teleport(host *h)
{
	zone_id zone = Pop_Stack(h);
	coord x = Pop_Stack(h);
	coord y = Pop_Stack(h);
	int facing = Pop_Stack(h);
	int transition = Pop_Stack(h);

#ifdef TRACE_CODE
	fprintf(trace, "teleport %d, %d, %d, %d, %d", zone, x, y, facing, transition);
#endif

	switch (transition) {
		/* TODO */
	}

	gSave.header.zone = zone;
	gSave.header.x = x;
	gSave.header.y = y;
	gSave.header.facing = facing;

	redraw_description = true;
	redraw_fp = true;
	trigger_on_enter = true;
}

/* M A I N /////////////////////////////////////////////////////////////// */

noexport void Run_Code_Instruction(host *h, bytecode op)
{
#ifdef TRACE_CODE
	fprintf(trace, "%04x: [%02x] ", h->pc, op);
#endif

	switch (op) {
		case coPushGlobal:	Push_Global(h); return;
		case coPushLocal:	Push_Local(h); return;
		case coPushTemp:	Push_Temp(h); return;
		case coPushInternal:Push_Internal(h); return;
		case coPushLiteral:	Push_Literal(h); return;
		case coPopGlobal:	Pop_Global(h); return;
		case coPopLocal:	Pop_Local(h); return;
		case coPopTemp:		Pop_Temp(h); return;

		case coAdd:			Add(h); return;
		case coSub:			Sub(h); return;
		case coMul:			Mul(h); return;
		case coDiv:			Div(h); return;

		case coEQ:			Eq(h); return;
		case coNEQ:			Neq(h); return;
		case coLT:			Lt(h); return;
		case coLTE:			Lte(h); return;
		case coGT:			Gt(h); return;
		case coGTE:			Gte(h); return;

		case coJump:		Jump(h); return;
		case coJumpFalse:	JumpFalse(h); return;
		case coReturn:		Return(h); return;

		case coCombat:		Combat(h); return;
		case coPcSpeak:		PcSpeak(h); return;
		case coText:		Text(h); return;
		case coUnlock:		Unlock(h); return;
		case coGiveItem:	GiveItem(h); return;
		case coEquipItem:	EquipItem(h); return;
		case coSetTileDescription: SetTileDescription(h); return;
		case coSetTileColour: SetTileColour(h); return;
		case coTeleport:	Teleport(h); return;
	}

	h->running = false;
	h->result = -1;
	Code_Error("Unknown opcode: %2x", op);

#ifdef TRACE_CODE
	fprintf(trace, "UNKNOWN");
#endif
}

noexport bool Run_Code_Host(host *h)
{
	h->running = true;

	while (h->running) {
		h->next_pc = h->pc + 1;
		Run_Code_Instruction(h, h->code[h->pc]);
#ifdef TRACE_CODE
		fprintf(trace, "\n");
#endif

		h->pc = h->next_pc;
	}

	return h->result == 0;
}

bool Run_Code(script_id id)
{
	bool result;
	host h;

#ifdef TRACE_CODE
	trace = fopen("JUNTRACE.TXT", "w");
#endif

	h.code = gZone.scripts[id];
	h.globals = gSave.script_globals;
	h.locals = gSave.script_locals[gSave.header.zone];
	h.temps = SzAlloc(MAX_TEMPS, int, "Run_Code.temps");
	h.stack = SzAlloc(MAX_STACK, int, "Run_Code.stack");

	h.pc = 0;
	h.result = 0;
	h.sp = 0;

	result = Run_Code_Host(&h);

	Free(h.temps);
	Free(h.stack);

#ifdef TRACE_CODE
	fclose(trace);
#endif

	return result;
}
