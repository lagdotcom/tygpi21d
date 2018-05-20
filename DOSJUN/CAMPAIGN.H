#ifndef DOSJUN_CAMPAIGN_H
#define DOSJUN_CAMPAIGN_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define CAMPAIGN_HEADER_PADDING		18

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct campaign_header {
	VERSION_HEADER;

	UINT16 global_count;
	UINT16 flag_count;
	UINT16 script_id;
	UINT16 name_id;
	UINT16 desc_id;

	unsigned char unused[CAMPAIGN_HEADER_PADDING];
} campaign_header;

typedef struct campaign {
	campaign_header header;

	char **zones;
} campaign;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Load_Campaign(char *filename, campaign *c);
void Free_Campaign(campaign *c);
bool Read_Campaign(FILE *fp, campaign *c);
bool Save_Campaign(char *filename, campaign *c);

#endif
