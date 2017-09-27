#ifndef DOSJUN_CAMPAIGN_H
#define DOSJUN_CAMPAIGN_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define CAMPAIGN_HEADER_PADDING		23

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	VERSION_HEADER;

	zone_id start_zone;
	coord start_x, start_y;
	direction start_facing;
	zone_id num_zones;

	char unused[CAMPAIGN_HEADER_PADDING];
} campaign_header;

typedef struct {
	campaign_header header;

	char **zones;
} campaign;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Initialise_Campaign(campaign *c);
bool Load_Campaign(char *filename, campaign *c);
void Free_Campaign(campaign *c);
bool Save_Campaign(char *filename, campaign *c);

#endif
