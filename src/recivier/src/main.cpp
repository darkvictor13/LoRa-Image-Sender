#include <Arduino.h>
#include <LoRaMESH.h>
#include "message_types.hpp"

#define WINDOW_SIZE 10
#define TIME_TO_RECEIVE_MESSAGE 1000

uint8_t buffer[MAX_PAYLOAD_SIZE];
uint8_t buffer_size = 0;
uint16_t local_id, received_id, remoteId, gateway, localNet;
uint32_t localUniqueId;
uint8_t command, receivedCommand;

uint8_t response[2];

void setup() {
	memset(buffer, 0, sizeof(buffer));
	memset(response, 0, sizeof(response));
	Serial.begin(115200);
    SerialCommandsInit(9600);  //(rx_pin,tx_pin)
    if (LocalRead(&local_id, &localNet, &localUniqueId) != MESH_OK) {
        Serial.printf("Couldn't read local ID\n\n");
    } else {
        Serial.printf("Local ID: %hu\nLocal NET: %hu\n", local_id, localNet);
    }
    delay(2000);
}

void loop() {
    if (
	ReceivePacketCommand(
		&received_id,
		&receivedCommand,
		buffer,
		&buffer_size,
		TIME_TO_RECEIVE_MESSAGE
	) == MESH_ERROR) {
		Serial.println("Não recebeu nenhum pacote");
		return;
    }

    Serial.printf("Dados recebido: ");
    for (int i = 0; i < buffer_size; i++) {
        Serial.printf("%02X ", buffer[i]);
    }
    Serial.printf("\n");

	response[0] = ACK;
	response[1] = buffer[2];
	Serial.print("Resposta: ");
	PrepareFrameCommand(received_id, CMD_SENDTRANSP, response, sizeof(response));
	if (SendPacket() == MESH_OK) {
		Serial.println("enviado");
	} else {
		Serial.println("não enviado");
	}
}