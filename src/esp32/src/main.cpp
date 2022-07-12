#include <Arduino.h>
#include "lora_interface.hpp"
#include "lora_joao.hpp"
#include "scoped_timer.hpp"

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
	SET_TIMER_DEFAULT;
	Serial.begin(115200);
	/*
	LoraInterface lora(Serial2);
	lora.setId(0x00CC);
	if (!lora.writeConfig()) {
		Serial.println("Erro ao escrever configurações");
		while (1);
	}
	lora.printConfig();
	*/
	LoRa lora;
	lora.LeituraConfiguracoesLoRa();
	lora.debugConfiguracoesLoRa();
	/*
	const auto ret = lora.writeRadioConfig();
	lora.printConfig();
	Serial.printf("Escrita de config: %s\n", ret ? "OK" : "ERRO");
	*/
}

void loop() {
}