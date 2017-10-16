#ifndef DOSJUN_PARTY_H
#define DOSJUN_PARTY_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Add_to_Inventory(unsigned char pc, item_id iid, unsigned char qty);
void Draw_Character_Status(int index, int x, int y);
void Draw_Party_Status(void);
bool Equip_Item(unsigned char pc, item_id iid);
item *Get_Equipped_Weapon(unsigned char pc);

/* Get the PC whose index this is */
character *Get_Pc(int pc);

bool In_Front_Row(unsigned char pc);
void Pc_Select(int num);

#endif
