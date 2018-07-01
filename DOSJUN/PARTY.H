#ifndef DOSJUN_PARTY_H
#define DOSJUN_PARTY_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef INT8 pcnum;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Add_to_Inventory(pc *pc, file_id iid, unsigned char qty);
void Draw_Character_Status(pcnum index, int x, int y);
void Draw_Party_Status(void);
pc *Duplicate_PC(pc *pc);
bool Equip_Item(pc *pc, file_id iid);
void Free_PC(pc *pc);
item *Get_Equipped_Item(pc *pc, itemslot sl);
file_id Get_Equipped_Item_Id(pc *pc, itemslot sl);
item *Get_Equipped_Weapon(pc *pc, bool primary);
file_id Get_Equipped_Weapon_Id(pc *pc, bool primary);

/* Get the PC whose index this is */
pc *Get_PC(pcnum index);

file_id Get_PC_ID(pcnum index);

stat_value Get_PC_Stat(pc *c, statistic st);

bool In_Front_Row(pc *pc);
void Pc_Select(pcnum index);
file_id Pick_Portrait(file_id org);
bool Read_PC(FILE *fp, pc *pc);
bool Remove_Item(pc *pc, file_id iid);
void Show_Pc_Screen(pcnum index);
bool Write_PC(FILE *fp, pc *pc);

#endif
