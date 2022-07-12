#include <Arduino.h>
#include <LoRaMESH.h>

uint8_t bufferPayload[MAX_PAYLOAD_SIZE] = {0};
uint8_t receivedBuffer[MAX_PAYLOAD_SIZE] = {0};
uint8_t payloadSize = 0, receivedSize = 0, rssi_ida = 0, rssi_volta = 0;
uint16_t localId, receivedId, remoteId, gateway, localNet;
uint32_t localUniqueId;
uint8_t command, receivedCommand;

void setup() {
	Serial.begin(115200);
    SerialCommandsInit(9600);  //(rx_pin,tx_pin)
    if (LocalRead(&localId, &localNet, &localUniqueId) != MESH_OK) {
        Serial.printf("Couldn't read local ID\n\n");
    } else {
        Serial.printf("Local ID: %hu\nLocal NET: %hu\n", localId, localNet);
    }
    delay(2000);
}

void loop() {
    if (ReceivePacketCommand(
		&receivedId,
		&receivedCommand,
		receivedBuffer,
        &receivedSize, 6000) == MESH_OK) {
        Serial.printf("Dados recebido: ");
        for (int i = 0; i < receivedSize; i++) {
            Serial.printf("%02X ", receivedBuffer[i]);
        }
        Serial.printf("\n");
    }
}