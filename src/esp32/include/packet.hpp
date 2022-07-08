#ifndef PACKET
#define PACKET

#define MAX_BUFFER_SIZE 232

struct Packet {
	uint8_t buffer[MAX_BUFFER_SIZE];
	uint8_t size;
};

#endif // PACKET
