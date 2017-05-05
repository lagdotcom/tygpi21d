/* D E F I N E S ///////////////////////////////////////////////////////// */

#define BUTTON_1_1			0x10	/* stick 1, button 1 */
#define BUTTON_1_2			0x20	/* stick 1, button 2 */
#define BUTTON_2_1			0x40	/* stick 2, button 1 */
#define BUTTON_2_2			0x80	/* stick 2, button 2 */

#define JOYSTICK_1			0x01
#define JOYSTICK_2			0x02

#define JOYSTICK_1_X		0x01	/* stick 1, x-axis */
#define JOYSTICK_1_Y		0x02	/* stick 1, y-axis */
#define JOYSTICK_2_X		0x04	/* stick 2, x-axis */
#define JOYSTICK_2_Y		0x08	/* stick 2, y-axis */

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

unsigned char Buttons(unsigned char button);
unsigned char Buttons_Bios(unsigned char button);
unsigned int Joystick(unsigned char stick);
int Joystick_Available(int stick_num);
unsigned int Joystick_Bios(unsigned char stick);
