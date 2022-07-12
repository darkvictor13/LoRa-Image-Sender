#include <Arduino.h>
#include "camera.hpp"
#include <LoRaMESH.h>

void setup() {
	Camera camera;
	uint16_t localId, receivedId, remoteId, gateway, localNet;
	uint32_t localUniqueId;
	uint8_t command, receivedCommand;
	Serial.begin(115200);
	Serial.println("Iniciando\n");
	SerialCommandsInit(9600);  //(rx_pin,tx_pin)
    if (LocalRead(&localId, &localNet, &localUniqueId) != MESH_OK) {
        Serial.printf("Couldn't read local ID\n\n");
    } else {
        Serial.printf("Local ID: %hu\nLocal NET: %hu\n", localId, localNet);
    }
    delay(2000);
	camera.init();
	auto picture = camera.takePicture();
	if (picture == NULL) {
		printf("Picture is NULL\n");
		return;
	} else {
		printf("Picture is not NULL\n");
	}
	Serial.printf("Tamanho do arquivo: %u\n", picture->len);
	Serial.printf("Largura da imagem: %u\n", picture->width);
	Serial.printf("Altura da imagem: %u\n", picture->height);
}

void loop() {
}