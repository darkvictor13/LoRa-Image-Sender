#include <Arduino.h>
#include <LoRaMESH.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>

#include "message_types.hpp"
#include "lora_definitions.hpp"

#define WINDOW_SIZE 1

#define img_path "/img.jpg"

uint16_t img_size = 0;
uint8_t img[0xFFFF];

uint8_t buffer[MAX_PAYLOAD_SIZE];
uint8_t buffer_size;
uint16_t local_id, received_id, remoteId, gateway, localNet;
uint32_t localUniqueId;
uint8_t command, receivedCommand;

uint8_t response[2 * WINDOW_SIZE];

WebServer server;

void setupAp() {
	// SSID & Password
	const char* ssid = "redeDoEsp";
	const char* password = "senhaSegura.";

	// IP Address details
	IPAddress local_ip(192, 168, 1, 1);
	IPAddress gateway(192, 168, 1, 1);
	IPAddress subnet(255, 255, 255, 0);

	WiFi.softAP(ssid, password);
	WiFi.softAPConfig(local_ip, gateway, subnet);

	Serial.printf("Conecte-se a %s para ver a imagem\n", ssid);
}

void serveImg() {
    auto file = SPIFFS.open(img_path);
    if (!file) {
		Serial.println("Falha ao abrir o arquivo");
		server.send(200, "text/plain", "Falha ao abrir o arquivo");
		return;
	}
	const String img = file.readString();
	Serial.printf("Servindo arquivo %s de %u bytes\n", img_path, file.size());
	file.close();
	server.send(200, "image/jpeg", img);
}
void taskHandleServer(void * pvParameters) {
	while (true) {
		server.handleClient();
		delay(2);
	}
}

void setup() {
	memset(buffer, 0, sizeof(buffer));
	memset(response, 0, sizeof(response));
	Serial.begin(115200);

	Serial.println("Iniciando SPIFFS");
	if (!SPIFFS.begin()) {
		Serial.println("Falha ao iniciar o SPIFFS");
		return ;
	}
	if (SPIFFS.exists(img_path)) {
		Serial.println("Arquivo existe");
	}else {
		Serial.println("Arquivo não existe");
	}
	setupAp();
    server.on("/lora_img", serveImg);
	server.begin();
	TaskHandle_t handle_server = NULL;
	xTaskCreate(taskHandleServer, "server", 20000, NULL, 1, &handle_server);

	Serial.println("Iniciando LoRaMESH");
    SerialCommandsInit(9600);  //(rx_pin,tx_pin)
    if (LocalRead(&local_id, &localNet, &localUniqueId) != MESH_OK) {
        Serial.printf("Couldn't read local ID\n\n");
    } else {
        Serial.printf("Local ID: %hu\nLocal NET: %hu\n", local_id, localNet);
    }
    delay(2000);
}

void loop() {
	/*
	const auto qnt_disponivel = Serial1.available();
	if (qnt_disponivel > 0) {
		Serial1.read(buffer + buffer_index, qnt_disponivel);
		Serial.print("Chegaram os bytes:");
		for (int i = buffer_index; i < buffer_index + qnt_disponivel; i++) {
			Serial.printf(" %02X", buffer[i]);
		}
		Serial.println();
		buffer_index += qnt_disponivel;
		Serial.printf("Bytes lidos ate agr: %d\n", buffer_index);
		if (buffer_index >= MAX_BUFFER_SIZE) {
			Serial.println("Buffer cheio");
			buffer_index = 0;
		}
	}
	*/
	buffer_size = 0;
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

	Serial.printf("Mensagem %d recebida de %hu\n", buffer[2], received_id);
	memcpy(img + img_size, buffer, buffer_size);
	img_size += buffer_size;

    Serial.printf("Dados recebido: ");
    for (int i = 0; i < buffer_size; i++) {
        Serial.printf("%02X ", buffer[i]);
    }
    Serial.printf("\n");

	response[0] = ACK;
	response[1] = buffer[2];
	Serial.print("Resposta: ");
	PrepareFrameCommand(received_id, CMD_SENDTRANSP, response, 2);
	if (SendPacket() == MESH_OK) {
		Serial.println("enviado");
	} else {
		Serial.println("não enviado");
	}

	if (buffer[2] == buffer[3]) {
		Serial.println("Imagem completa");
		Serial.printf("Image size: %d, bytes:\n", img_size);
		for (int i = 0; i < img_size; i++) {
			Serial.printf("%02X ", img[i]);
		}
		Serial.println();

		auto file = SPIFFS.open(img_path, "w");
		if (!file) {
			Serial.println("Falha ao abrir o arquivo");
			return;
		}
		if (file.write(img, img_size) != img_size) {
			Serial.println("Falha ao escrever imagem");
			return;
		}
		file.close();
	}
}