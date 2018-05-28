/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define CODE_LOG		1
#define CODE_LOG_STACK	0
#define MAX_OPTIONS		10

#define INVALID_SCRIPT	32767

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct code_host {
	bytecode *code;
	int *globals;
	int *flags;
	int *locals;
	int *temps;
	int *stack;

	int pc, next_pc;
	int sp;
	bool running;
	int result;
	int call_result;
} code_host;


/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport int call_depth = 0;
noexport int converse_npc = 0;
noexport script_id conversation_state = INVALID_SCRIPT;
noexport char **option_menu;
noexport script_id *option_state;
noexport int num_options = 0;
noexport char *formatting_buf;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport bytecode Next_Op(code_host *h)
{
	h->pc = h->next_pc;
	h->next_pc++;
	return h->code[h->pc];
}

noexport int Next_Literal(code_host *h)
{
	bytecode little = Next_Op(h);
	bytecode big = Next_Op(h);
	return little | (big << 8);
}

noexport void Push_Stack(code_host *h, int value)
{
	assert(h->sp < MAX_STACK, "Push_Stack: stack is full");

	h->stack[h->sp++] = value;
#if CODE_LOG_STACK
	Log("C|Push_Stack: %d", value);
#endif
}

noexport int Pop_Stack(code_host *h)
{
	int value;
	assert(h->sp > 0, "Pop_Stack: stack is empty");

	value = h->stack[--h->sp];
#if CODE_LOG_STACK
	Log("C|Pop_Stack: %d", value);
#endif
	return value;
}

noexport int Bool(int result)
{
	return result ? 1 : 0;
}

/* TODO */
noexport char *Get_Npc_Name(file_id ref)
{
	return "NPC";

	/*
	npc *npc = Lookup_File(gSave, ref);
	return npc->name;
	*/
}

noexport char *Get_Pc_Name(file_id ref)
{
	pc *pc = Lookup_File(gSave, ref);
	return pc->name;
}

/* O P C O D E  F U N C T I O N S //////////////////////////////////////// */

noexport void Push_Global(code_host *h)
{
	bytecode index = Next_Op(h);
#if CODE_LOG
	Log("C|Push_Global.%d", index);
#endif
	Push_Stack(h, h->globals[index]);
}

noexport void Push_Local(code_host *h)
{
	bytecode index = Next_Op(h);
#if CODE_LOG
	Log("C|Push_Local.%d", index);
#endif
	Push_Stack(h, h->locals[index]);
}

noexport void Push_Temp(code_host *h)
{
	bytecode index = Next_Op(h);
#if CODE_LOG
	Log("C|Push_Temp.%d", index);
#endif
	Push_Stack(h, h->temps[index]);
}

noexport void Push_Internal(code_host *h)
{
	internal_id index = Next_Op(h);
#if CODE_LOG
	Log("C|Push_Internal.%d", index);
#endif

	switch (index) {
		case internalX:
			Push_Stack(h, gParty->x);
			return;

		case internalY:
			Push_Stack(h, gParty->y);
			return;

		case internalFacing:
			Push_Stack(h, gParty->facing);
			return;

		case internalDanger:
			Push_Stack(h, gParty->danger);
			return;

		case internalJustMoved:
			Push_Stack(h, Bool(just_moved));
			return;

		case internalSuccess:
			Push_Stack(h, h->result);
			return;
	}

	dief("Push_Internal: Unknown internal #%d", index);
}

noexport void Push_Literal(code_host *h)
{
	int value = Next_Literal(h);
#if CODE_LOG
	Log("C|Push_Literal: %d", value);
#endif
	Push_Stack(h, value);
}

noexport void Pop_Global(code_host *h)
{
	bytecode index = Next_Op(h);
#if CODE_LOG
	Log("C|Pop_Global.%d", index);
#endif
	h->globals[index] = Pop_Stack(h);
}

noexport void Pop_Local(code_host *h)
{
	bytecode index = Next_Op(h);
#if CODE_LOG
	Log("C|Pop_Local.%d", index);
#endif
	h->locals[index] = Pop_Stack(h);
}

noexport void Pop_Temp(code_host *h)
{
	bytecode index = Next_Op(h);
#if CODE_LOG
	Log("C|Pop_Temp.%d", index);
#endif
	h->temps[index] = Pop_Stack(h);
}

noexport void Add(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|Add: %d + %d", left, right);
#endif
	Push_Stack(h, left + right);
}

noexport void Sub(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|Sub: %d - %d", left, right);
#endif
	Push_Stack(h, left - right);
}

noexport void Mul(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|Mul: %d * %d", left, right);
#endif
	Push_Stack(h, left * right);
}

noexport void Div(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|Div: %d / %d", left, right);
#endif
	Push_Stack(h, left / right);
}

noexport void And(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|And: %d & %d", left, right);
#endif
	Push_Stack(h, left & right);
}

noexport void Or(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|Or: %d | %d", left, right);
#endif
	Push_Stack(h, left | right);
}

noexport void Eq(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|Eq: %d == %d", left, right);
#endif
	Push_Stack(h, Bool(left == right));
}

noexport void Neq(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|Neq: %d != %d", left, right);
#endif
	Push_Stack(h, Bool(left != right));
}

noexport void Lt(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|Lt: %d < %d", left, right);
#endif
	Push_Stack(h, Bool(left < right));
}

noexport void Lte(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|Lte: %d <= %d", left, right);
#endif
	Push_Stack(h, Bool(left <= right));
}

noexport void Gt(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|Gt: %d > %d", left, right);
#endif
	Push_Stack(h, Bool(left > right));
}

noexport void Gte(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_LOG
	Log("C|Gte: %d >= %d", left, right);
#endif
	Push_Stack(h, Bool(left >= right));
}

noexport void Jump(code_host *h)
{
	h->next_pc = Next_Literal(h);
#if CODE_LOG
	Log("C|Jump: @%04x", h->next_pc);
#endif
}

noexport void JumpFalse(code_host *h)
{
	int offset = Next_Literal(h);
#if CODE_LOG
	Log("C|JumpFalse: @%04x", offset);
#endif
	if (!Pop_Stack(h)) h->next_pc = offset;
}

noexport void Return(code_host *h)
{
	h->running = false;
	h->result = 0;
#if CODE_LOG
	Log("%s", "C|Return @%04x", h->pc);
#endif
}

noexport void Call(code_host *h)
{
	int func = Pop_Stack(h);

#if CODE_LOG
	Log("C|Call %d", func);
#endif

	h->call_result = Run_Code(func);
}

noexport void Combat(code_host *h)
{
	encounter_id combat = Pop_Stack(h);

#if CODE_LOG
	Log("C|Combat %d", combat);
#endif

	Start_Combat(combat);
}

noexport void PcSpeak(code_host *h)
{
	string_id string = Pop_Stack(h);
	int pc = Pop_Stack(h);

#if CODE_LOG
	Log("C|PcSpeak %d, %d", pc, string);
#endif
	
	sprintf(formatting_buf, "%s:\n%s", Get_Pc_Name(pc), Resolve_String(string));
	Show_Game_String(formatting_buf, true);
}

noexport void PcAction(code_host *h)
{
	string_id string = Pop_Stack(h);
	int pc = Pop_Stack(h);

#if CODE_LOG
	Log("C|PcAction %d, %d", pc, string);
#endif

	sprintf(formatting_buf, "%s %s", Get_Pc_Name(pc), Resolve_String(string));
	Show_Game_String(formatting_buf, true);
}

noexport void NpcSpeak(code_host *h)
{
	string_id string = Pop_Stack(h);
	int npc = Pop_Stack(h);

#if CODE_LOG
	Log("C|NpcSpeak %d, %d", npc, string);
#endif

	sprintf(formatting_buf, "%s:\n%s", Get_Npc_Name(npc), Resolve_String(string));
	Show_Game_String(formatting_buf, true);
}

noexport void NpcAction(code_host *h)
{
	string_id string = Pop_Stack(h);
	int npc = Pop_Stack(h);

#if CODE_LOG
	Log("C|NpcAction %d, %d", npc, string);
#endif

	sprintf(formatting_buf, "%s %s", Get_Npc_Name(npc), Resolve_String(string));
	Show_Game_String(formatting_buf, true);
}

noexport void Text(code_host *h)
{
	string_id string = Pop_Stack(h);

#if CODE_LOG
	Log("C|Text %d", string);
#endif

	Show_Game_String(Resolve_String(string), true);
}

noexport void Unlock(code_host *h)
{
	tile *tile;
	dir dir = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);

#if CODE_LOG
	Log("C|Unlock %d, %d, %d", x, y, dir);
#endif

	tile = TILE(gZone, x, y);
	tile->walls[dir].type = wtDoor;

	/* TODO: unlock wall on other side too? */
}

noexport void GiveItem(code_host *h)
{
	int qty = Pop_Stack(h);
	item_id item = Pop_Stack(h);
	file_id ref = Pop_Stack(h);
	pc *pc = Lookup_File(gSave, ref);

#if CODE_LOG
	Log("C|GiveItem %d, %d, %d", pc->name, item, qty);
#endif

	h->result = Add_to_Inventory(pc, item, qty);
}

noexport void EquipItem(code_host *h)
{
	item_id item = Pop_Stack(h);
	file_id ref = Pop_Stack(h);
	pc *pc = Lookup_File(gSave, ref);

#if CODE_LOG
	Log("C|EquipItem %d, %d", pc->name, item);
#endif

	h->result = Equip_Item(pc, item);
}

noexport void SetTileDescription(code_host *h)
{
	string_id string = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);

#if CODE_LOG
	Log("C|SetTileDescription %d, %d, #%d", x, y, string);
#endif

	TILE(gZone, x, y)->description = string;
}

noexport void SetTileColour(code_host *h)
{
	colour c = Pop_Stack(h);
	int f = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);

#if CODE_LOG
	Log("C|SetTileColour %d, %d, %d, %d", x, y, f, c);
#endif

	switch (f) {
		case dUp:
			TILE(gZone, x, y)->ceil = c;
			break;

		case dDown:
			TILE(gZone, x, y)->floor = c;
			break;

		default:
			TILE(gZone, x, y)->walls[f].texture = c;
			break;
	}

	redraw_fp = true;
}

noexport void SetTileThing(code_host *h)
{
	thing_id th = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);

#if CODE_LOG
	Log("C|SetTileThing %d, %d, %d", x, y, th);
#endif

	TILE(gZone, x, y)->thing = th;
	redraw_fp = true;
}

noexport void Teleport(code_host *h)
{
	int transition = Pop_Stack(h);
	dir facing = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);
	zone_id zone = Pop_Stack(h);

#if CODE_LOG
	Log("C|Teleport %d, %d, %d, %d, %d", zone, x, y, facing, transition);
#endif

	switch (transition) {
		/* TODO */
	}

	gParty->x = x;
	gParty->y = y;
	gParty->facing = facing;

	if (gParty->zone != zone) {
		Move_to_Zone(zone);
	}

	redraw_description = true;
	redraw_fp = true;
	trigger_on_enter = true;
}

noexport void SetDanger(code_host *h)
{
	int danger = Pop_Stack(h);

#if CODE_LOG
	Log("C|SetDanger %d", danger);
#endif

	gParty->danger = danger;
}

noexport void Safe(code_host *h)
{
#if CODE_LOG
	Log("%s", "C|Safe");
#endif

	can_save = true;
}

noexport void RemoveWall(code_host *h)
{
	tile *tile;
	dir dir = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);

#if CODE_LOG
	Log("C|RemoveWall %d, %d, %d", x, y, dir);
#endif

	tile = TILE(gZone, x, y);
	tile->walls[dir].texture = 0;
	tile->walls[dir].texture = wtNormal;

	redraw_fp = true;
}

noexport void Refresh(code_host *h)
{
#if CODE_LOG
	Log("%s", "C|Refresh");
#endif

	Draw_FP();
	Draw_Party_Status();
}

noexport void AddItem(code_host *h)
{
	pcnum index;
	int qty = Pop_Stack(h);
	item_id item = Pop_Stack(h);
	pc *pc;

#if CODE_LOG
	Log("C|AddItem %d, %d", item, qty);
#endif

	h->result = false;
	for (index = 0; index < PARTY_SIZE; index++) {
		pc = Get_Pc(index);
		if (!pc) continue;

		h->result = Add_to_Inventory(pc , item, qty);
		if (h->result) break;
	}
}

noexport void Music(code_host *h)
{
	file_id id = Pop_Stack(h);

#if CODE_LOG
	Log("C|Music %d", id);
#endif

	Play_Music(Lookup_File(gDjn, id));
}

noexport void Converse(code_host *h)
{
	conversation_state = Pop_Stack(h);
	converse_npc = Pop_Stack(h);

#if CODE_LOG
	Log("C|Converse %d, %d", converse_npc, conversation_state);
#endif

	gState = gsConverse;
	/* TODO: show conversation UI */
}

noexport void EndConverse(code_host *h)
{
#if CODE_LOG
	Log("%s", "C|EndConverse");
#endif

	gState = gsDungeon;

	/* TODO: remove conversation UI */
	redraw_everything = true;
}

noexport void ChangeState(code_host *h)
{
	conversation_state = Pop_Stack(h);

#if CODE_LOG
	Log("C|ChangeState %d", conversation_state);
#endif

	if (gState != gsConverse) {
		dief("ChangeState: tried to change to %d while not in gsConverse", conversation_state);
		return;
	}
}

noexport void Option(code_host *h)
{
	string_id string = Pop_Stack(h);
	script_id state = Pop_Stack(h);

#if CODE_LOG
	Log("C|Option %d, %d", state, string);
#endif

	if (num_options == MAX_OPTIONS) {
		dief("Option: too many Options, max is %d", MAX_OPTIONS);
		return;
	}

	option_state[num_options] = state;
	option_menu[num_options] = Resolve_String(string);
	num_options++;
}

/* M A I N /////////////////////////////////////////////////////////////// */

noexport void Run_Code_Instruction(code_host *h, bytecode op)
{
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

		case coAnd:			And(h); return;
		case coOr:			Or(h); return;

		case coEQ:			Eq(h); return;
		case coNEQ:			Neq(h); return;
		case coLT:			Lt(h); return;
		case coLTE:			Lte(h); return;
		case coGT:			Gt(h); return;
		case coGTE:			Gte(h); return;

		case coJump:		Jump(h); return;
		case coJumpFalse:	JumpFalse(h); return;
		case coReturn:		Return(h); return;
		case coCall:		Call(h); return;

		case coCombat:		Combat(h); return;
		case coPcSpeak:		PcSpeak(h); return;
		case coText:		Text(h); return;
		case coUnlock:		Unlock(h); return;
		case coGiveItem:	GiveItem(h); return;
		case coEquipItem:	EquipItem(h); return;
		case coSetTileDescription: SetTileDescription(h); return;
		case coSetTileColour: SetTileColour(h); return;
		case coTeleport:	Teleport(h); return;
		case coSetTileThing: SetTileThing(h); return;
		case coSetDanger:	SetDanger(h); return;
		case coSafe:		Safe(h); return;
		case coRemoveWall:	RemoveWall(h); return;
		case coRefresh:		Refresh(h); return;
		case coAddItem:		AddItem(h); return;
		case coMusic:		Music(h); return;

		case coConverse:	Converse(h); return;
		case coEndConverse:	EndConverse(h); return;
		case coChangeState:	ChangeState(h); return;
		case coOption:		Option(h); return;
		case coPcAction:	PcAction(h); return;
		case coNpcAction:	NpcAction(h); return;
		case coNpcSpeak:	NpcSpeak(h); return;
	}

	h->running = false;
	h->result = -1;
	dief("Run_Code_Instruction: Unknown opcode: %02x", op);
}

noexport int Run_Code_Host(code_host *h)
{
	h->running = true;

	while (h->running) {
		h->next_pc = h->pc + 1;
		Run_Code_Instruction(h, h->code[h->pc]);
		h->pc = h->next_pc;
	}

	return h->result;
}

noexport script_id Pick_Option(void)
{
	int choice;

	if (num_options == 0) {
		return INVALID_SCRIPT;
	}

	/* TODO */
	choice = Input_Menu(option_menu, num_options, 20, 20);
	return option_state[choice];
}

noexport void Reset_Host(code_host *h, script_id id)
{
	h->code = Lookup_File(gDjn, id);
	h->pc = 0;
	h->result = 0;
	h->sp = 0;
}

int Run_Code(script_id id)
{
	bool result;
	code_host *h;

	Log("Run_Code: %d", id);

	h = SzAlloc(1, code_host, "Run_Code.host");

	h->globals = gGlobals->globals;
	h->flags = gGlobals->flags;
	h->locals = gOverlay->locals;
	h->temps = SzAlloc(MAX_TEMPS, int, "Run_Code.temps");
	h->stack = SzAlloc(MAX_STACK, int, "Run_Code.stack");

	Reset_Host(h, id);
	call_depth++;

	do {
		conversation_state = INVALID_SCRIPT;
		result = Run_Code_Host(h);

		/* Continue active conversation */
		if (gState == gsConverse && call_depth == 1) {
#if CODE_LOG
			Log("%s", "Run_Code: still in conversation");
#endif
			if (conversation_state == INVALID_SCRIPT) {
				conversation_state = Pick_Option();
#if CODE_LOG
				Log("Run_Code: player picked option %d", conversation_state);
#endif
			}

			if (conversation_state != INVALID_SCRIPT) {
				Reset_Host(h, conversation_state);
				num_options = 0;

				Log("Run_Code: state %d", conversation_state);
			} else {
				EndConverse(h);
			}
		}
	} while (gState == gsConverse);

	Free(h->temps);
	Free(h->stack);
	Free(h);

	call_depth--;
	return result;
}

void Initialise_Code(void)
{
	option_menu = SzAlloc(MAX_OPTIONS, char *, "Initialise_Code.option_menu");
	option_state = SzAlloc(MAX_OPTIONS, script_id, "Initialise_Code.option_state");
	formatting_buf = SzAlloc(300, char, "Initialise_Code.buf");
}

void Free_Code(void)
{
	Free(option_menu);
	Free(option_state);
	Free(formatting_buf);
}
