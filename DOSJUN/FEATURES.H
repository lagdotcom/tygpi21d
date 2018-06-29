#ifndef DOSJUN_FEATURES_H
#define DOSJUN_FEATURES_H

/* These flags turn on subsystems */

#define FEATURE_FONT			1
#define FEATURE_TEXTURES		1
#define FEATURE_THINGS			1

/* These flags affect the behaviour of subsystems */

#define CODE_DEBUG				1
#define CODE_DEBUG_STACK		0

#define COMBAT_ALLOC_MSGBUF		0
#define COMBAT_RECLAIM_TEXTURES	0
#define COMBAT_DEBUG			1

#define LIST_REPORT				1

#define STRINGS_DEBUG			0

#define TIMER_TICK_FREQ			(0x427d / 5)

/* These values affect memory use */

#define COMBAT_BUFFER_SIZE		400

#define IO_MAXFILES				20

#define LIST_START_CAPACITY		10

#define STRINGS_BUFFER_SIZE		30

#endif
