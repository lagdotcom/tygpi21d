/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <time.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define LOG_FILE "DOSJUN.LOG"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Clear_Log(void)
{
	FILE *f = fopen(LOG_FILE, "w");
	if (f == NULL) {
		/* TODO */
		return;
	}

	fclose(f);
}

void Log(char *format, ...)
{
	time_t timer;
	struct tm *t;
	va_list vargs;
	FILE *f = fopen(LOG_FILE, "a");
	if (f == NULL) {
		/* TODO */
		return;
	}

	time(&timer);
	t = localtime(&timer);
	fprintf(f, "%02d:%02d:%02d ", t->tm_hour, t->tm_min, t->tm_sec);

	va_start(vargs, format);
	vfprintf(f, format, vargs);
	va_end(vargs);

	fputc('\n', f);
	fclose(f);
}
