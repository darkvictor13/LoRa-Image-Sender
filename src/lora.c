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

typedef enum {
  CMD_LORA_PARAMETER   = 0xD6,   /* Gets or Sets the LoRa modulation parameters */
  CMD_LOCAL_READ       = 0xE2,   /* Gets the ID, NET and UNIQUE ID info from the local device */
  CMD_REMOTE_READ      = 0xD4,   /* Gets the ID, NET and UNIQUE ID info from a remote device */
  CMD_WRITE_CONFIG     = 0xCA,   /* Writes configuration info to the device, i.e. NET and ID */
  CMD_GPIO_CONFIG      = 0xC2,   /* Configures a given GPIO pin to a desired mode, gets or sets its value */
  CMD_DIAGNOSIS        = 0xE7,   /* Gets diagnosis information from the device */
  CMD_READ_NOISE       = 0xD8,   /* Reads the noise floor on the current channel */
  CMD_READ_RSSI        = 0xD5,   /* Reads the RSSI between the device and a neighbour */
  CMD_TRACE_ROUTE      = 0xD2,   /* Traces the hops from the device to the master */
  CMD_SEND_TRANSP      = 0x28    /* Sends a packet to the device's transparent serial port */
} Cmd_Typedef;


void ms_sleep(uint64_t ms) {
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

#define CRC_POLY  (0xA001)
#define CRC_BEGIN (0xC181)

static const char *lora_port = "/dev/ttyS0";

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

// abre uma porta serial
int open_serial() {
	int fd = open(lora_port, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0) {
		printf("Erro ao abrir a porta serial\n");
		exit(1);
	} else {
		printf("Porta serial aberta\n");
	}
	// configura a porta serial
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0) {
		printf("Erro ao obter configurações da porta serial\n");
		exit(1);
	}
	cfsetospeed(&tty, B9600);
	cfsetispeed(&tty, B9600);
	// set 8 bits, no parity, no stop bits
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;
	
	// aplica as configurações
	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		printf("Erro ao aplicar configurações da porta serial\n");
		exit(1);
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
				printf("Erro ao ler da porta serial na funcao read\n");
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
	return write(fd, message, size_message) == size_message;
}

int main() {
	bool ret;
	int serial = open_serial();
	printf("FD = %d\n", serial);

	uint8_t mensagem_config[] = {0, 0, CMD_LOCAL_READ, 0, 0, 0};
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