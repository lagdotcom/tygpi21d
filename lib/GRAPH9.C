/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

char far *driver_ptr = NULL;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Voc_Load_Driver(void)
{
	/* This function loads the CT-VOICE.DRV, which allows digitized effects
	to be played. */
	int driver_handle;
	unsigned segment, num_para, bytes_read;
	char far *data_ptr;

	/* Open the driverfile. */
	driver_handle = open("CT-VOICE.DRV", O_RDONLY|O_BINARY);

	/* Allocate the memory. */
	num_para = 1 + filelength(driver_handle) / 16;
	allocmem(num_para, &segment);

	/* Point the driver pointer to data area. */
	driver_ptr = MK_FP(segment, 0);

	/* Load in the driver code. */
	data_ptr = driver_ptr;
	do {
		bytes_read = read(driver_handle, data_ptr, 0x4000);
		data_ptr += bytes_read;
	} while (bytes_read == 0x4000);

	/* Close the file. */
	close(driver_handle);
}

char far *Voc_Load_Sound(char *filename, unsigned char *header_length)
{
	/* This function loads a sound off disk into memory and returns a pointer
	to the data. */
	char far *temp_ptr, *data_ptr;
	unsigned int sum = 0;
	int sound_handle;
	unsigned segment, num_para, bytes_read;

	/* Open the sound file. */
	sound_handle = open(filename, O_RDONLY|O_BINARY);

	/* Allocate the memory. */
	num_para = 1 + filelength(sound_handle) / 16;
	allocmem(num_para, &segment);

	/* Point the data pointer to the allocated data area. */
	data_ptr = MK_FP(segment, 0);

	/* Load in the sound data. */
	temp_ptr = data_ptr;
	do {
		bytes_read = read(sound_handle, temp_ptr, 0x4000);
		temp_ptr += bytes_read;
		sum += bytes_read;
	} while (bytes_read == 0x4000);

	/* Make sure it's a VOC file. */
	if ((data_ptr[0] != 'C') || (data_ptr[1] != 'r')) {
		printf("\n%s is not a VOC file!", filename);
		freemem(FP_SEG(data_ptr));
		return 0;
	}

	*header_length = (unsigned char)data_ptr[20];

	/* Close the file. */
	close(sound_handle);

	return data_ptr;
}

int Voc_Init_Driver(void)
{
	/* This function initializes the driver and returns the status. */
	char far *drv = driver_ptr;
	int status;

	asm mov bx, 3;			/* function 3: initialize driver */
	asm call drv;
	asm mov status, ax;

	printf("\nDriver Initialized");
	return status;
}

void Voc_Set_Status_Addr(char far *status)
{
	/* This function sets the address of the global status word in the
	driver. */
	char far *drv = driver_ptr;
	unsigned segm, offm;

	/* Extract the segment and offset of the status variable. */
	segm = FP_SEG(status);
	offm = FP_OFF(status);

	asm mov bx, 5;			/* function 5: set status var address */
	asm mov es, segm;
	asm mov di, offm;
	asm call drv;
}

int Voc_Play_Sound(unsigned char far* addr, unsigned char header_length)
{
	/* This function plays a preloaded VOC file. */
	char far *drv = driver_ptr;
	unsigned segm, offm;

	segm = FP_SEG(addr);
	offm = FP_OFF(addr) + header_length;
		/* add in the length of the header */

	asm mov bx, 6;			/* function 6: play VOC */
	asm mov es, segm;
	asm mov di, offm;
	asm call drv;
}

void Voc_Get_Version(void)
{
	/* This function prints out the version of the driver. */
	char far *drv = driver_ptr;
	unsigned version;

	asm mov bx, 0;			/* function 0: get version */
	asm call drv;
	asm mov version, ax;

	printf("\nVersion of Driver = %X.0%X",
		((version >> 8) & 0x00FF), (version & 0x00FF));
}

int Voc_Terminate_Driver(void)
{
	/* This function terminates the driver and deinstalls it from memory. */
	char far *drv = driver_ptr;

	asm mov bx, 9;			/* function 9: terminate */
	asm call drv;

	freemem(FP_SEG(driver_ptr));
	printf("\nDriver Terminated");
}

void Voc_Set_Port(unsigned int port)
{
	/* This function sets the I/O port of the Sound Blaster. */
	char far *drv = driver_ptr;

	asm mov bx, 1;			/* function 1: set port address */
	asm mov ax, port;
	asm call drv;
}

void Voc_Set_Speaker(unsigned int on)
{
	/* This function turns the speaker on and off. */
	char far *drv = driver_ptr;

	asm mov bx, 4;			/* function 4: speaker status */
	asm mov ax, on;
	asm call drv;
}

int Voc_Stop_Sound(void)
{
	/* This function stops a currently playing sound. */
	char far *drv = driver_ptr;

	asm mov bx, 8;			/* function 8: stop sound */
	asm call drv;
}

int Voc_Pause_Sound(void)
{
	/* This function pauses a sound that's playing. */
	char far *drv = driver_ptr;

	asm mov bx, 10;			/* function 10: pause sound */
	asm call drv;
}

int Voc_Continue_Sound(void)
{
	/* This function continues a sound that had been paused. */
	char far *drv = driver_ptr;

	asm mov bx, 11;			/* function 11: continue play */
	asm call drv;
}

int Voc_Break_Sound(void)
{
	/* This function breaks a sound that's in a loop. */
	char far *drv = driver_ptr;

	asm mov bx, 12;			/* function 12: break loop */
	asm call drv;
}

void Voc_Set_DMA(unsigned int dma)
{
	/* This function sets the DMA channel for the Sound Blaster. */
	char far *drv = driver_ptr;

	asm mov bx, 2;			/* function 2: set DMA int number */
	asm mov ax, dma;
	asm call drv;
}

void Voc_Unload_Sound(char far *sound_ptr)
{
	/* This function deletes the sound from memory. */
	freemem(FP_SEG(sound_ptr));
}
