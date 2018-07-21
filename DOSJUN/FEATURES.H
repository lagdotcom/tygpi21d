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

#define CODE_BUFFER_SIZE		300
#define CODE_NAME_BUFFER_SIZE	50

#define COMBAT_BUFFER_SIZE		400

#define DOSJUN_BUFFER_SIZE		300
#define DOSJUN_REMOVAL_LIST_SIZE		100

#define IO_MAXFILES				20

#define JOBS_BUFFER_SIZE		100

#define LIST_START_CAPACITY		10

#define PARTY_PORTRAITS			50

#define STRINGS_BUFFER_SIZE		30

#endif
