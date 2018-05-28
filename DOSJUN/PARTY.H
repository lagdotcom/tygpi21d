#ifndef DOSJUN_PARTY_H
#define DOSJUN_PARTY_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef unsigned char pcnum;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Add_to_Inventory(pc *pc, item_id iid, unsigned char qty);
void Draw_Character_Status(pcnum index, int x, int y);
void Draw_Party_Status(void);
bool Equip_Item(pc *pc, item_id iid);
item *Get_Equipped_Item(pc *pc, itemslot sl);
item *Get_Equipped_Weapon(pc *pc, bool primary);

/* Get the PC whose index this is */
pc *Get_Pc(pcnum index);

stat_value Get_Pc_Stat(pc *c, statistic st);

bool In_Front_Row(pc *pc);
void Pc_Select(pcnum index);

bool Remove_Item(pc *pc, item_id iid);

void Show_Pc_Screen(pcnum index);

#endif
