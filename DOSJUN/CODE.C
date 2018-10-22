/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "features.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

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
	event_data *edata;

	int pc, next_pc;
	int sp;
	bool running;
	int result;
	int call_result;
} code_host;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport int call_depth = 0;
noexport file_id converse_npc = 0;
noexport file_id conversation_state = INVALID_SCRIPT;
noexport char **option_menu;
noexport file_id *option_state;
noexport int num_options = 0;
noexport char *formatting_buf;
noexport point2d text_pos;
noexport RGB_color faded_colour;

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
#if CODE_DEBUG_STACK
	Log("C|Push_Stack: %d", value);
#endif
}

noexport int Pop_Stack(code_host *h)
{
	int value;
	assert(h->sp > 0, "Pop_Stack: stack is empty");

	value = h->stack[--h->sp];
#if CODE_DEBUG_STACK
	Log("C|Pop_Stack: %d", value);
#endif
	return value;
}

noexport int Bool(int result)
{
	return result ? 1 : 0;
}

noexport char *Get_Npc_Name(file_id ref)
{
	npc *npc = Lookup_File_Chained(gSave, ref);
	return Resolve_String(npc->name_id);
}

noexport char *Get_PC_Name(file_id ref)
{
	pc *pc = Lookup_File_Chained(gSave, ref);
	return pc->name;
}

/* O P C O D E  F U N C T I O N S //////////////////////////////////////// */

noexport void Push_Global(code_host *h)
{
	int index = Next_Literal(h);
#if CODE_DEBUG
	Log("C|Push_Global.%d", index);
#endif
	assert(index < gGlobals->num_globals, "Push_Global: index too high");
	Push_Stack(h, h->globals[index]);
}

noexport void Push_Flag(code_host *h)
{
	int index = Next_Literal(h);
	int offset, bit;

#if CODE_DEBUG
	Log("C|Push_Flag.%d", index);
#endif

	assert(index < gGlobals->num_flags, "Push_Flag: index too high");
	offset = index >> 5;
	bit = 1 << (index % 16);

	Push_Stack(h, Bool(h->flags[offset] & bit));
}

noexport void Push_Local(code_host *h)
{
	int index = Next_Literal(h);
#if CODE_DEBUG
	Log("C|Push_Local.%d", index);
#endif
	assert(index < gOverlay->num_locals, "Push_Local: index too high");
	Push_Stack(h, h->locals[index]);
}

noexport void Push_Temp(code_host *h)
{
	bytecode index = Next_Op(h);
#if CODE_DEBUG
	Log("C|Push_Temp.%d", index);
#endif
	assert(index < MAX_TEMPS, "Push_Temp: index too high");
	Push_Stack(h, h->temps[index]);
}

#define CHECK_EDATA() { \
	if (h->edata == null) { \
		Log("C|Push_Internal.%d: not in event", index); \
		Push_Stack(h, 0); \
		return; \
	} \
}

noexport void Push_Internal(code_host *h)
{
	internal_id index = Next_Op(h);
#if CODE_DEBUG
	Log("C|Push_Internal.%d", index);
#endif

	switch (index) {
		case internalX:
			Push_Stack(h, gParty->x);
			return;

		case internalY:
			Push_Stack(h, gParty->y);
			return;

		case internalZone:
			Push_Stack(h, gParty->zone);
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

		case internalEventAttacker:
			CHECK_EDATA();
			Push_Stack(h, h->edata->attacker);
			return;

		case internalEventItem:
			CHECK_EDATA();
			Push_Stack(h, h->edata->item);
			return;

		case internalEventPC:
			CHECK_EDATA();
			Push_Stack(h, h->edata->pc);
			return;

		case internalEventTarget:
			CHECK_EDATA();
			Push_Stack(h, h->edata->target);
			return;
	}

	dief("Push_Internal: Unknown internal #%d", index);
}

noexport void Push_Literal(code_host *h)
{
	int value = Next_Literal(h);
#if CODE_DEBUG
	Log("C|Push_Literal: %d", value);
#endif
	Push_Stack(h, value);
}

noexport void Pop_Global(code_host *h)
{
	int index = Next_Literal(h);
#if CODE_DEBUG
	Log("C|Pop_Global.%d", index);
#endif
	assert(index < gGlobals->num_globals, "Pop_Global: index too high");
	h->globals[index] = Pop_Stack(h);
}

noexport void Pop_Flag(code_host *h)
{
	int index = Next_Literal(h);
	int set = Pop_Stack(h);
	int offset, bit;

#if CODE_DEBUG
	Log("C|Pop_Flag.%d", index);
#endif

	assert(index < gGlobals->num_flags, "Pop_Flag: index too high");
	offset = index >> 5;
	bit = 1 << (index % 16);

	if (set) {
		h->flags[offset] |= bit;
	} else {
		h->flags[offset] &= ~bit;
	}
}

noexport void Pop_Local(code_host *h)
{
	int index = Next_Literal(h);
#if CODE_DEBUG
	Log("C|Pop_Local.%d", index);
#endif
	assert(index < gOverlay->num_locals, "Pop_Local: index too high");
	h->locals[index] = Pop_Stack(h);
}

noexport void Pop_Temp(code_host *h)
{
	bytecode index = Next_Op(h);
#if CODE_DEBUG
	Log("C|Pop_Temp.%d", index);
#endif
	assert(index < MAX_TEMPS, "Pop_Temp: index too high");
	h->temps[index] = Pop_Stack(h);
}

noexport void Add(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Add: %d + %d", left, right);
#endif
	Push_Stack(h, left + right);
}

noexport void Sub(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Sub: %d - %d", left, right);
#endif
	Push_Stack(h, left - right);
}

noexport void Mul(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Mul: %d * %d", left, right);
#endif
	Push_Stack(h, left * right);
}

noexport void Div(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Div: %d / %d", left, right);
#endif
	Push_Stack(h, left / right);
}

noexport void And(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|And: %d & %d", left, right);
#endif
	Push_Stack(h, left & right);
}

noexport void Or(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Or: %d | %d", left, right);
#endif
	Push_Stack(h, left | right);
}

noexport void Invert(code_host *h)
{
	int value = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Invert: %d", value);
#endif
	Push_Stack(h, Bool(!value));
}

noexport void Eq(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Eq: %d == %d", left, right);
#endif
	Push_Stack(h, Bool(left == right));
}

noexport void Neq(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Neq: %d != %d", left, right);
#endif
	Push_Stack(h, Bool(left != right));
}

noexport void Lt(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Lt: %d < %d", left, right);
#endif
	Push_Stack(h, Bool(left < right));
}

noexport void Lte(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Lte: %d <= %d", left, right);
#endif
	Push_Stack(h, Bool(left <= right));
}

noexport void Gt(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Gt: %d > %d", left, right);
#endif
	Push_Stack(h, Bool(left > right));
}

noexport void Gte(code_host *h)
{
	int right = Pop_Stack(h);
	int left = Pop_Stack(h);
#if CODE_DEBUG
	Log("C|Gte: %d >= %d", left, right);
#endif
	Push_Stack(h, Bool(left >= right));
}

noexport void Jump(code_host *h)
{
	h->next_pc = Next_Literal(h);
#if CODE_DEBUG
	Log("C|Jump: @%04x", h->next_pc);
#endif
}

noexport void JumpFalse(code_host *h)
{
	int offset = Next_Literal(h);
#if CODE_DEBUG
	Log("C|JumpFalse: @%04x", offset);
#endif
	if (!Pop_Stack(h)) h->next_pc = offset;
}

noexport void Return(code_host *h)
{
	h->running = false;
	h->result = 0;
#if CODE_DEBUG
	Log("C|Return @%04x", h->pc);
#endif
}

noexport void Call(code_host *h)
{
	file_id script_id = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|Call %d", script_id);
#endif

	h->call_result = Run_Code(script_id);
}

noexport void Combat(code_host *h)
{
	encounter_id combat = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|Combat %d", combat);
#endif

	Start_Combat(combat);
}

noexport void PcSpeak(code_host *h)
{
	str_id s_id = Pop_Stack(h);
	file_id pc_id = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|PcSpeak %d, %d", pc_id, s_id);
#endif
	
	sprintf(formatting_buf, "%s:\n%s", Get_PC_Name(pc_id), Resolve_String(s_id));
	Show_Game_String_Context(formatting_buf, true, pc_id, 0);
}

noexport void PcAction(code_host *h)
{
	str_id s_id = Pop_Stack(h);
	file_id pc_id = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|PcAction %d, %d", pc_id, s_id);
#endif

	sprintf(formatting_buf, "%s %s", Get_PC_Name(pc_id), Resolve_String(s_id));
	Show_Game_String_Context(formatting_buf, true, pc_id, 0);
}

noexport void NpcSpeak(code_host *h)
{
	str_id s_id = Pop_Stack(h);
	file_id npc_id = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|NpcSpeak %d, %d", npc_id, s_id);
#endif

	sprintf(formatting_buf, "%s:\n%s", Get_Npc_Name(npc_id), Resolve_String(s_id));
	Show_Game_String_Context(formatting_buf, true, npc_id, 0);
}

noexport void NpcAction(code_host *h)
{
	str_id s_id = Pop_Stack(h);
	file_id npc_id = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|NpcAction %d, %d", npc_id, s_id);
#endif

	sprintf(formatting_buf, "%s %s", Get_Npc_Name(npc_id), Resolve_String(s_id));
	Show_Game_String_Context(formatting_buf, true, npc_id, 0);
}

noexport void Text(code_host *h)
{
	str_id s_id = Pop_Stack(h);
	char *s = Resolve_String(s_id);
	box2d box;
	point2d end;

#if CODE_DEBUG
	Log("C|Text %d", s_id);
#endif

	switch (gState)
	{
		case gsCutscene:
			/* TODO: this is kind of dumb */
			box.start = text_pos;
			box.end.x = SCREEN_WIDTH;
			box.end.y = SCREEN_HEIGHT;
			end = Show_Formatted_String(s, 0, 0, &box, gFont, 0, true);
			text_pos.y = end.y;
			break;

		default:
			Show_Game_String(s, true);
			break;
	}
}

noexport void Unlock(code_host *h)
{
	tile *tile;
	dir dir = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|Unlock %d, %d, %d", x, y, dir);
#endif

	tile = TILE(gZone, x, y);
	tile->walls[dir].type = wtDoor;

	/* TODO: unlock wall on other side too? */
}

noexport void GiveItem(code_host *h)
{
	int qty = Pop_Stack(h);
	file_id item_id = Pop_Stack(h);
	file_id pc_id = Pop_Stack(h);
	pc *pc = Lookup_File_Chained(gSave, pc_id);

#if CODE_DEBUG
	Log("C|GiveItem %d, %d, %d", pc->name, item_id, qty);
#endif

	Push_Stack(h, Bool(Add_to_Inventory(pc, item_id, qty)));
	Add_PC_to_Save(gSave, pc, pc_id);
}

noexport void EquipItem(code_host *h)
{
	file_id item_id = Pop_Stack(h);
	file_id pc_id = Pop_Stack(h);
	pc *pc = Lookup_File_Chained(gSave, pc_id);

#if CODE_DEBUG
	Log("C|EquipItem %d, %d", pc->name, item_id);
#endif

	Push_Stack(h, Bool(Equip_Item(pc, item_id)));
	Add_PC_to_Save(gSave, pc, pc_id);
}

noexport void SetTileDescription(code_host *h)
{
	str_id s_id = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|SetTileDescription %d, %d, #%d", x, y, s_id);
#endif

	TILE(gZone, x, y)->description = s_id;
}

noexport void SetTileColour(code_host *h)
{
	file_id grf_id = Pop_Stack(h);
	dir face = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|SetTileColour %d, %d, %d, %d", x, y, face, grf_id);
#endif

	switch (face) {
		case dUp:
			TILE(gZone, x, y)->ceil = grf_id;
			break;

		case dDown:
			TILE(gZone, x, y)->floor = grf_id;
			break;

		default:
			TILE(gZone, x, y)->walls[face].texture = grf_id;
			break;
	}

	redraw_fp = true;
}

noexport void SetTileThing(code_host *h)
{
	file_id grf_id = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|SetTileThing %d, %d, %d", x, y, grf_id);
#endif

	TILE(gZone, x, y)->thing = grf_id;
	redraw_fp = true;
}

noexport void Teleport(code_host *h)
{
	int transition = Pop_Stack(h);
	dir facing = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);
	file_id zone_id = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|Teleport %d, %d, %d, %d, %d", zone_id, x, y, facing, transition);
#endif

	switch (transition) {
		/* TODO */
	}

	gParty->x = x;
	gParty->y = y;
	gParty->facing = facing;

	if (gParty->zone != zone_id) {
		Move_to_Zone(zone_id);
	}

	redraw_description = true;
	redraw_fp = true;
	trigger_on_enter = true;
	trigger_zone_enter = true;
}

noexport void SetDanger(code_host *h)
{
	UINT8 danger = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|SetDanger %d", danger);
#endif

	gParty->danger = danger;
}

noexport void Safe(code_host *h)
{
#if CODE_DEBUG
	Log("%s", "C|Safe");
#endif

	can_save = true;
}

noexport void RemoveWall(code_host *h)
{
	tile *tile;
	dir face = Pop_Stack(h);
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|RemoveWall %d, %d, %d", x, y, face);
#endif

	tile = TILE(gZone, x, y);
	tile->walls[face].texture = 0;
	tile->walls[face].texture = wtNormal;

	redraw_fp = true;
}

noexport void Refresh(code_host *h)
{
#if CODE_DEBUG
	Log("%s", "C|Refresh");
#endif

	switch (gState) {
		case gsCutscene:
			redraw_everything = true;
			Fill_Double_Buffer(0);
			Show_Double_Buffer();
			break;

		default:
			Draw_FP();
			Draw_Party_Status();
			break;
	}
}

noexport void AddItem(code_host *h)
{
	int qty = Pop_Stack(h);
	file_id item_id = Pop_Stack(h);
	pcnum num;
	bool success = false;

#if CODE_DEBUG
	Log("C|AddItem %d, %d", item_id, qty);
#endif

	success = Add_Item_to_Party(item_id, qty, &num);
	Push_Stack(h, Bool(success));
}

noexport void Music(code_host *h)
{
	file_id sng_id = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|Music %d", sng_id);
#endif

	Start_Music(sng_id);
}

noexport void Converse(code_host *h)
{
	conversation_state = Pop_Stack(h);
	converse_npc = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|Converse %d, %d", converse_npc, conversation_state);
#endif

	gState = gsConverse;
	/* TODO: show conversation UI */
}

noexport void EndConverse(code_host *h)
{
#if CODE_DEBUG
	Log("%s", "C|EndConverse");
#endif

	gState = gsDungeon;

	/* TODO: remove conversation UI */
	redraw_everything = true;
}

noexport void ChangeState(code_host *h)
{
	conversation_state = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|ChangeState %d", conversation_state);
#endif

	if (gState != gsConverse) {
		dief("ChangeState: tried to change to %d while not in gsConverse", conversation_state);
		return;
	}
}

noexport void Option(code_host *h)
{
	str_id s_id = Pop_Stack(h);
	file_id script_id = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|Option %d, %d", script_id, s_id);
#endif

	if (num_options == MAX_OPTIONS) {
		dief("Option: too many Options, max is %d", MAX_OPTIONS);
		return;
	}

	option_state[num_options] = script_id;
	option_menu[num_options] = Resolve_String(s_id);
	num_options++;
}

noexport void ChoosePcName(code_host *h)
{
	file_id pc_id = Pop_Stack(h);
	int row_height;
	pc *pc;
	char *name;

#if CODE_DEBUG
	Log("C|ChoosePcName %d", pc_id);
#endif

	pc = Lookup_File_Chained(gSave, pc_id);
	if (pc == null) {
		dief("ChoosePcName: invalid resource ID: %d", pc_id);
		return;
	}

	row_height = Char_Height(gFont, ' ');
	name = SzAlloc(CODE_NAME_BUFFER_SIZE, char, "ChoosePcName");
	Draw_Font(text_pos.x, text_pos.y, 0, "Choose a name:", gFont, true);
	Input_String(text_pos.x, text_pos.y + row_height, name, CODE_NAME_BUFFER_SIZE);
	text_pos.y += row_height * 2;

	pc->name = name;
	pc->header.name_id = 0;
	Add_PC_to_Save(gSave, pc, pc_id);
}

noexport void ChoosePcPortrait(code_host *h)
{
	file_id pc_id = Pop_Stack(h);
	pc *pc;

#if CODE_DEBUG
	Log("C|ChoosePcPortrait %d", pc_id);
#endif

	pc = Lookup_File_Chained(gSave, pc_id);
	if (pc == null) {
		dief("ChoosePcPortrait: invalid resource ID: %d", pc_id);
		return;
	}

	pc->header.portrait_id = Pick_Portrait(pc->header.portrait_id);
	Add_PC_to_Save(gSave, pc, pc_id);
}

noexport void ChoosePcPronouns(code_host *h)
{
	char **menu;
	file_id pc_id = Pop_Stack(h);
	pc *pc;
	int i;

#if CODE_DEBUG
	Log("C|ChoosePcPronouns %d", pc_id);
#endif

	pc = Lookup_File_Chained(gSave, pc_id);
	if (pc == null) {
		dief("ChoosePcPronouns: invalid resource ID: %d", pc_id);
		return;
	}

	menu = SzAlloc(proNobody, char*, "ChoosePcPronouns");
	for (i = 0; i < proNobody; i++)
		menu[i] = Get_Pronoun_Name(i);
	Draw_Font(text_pos.x, text_pos.y, 0, "Choose a pronoun:", gFont, false);
	i = Input_Menu(menu, proNobody, text_pos.x, text_pos.y + Char_Height(gFont, ' '));
	Free(menu);

	pc->header.pronouns = i;
	Add_PC_to_Save(gSave, pc, pc_id);
}

noexport void SetAttitude(code_host *h)
{
	int attitude = Pop_Stack(h);
	file_id _id = Pop_Stack(h);
	djn_file *file;
	pc *pc;
	npc *npc;

#if CODE_DEBUG
	Log("C|SetAttitude %d, %d", _id, attitude);
#endif

	file = Lookup_File_Entry(gSave, _id, true, true);
	if (file->type == ftPC) {
		pc = file->object;
		pc->header.attitude = attitude;
		Add_PC_to_Save(gSave, pc, _id);
	} else {
		npc = file->object;
		npc->attitude = attitude;
		Add_NPC_to_Save(gSave, npc, _id);
	}
}

noexport void GetAttitude(code_host *h)
{
	file_id _id = Pop_Stack(h);
	djn_file *file;
	pc *pc;
	npc *npc;

#if CODE_DEBUG
	Log("C|GetAttitude %d", _id);
#endif

	file = Lookup_File_Entry(gSave, _id, true, true);
	if (file->type == ftPC) {
		pc = file->object;
		Push_Stack(h, pc->header.attitude);
	} else {
		npc = file->object;
		Push_Stack(h, npc->attitude);
	}
}

noexport void GotoXY(code_host *h)
{
	text_pos.y = Pop_Stack(h);
	text_pos.x = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|GotoXY %d, %d", text_pos.x, text_pos.y);
#endif
}

noexport void JoinParty(code_host *h)
{
	file_id pc_id = Pop_Stack(h);
	pc *pc;
	int i;
	bool result = false;

#if CODE_DEBUG
	Log("C|JoinParty %d", pc_id);
#endif

	/* TODO: this is technically wrong; might be a gap before the PC */
	for (i = 0; i < PARTY_SIZE; i++) {
		if (gParty->members[i] == pc_id)
			break;

		if (gParty->members[i] == 0) {
			pc = Lookup_File_Chained(gSave, pc_id);
			gParty->members[i] = pc_id;
			Add_PC_to_Save(gSave, pc, pc_id);
			result = true;
			break;
		}
	}

	Push_Stack(h, Bool(result));
}

noexport void LeaveParty(code_host *h)
{
	file_id pc_id = Pop_Stack(h);
	int i;
	bool success = false;

#if CODE_DEBUG
	Log("C|LeaveParty %d", pc_id);
#endif

	for (i = 0; i < PARTY_SIZE; i++) {
		if (gParty->members[i] == pc_id) {
			gParty->members[i] = 0;
			success = true;
			break;
		}
	}

	Push_Stack(h, Bool(success));
}

noexport void InParty(code_host *h)
{
	file_id pc_id = Pop_Stack(h);
	int i;
	bool success = false;

#if CODE_DEBUG
	Log("C|InParty %d", pc_id);
#endif

	for (i = 0; i < PARTY_SIZE; i++) {
		if (gParty->members[i] == pc_id) {
			success = true;
			break;
		}
	}

	Push_Stack(h, Bool(success));
}

noexport void AddBuff(code_host *h)
{
	int arg2 = Pop_Stack(h);
	int arg1 = Pop_Stack(h);
	int duration = Pop_Stack(h);
	buff_id buff = Pop_Stack(h);
	file_id pc_id = Pop_Stack(h);
	pc *pc;

#if CODE_DEBUG
	Log("C|AddBuff %d, %d, %d, %d, %d", pc_id, buff, duration, arg1, arg2);
#endif

	pc = Lookup_File_Chained(gSave, pc_id);
	Add_to_List(pc->buffs, Make_Buff(buff, duration, arg1, arg2, "AddBuff"));

	Add_PC_to_Save(gSave, pc, pc_id);
}

noexport void RemoveBuff(code_host *h)
{
	buff_id buff = Pop_Stack(h);
	file_id pc_id = Pop_Stack(h);
	pc *pc;

#if CODE_DEBUG
	Log("C|RemoveBuff %d, %d", pc_id, buff);
#endif

	pc = Lookup_File_Chained(gSave, pc_id);
	Push_Stack(h, Bool(Remove_Buff_from_PC(pc_id, buff)));

	/* This shouldn't be necessary, but who knows */
	Add_PC_to_Save(gSave, pc, pc_id);
}

noexport void HasBuff(code_host *h)
{
	buff_id buff_id = Pop_Stack(h);
	file_id pc_id = Pop_Stack(h);
	buff *b;
	pc *pc;
	int i;
	bool result = false;

#if CODE_DEBUG
	Log("C|HasBuff %d, %d", pc_id, buff_id);
#endif

	pc = Lookup_File_Chained(gSave, pc_id);

	for (i = 0; i < pc->buffs->size; i++) {
		b = pc->buffs->items[i];
		if (b->id == buff_id) {
			result = true;
			break;
		}
	}

	Push_Stack(h, Bool(result));
}

noexport void Fade(code_host *h)
{
	int delay = Pop_Stack(h);
	int i = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|Fade %d, %d", i, delay);
#endif

	if (i) {
		faded_colour = gPalette->colours[i];
		Fade_To(gPalette, &faded_colour, delay);
	} else {
		Fade_From(gPalette, &faded_colour, delay);
	}
}

noexport void GetPCInSlot(code_host *h)
{
	int slot = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|GetPCInSlot %d", slot);
#endif

	Push_Stack(h, Get_PC_ID(slot));
}

noexport void PlaceItem(code_host *h)
{
	int y = Pop_Stack(h);
	int x = Pop_Stack(h);
	file_id ref = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|PlaceItem #%d, %d, %d", ref, x, y);
#endif

	Add_Item_to_Tile(x, y, ref);
}

noexport void ShowImage(code_host *h)
{
	file_id ref;
	int img;
	grf *g;
	point2d p;

	img = Pop_Stack(h);
	p.y = Pop_Stack(h);
	p.x = Pop_Stack(h);
	ref = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|ShowImage #%d, %d, %d, %d", ref, p.x, p.y, img);
#endif

	g = Lookup_File(gDjn, ref, true);
	if (!g) {
		dief("ShowImage: invalid resource ID: %d", ref);
		return;
	}

	redraw_everything = true;
	Draw_GRF(&p, g, img, 0);
	Show_Double_Buffer();
}

noexport void Wait(code_host *h)
{
#if CODE_DEBUG
	Log("%s", "C|Wait");
#endif

	Show_Double_Buffer();
	Get_Next_Scan_Code();
}

noexport void Listen(code_host *h)
{
	event_expiry ee = Pop_Stack(h);
	file_id script = Pop_Stack(h);
	event_id ev = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|Listen e%d, #%d, x%d", ev, script, ee);
#endif

	Add_Script_Listener(ev, ee, script);
}

noexport void HasItem(code_host *h)
{
	file_id item = Pop_Stack(h);

#if CODE_DEBUG
	Log("C|HasItem %d", item);
#endif

	Push_Stack(h, Bool(Party_Has_Item(item)));
}

noexport void TakeItem(code_host *h)
{
	file_id item = Pop_Stack(h);
	bool result;

#if CODE_DEBUG
	Log("C|TakeItem %d", item);
#endif

	result = Party_Take_Item(item);
	Push_Stack(h, Bool(result));

	if (result) {
		/* TODO: save the PC whose item got taken */
	}
}

noexport void ItemAt(code_host *h)
{
	coord y = Pop_Stack(h);
	coord x = Pop_Stack(h);
	file_id item = Pop_Stack(h);
	int i;
	itempos *ip;

#if CODE_DEBUG
	Log("C|ItemAt #%d, %d, %d", item, x, y);
#endif

	for (i = 0; i < gOverlay->items->size; i++) {
		ip = List_At(gOverlay->items, i);

		if (ip->x == x && ip->y == y && ip->item == item) {
			Push_Stack(h, Bool(true));
			return;
		}
	}

	Push_Stack(h, Bool(false));
	return;
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
		case coPushFlag:	Push_Flag(h); return;
		case coPopGlobal:	Pop_Global(h); return;
		case coPopLocal:	Pop_Local(h); return;
		case coPopTemp:		Pop_Temp(h); return;
		case coPopFlag:		Pop_Flag(h); return;

		case coAdd:			Add(h); return;
		case coSub:			Sub(h); return;
		case coMul:			Mul(h); return;
		case coDiv:			Div(h); return;

		case coAnd:			And(h); return;
		case coOr:			Or(h); return;
		case coInvert:		Invert(h); return;

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
		case coHasItem:		HasItem(h); return;
		case coTakeItem:	TakeItem(h); return;
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

		case coChoosePcName: ChoosePcName(h); return;
		case coChoosePcPortrait: ChoosePcPortrait(h); return;
		case coChoosePcPronouns: ChoosePcPronouns(h); return;
		case coSetAttitude: SetAttitude(h); return;
		case coGetAttitude: GetAttitude(h); return;
		case coJoinParty:	JoinParty(h); return;
		case coLeaveParty:	LeaveParty(h); return;
		case coInParty:		InParty(h); return;

		case coGotoXY:		GotoXY(h); return;
		case coAddBuff:		AddBuff(h); return;
		case coHasBuff:		HasBuff(h); return;
		case coRemoveBuff:	RemoveBuff(h); return;
		case coFade:		Fade(h); return;
		case coGetPCInSlot:	GetPCInSlot(h); return;
		case coPlaceItem:	PlaceItem(h); return;
		case coShowImage:	ShowImage(h); return;
		case coWait:		Wait(h); return;
		case coListen:		Listen(h); return;
		case coItemAt:		ItemAt(h); return;
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

noexport file_id Pick_Option(void)
{
	int choice;

	if (num_options == 0) {
		return INVALID_SCRIPT;
	}

	/* TODO */
	choice = Input_Menu(option_menu, num_options, 20, 20);
	return option_state[choice];
}

noexport void Reset_Host(code_host *h, file_id id)
{
	script *s = Lookup_File_Chained(gDjn, id);
	
	h->code = s->code;
	h->pc = 0;
	h->result = 0;
	h->sp = 0;
}

int Run_Event_Code(file_id id, event_data *data)
{
	bool result;
	code_host *h;

	Log("Run_Code: %d (%p)", id, data);

	h = SzAlloc(1, code_host, "Run_Code.host");

	h->globals = gGlobals->globals;
	h->flags = gGlobals->flags;
	h->locals = gOverlay->locals;
	h->temps = SzAlloc(MAX_TEMPS, int, "Run_Code.temps");
	h->stack = SzAlloc(MAX_STACK, int, "Run_Code.stack");
	h->edata = data;

	Reset_Host(h, id);
	call_depth++;

	do {
		conversation_state = INVALID_SCRIPT;
		result = Run_Code_Host(h);

		/* Continue active conversation */
		if (gState == gsConverse && call_depth == 1) {
#if CODE_DEBUG
			Log("%s", "Run_Code: still in conversation");
#endif
			if (conversation_state == INVALID_SCRIPT) {
				conversation_state = Pick_Option();
#if CODE_DEBUG
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

int Run_Code(file_id id)
{
	return Run_Event_Code(id, null);
}

void Initialise_Code(void)
{
	option_menu = SzAlloc(MAX_OPTIONS, char *, "Initialise_Code.option_menu");
	option_state = SzAlloc(MAX_OPTIONS, file_id, "Initialise_Code.option_state");
	formatting_buf = SzAlloc(CODE_BUFFER_SIZE, char, "Initialise_Code.buf");
}

void Free_Code(void)
{
	Free(option_menu);
	Free(option_state);
	Free(formatting_buf);
}

void Free_Script(script *s)
{
	Free(s->code);
}

bool Read_Script(FILE *fp, script *s)
{
	s->code = SzAlloc(current_reading_file->size, char, "Read_Script");
	if (s->code == null) goto _dead;
	fread(s->code, current_reading_file->size, 1, fp);
	
	return true;

_dead:
	die("Read_Script: out of memory");
	return false;
}
