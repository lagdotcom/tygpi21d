#ifndef GAMELIB_GRAPH_9
#define GAMELIB_GRAPH_9

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern char far *driver_ptr;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Voc_Load_Driver(void);
char far *Voc_Load_Sound(char *filename, unsigned char *header_length);
int Voc_Init_Driver(void);
void Voc_Set_Status_Addr(char far *status);
int Voc_Play_Sound(unsigned char far* addr, unsigned char header_length);
void Voc_Get_Version(void);
int Voc_Terminate_Driver(void);
void Voc_Set_Port(unsigned int port);
void Voc_Set_Speaker(unsigned int on);
int Voc_Stop_Sound(void);
int Voc_Pause_Sound(void);
int Voc_Continue_Sound(void);
int Voc_Break_Sound(void);
void Voc_Set_DMA(unsigned int dma);
void Voc_Unload_Sound(char far *sound_ptr);

#endif
