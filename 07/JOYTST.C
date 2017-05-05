/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "graph3.h"
#include "graph7.h"

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct calib_typ {
	unsigned int min_x, min_y;
	unsigned int max_x, max_y;
	unsigned int cx, cy;
} calib, *calib_ptr;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Joystick_Calibrate(int stick, calib_ptr joy)
{
	/* This function calibrates the joystick by finding the minimum and
	maximum deflections in both the x- and y-axis, then stores it for future
	use. */
	unsigned int x_new, y_new;		/* temporary joystick positions */

	if (stick == JOY_1_CAL) {
		printf("\nCalibrating Joystick #1: ");
		printf("Swirl stick then release and press fire");

		/* Set calibrations to impossible values. */
		joy->max_x = 0;
		joy->max_y = 0;
		joy->min_x = 10000;
		joy->min_y = 10000;

		/* Now the user should swirl the joystick, let the stick fall
		neutral, and then press any button. */
		while (!Buttons(BUTTON_1_1 | BUTTON_1_2)) {
			/* Get the new values and try to update calibration. */
			x_new = Joystick(JOYSTICK_1_X);
			y_new = Joystick(JOYSTICK_1_Y);

			/* Process the x-axis. */
			if (x_new > joy->max_x) joy->max_x = x_new;
			if (x_new < joy->min_x) joy->min_x = x_new;

			/* Process the x-axis. */
			if (y_new > joy->max_y) joy->max_y = y_new;
			if (y_new < joy->min_y) joy->min_y = y_new;
		}

		/* The user has let the stick go to center. */
		joy->cx = x_new;
		joy->cy = y_new;
	} else if (stick == JOY_2_CAL) {
		printf("\nCalibrating Joystick #2: ");
		printf("Swirl stick then release and press fire");

		/* Set calibrations to impossible values. */
		joy->max_x = 0;
		joy->max_y = 0;
		joy->min_x = 10000;
		joy->min_y = 10000;

		/* Now the user should swirl the joystick, let the stick fall
		neutral, and then press any button. */
		while (!Buttons(BUTTON_2_1 | BUTTON_2_2)) {
			/* Get the new values and try to update calibration. */
			x_new = Joystick(JOYSTICK_2_X);
			y_new = Joystick(JOYSTICK_2_Y);

			/* Process the x-axis. */
			if (x_new > joy->max_x) joy->max_x = x_new;
			if (x_new < joy->min_x) joy->min_x = x_new;

			/* Process the x-axis. */
			if (y_new > joy->max_y) joy->max_y = y_new;
			if (y_new < joy->min_y) joy->min_y = y_new;
		}

		/* The user has let the stick go to center. */
		joy->cx = x_new;
		joy->cy = y_new;
	}
}

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	calib joy_1;
	char buffer[200];

	/* Test whether the joystick is plugged in. */
	if (!Joystick_Available(JOYSTICK_1)) {
		printf("\nThere is not a joystick plugged into port #1.");
		return;
	}

	/* Calibrate the joystick. */
	Joystick_Calibrate(JOY_1_CAL, &joy_1);

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	while (!kbhit()) {
		sprintf(buffer, "Joystick 1 = [%u,%u]    ",
			Joystick(JOYSTICK_1_X),
			Joystick(JOYSTICK_1_Y));
		Blit_String(20, 20, 15, buffer, 0);

		if (Buttons(BUTTON_1_1)) {
			Blit_String(20, 40, 15, "Button 1 pressed. ", 0);
		} else if (Buttons(BUTTON_1_2)) {
			Blit_String(20, 40, 15, "Button 2 pressed. ", 0);
		} else {
			Blit_String(20, 40, 15, "No button pressed.", 0);
		}
	}

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);

	/* Let the user know what the calibrations turned out to be. */
	printf("\nThe calibration data was:");
	printf("\nmax(%u, %u) min(%u, %u) center(%u, %u)",
		joy_1.max_x, joy_1.max_y,
		joy_1.min_x, joy_1.min_y,
		joy_1.cx, joy_1.cy);

	/* Later! */
}
