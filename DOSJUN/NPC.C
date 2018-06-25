/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Free_NPC(npc *npc)
{

}

bool Read_NPC(FILE *fp, npc *n)
{
	fread(n, sizeof(npc), 1, fp);
	return true;
}

bool Write_NPC(FILE *fp, npc *n)
{
	fwrite(n, sizeof(npc), 1, fp);
	return true;
}

npc *Duplicate_NPC(npc *org)
{
	npc *dup = SzAlloc(1, npc, "Duplicate_NPC");

	memcpy(dup, org, sizeof(npc));
	return dup;
}
