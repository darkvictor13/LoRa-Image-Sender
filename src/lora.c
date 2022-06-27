#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// Linux headers
#include <errno.h>    // Error integer and strerror() function
#include <fcntl.h>    // Contains file controls like O_RDWR
#include <termios.h>  // Contains POSIX terminal control definitions
#include <unistd.h>   // write(), read(), close()

void ms_sleep(uint64_t ms) {
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

#define CRC_POLY  (0xA001)
#define CRC_BEGIN (0xC181)

void print_hex(uint8_t *data, uint32_t len) {
	for (uint8_t i = 0; i < len; i++) {
		printf("%02X", data[i]);
	}
	printf("\n");
}

// calcula crc 16 bits
uint16_t crc16(uint8_t* data_in, uint32_t length) {
    uint32_t i;
    uint8_t bitbang, j;
    uint16_t crc_calc = CRC_BEGIN;
    for (i = 0; i < length; i++) {
        crc_calc ^= ((uint16_t)data_in[i]) & 0x00FF;
        for (j = 0; j < 8; j++) {
            bitbang = crc_calc;
            crc_calc >>= 1;
            if (bitbang & 1) {
                crc_calc ^= CRC_POLY;
            }
        }
    }
	printf("Primeiro bit %02X\n", *((uint8_t*)&crc_calc));
	printf("Segundo bit %02X\n", ((uint8_t*)&crc_calc)[1]);
	printf("Bits mais significativos  %d\n", crc_calc / 256);
	printf("Bits menos significativos %d\n", crc_calc % 256);
	printf("CRC = %04X\n", crc_calc);
    return crc_calc;
}

// abre uma porta serial em /dev/ttyS0
int open_serial() {
	int fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0) {
		printf("Erro ao abrir a porta serial\n");
		exit(1);
	} else {
		printf("Porta serial aberta\n");
	}
	return fd;
}

// le n bytes da porta serial
bool read_serial(int fd, uint8_t *buffer, int size_buffer) {
	int it = 0;
	while (it < size_buffer) {
		int ret = read(fd, buffer + it, size_buffer - it);
		printf("foram lidos %d bytes\n", ret);
		if (ret > 0) {
			it += ret;
		} else {
			if (errno == EAGAIN) {
				ms_sleep(100);
			} else {
				printf("Erro ao ler da porta serial\n");
				memset(buffer, 0, size_buffer);
				return false;
			}
		}
	}
	return true;
}

bool write_serial(int fd, uint8_t *buffer, uint32_t size_buffer) {
	uint16_t crc = crc16(buffer, size_buffer);

	uint32_t size_message = size_buffer + sizeof(crc);
	uint8_t message[size_message];
	memcpy(message, buffer, size_buffer);
	memcpy(message + size_buffer, &crc, sizeof(crc));
	printf("Mensagem: ");
	print_hex(message, size_message);
	return write(fd, message, tam_mensagem) == size_message;
}

int main() {
	bool ret;
	int serial = open_serial();
	printf("FD = %d\n", serial);

	uint8_t mensagem_config[] = {0, 0, 0xE2, 0, 0, 0};
	uint8_t tam_mensagem_config = sizeof(mensagem_config);
	ret = write_serial(serial, mensagem_config, tam_mensagem_config);
	if (!ret) {
		printf("Erro ao enviar mensagem de configuração\n");
		exit(1);
	}

	const uint8_t tam_configs_serial = 31;
	uint8_t configs_serial[tam_configs_serial];
	ret = read_serial(serial, configs_serial, tam_configs_serial);
	if (!ret) {
		printf("Erro ao ler da porta serial\n");
		exit(1);
	}
	print_hex(configs_serial, tam_configs_serial);
	return 0;
}