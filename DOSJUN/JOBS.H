#ifndef DOSJUN_CLASS_H
#define DOSJUN_CLASS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "save.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define JOB_LEVELS	10

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Jobs(void);
void Initialise_Jobs(void);
void Level_Up(character *c);
void Set_Job(character *c, job_id job);

#endif
