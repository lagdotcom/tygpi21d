#ifndef DOSJUN_NPC_H
#define DOSJUN_NPC_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"
#include "strings.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define NPC_PADDING			21

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct npc {
	VERSION_HEADER;
	str_id name_id;
	UINT8 pronouns;
	file_id portrait_id;
	INT16 attitude;

	char padding[NPC_PADDING];
} npc;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

npc *Duplicate_NPC(npc *npc);
void Free_NPC(npc *npc);
bool Read_NPC(FILE *fp, npc *npc);
bool Write_NPC(FILE *fp, npc *npc);

#endif
