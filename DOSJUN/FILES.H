#ifndef DOSJUN_FILES_H
#define DOSJUN_FILES_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include "common.h"
#include "io.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define FILE_MAGIC		"JUN"
#define VERSION_1		1
#define VERSION_NOW		VERSION_1

#define NAME_SIZE		24
#define PARTY_SIZE		6

typedef enum {
	slWeapon,
	slOffHand,
	slHead,
	slBody,
	slFeet,
	slAccessory,
} slot_id;
#define NUM_SLOTS		6
#define INVENTORY_SIZE	10

typedef enum {
	sMaxHP,
	sMaxMP,
	sMinDamage,
	sMaxDamage,
	sArmour,
	sStrength,
	sDexterity,
	sIntelligence,
	sHP,
	sMP,
} stat_id;
#define NUM_STATS		10

typedef enum {
	vfNone = 0,
	vfEquipped = 1
} inventoryflags;

typedef enum {
	wtNormal,
	wtDoor,
	wtLockedDoor,
} walltypes;

typedef enum {
	tiNone = 0,
	tiImpassable = 1,
} tileflags;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

#define VERSION_HEADER char magic[3]; unsigned char version

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

#define Set_Version_Header(h) strncpy(h.magic, FILE_MAGIC, 3); h.version = VERSION_1
#define Check_Version_Header(h) if (!Check_Version(h.magic, h.version)) { fclose(fp); return false; }

#endif
