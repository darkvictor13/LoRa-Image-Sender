#ifndef DEBUG
#define DEBUG

#ifdef DEBUG_BUILD
	#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
	#define DEBUG_PRINTF(...)
#endif

#ifdef DEBUG_BUILD
	#define BEGIN_DEBUG Serial.begin(115200)
#else
	#define BEGIN_DEBUG
#endif

#endif // DEBUG
