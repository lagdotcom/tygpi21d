#ifndef DOSJUN_PARTY_H
#define DOSJUN_PARTY_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef unsigned char pcnum;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Add_to_Inventory(pcnum pc, item_id iid, unsigned char qty);
void Draw_Character_Status(pcnum pc, int x, int y);
void Draw_Party_Status(void);
bool Equip_Item(pcnum pc, item_id iid);
item *Get_Equipped_Item(pcnum pc, itemslot sl);
item *Get_Equipped_Weapon(pcnum pc, bool primary);

/* Get the PC whose index this is */
character *Get_Pc(pcnum pc);

stat_value Get_Pc_Stat(character *c, statistic st);

bool In_Front_Row(pcnum pc);
void Pc_Select(pcnum pc);

bool Remove_Item(pcnum pc, item_id iid);

void Show_Pc_Screen(pcnum pc);

#endif
