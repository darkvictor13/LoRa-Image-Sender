#include <Arduino.h>
#include "lora_interface.hpp"

/*
	Codigo para testar LSB e MSB
	uint16_t valor = 0x1234;
	uint8_t *ptr = (uint8_t *)(&valor);
	for (int i = 0; i < 2; i++) {
		printf("%02X", ptr[i]);
	}
	printf("\n");

	34 fica no índice 0 e 12 no índice 1
	o id vai na ordem da documentação
*/

void setup() {
	LoraInterface lora;
	lora.printConfig();
}

void loop() {
}