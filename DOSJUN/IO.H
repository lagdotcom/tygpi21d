#ifndef DOSJUN_IO_H
#define DOSJUN_IO_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Check_Version(char *magic, unsigned char version);
void Free_Directory_Listing(char **listing, int count);
char **Get_Directory_Listing(char *pattern, int *count);
unsigned char Get_Next_Scan_Code(void);
char *Read_LengthString(FILE *fp);
void Write_LengthString(char *string, FILE *fp);

#endif
