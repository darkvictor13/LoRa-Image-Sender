#include <Arduino.h>
#include <LoRaMESH.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <array>
#include <queue>

#include "camera.hpp"
#include "image_part.hpp"
#include "message_types.hpp"
#include "lora_definitions.hpp"

/*
	Como esta configurado o Lora:
	Largura de banda: 2, 500 kHz
	Spreading Factor: 7 
	Coding rate: 1, 4/5

	Taxa de dados segundo a pagina 9 da documentacao = 21875 bits
*/

#define img_path "/img.jpg"

uint8_t image_id = 1;

std::vector<ImagePart> image_parts;

void printHexBuffer(const uint8_t* buffer, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        Serial.printf(" %02X", buffer[i]);
    }
    Serial.println();
}

//void separate(const camera_fb_t *frame) {
void separate(const uint8_t *buffer, size_t size) {
	const uint8_t last_part = ((uint8_t)ceil(size / (float)MAX_IMAGE_SIZE)) - 1;
	for (size_t i = 0; i < size; i+=MAX_IMAGE_SIZE) {
		ImagePart part;
		//memset(part.payload.byte_array, 0, sizeof(part.payload.byte_array));
		part.fields.type = IMAGE_JPEG;
		part.fields.id = image_id;
		part.fields.part = (i / MAX_IMAGE_SIZE) & 0xFF;
		part.fields.last_part = last_part;
		if (MAX_IMAGE_SIZE < size - i) {
			part.payload.size = APPLICATION_MAX_PAYLOAD_SIZE;
			memcpy(part.payload.byte_array + INDEX_BEGIN_IMAGE, buffer + i, MAX_IMAGE_SIZE);
		} else {
			part.payload.size = size - i + INDEX_BEGIN_IMAGE;
			memcpy(part.payload.byte_array + INDEX_BEGIN_IMAGE, buffer + i, size - i);
		}
		/*
		Serial.printf("Gerando frame %d de %d\n", part.fields.part, last_part);
		part.payload.size = std::min (
			static_cast<size_t>(APPLICATION_MAX_PAYLOAD_SIZE),
			(size - i) + INDEX_BEGIN_IMAGE
		);
		Serial.printf("Comecando a copiar o byte %02x e terminando com %02x\n", *(buffer + i), *(buffer + i + part.payload.size));
		memcpy(part.payload.byte_array + INDEX_BEGIN_IMAGE, buffer + i, part.payload.size);
		*/
		image_parts.push_back(part);
	}
	Serial.println("\n");
}

bool sendImagePart(ImagePart &part, const uint8_t id) {
	PrepareFrameCommand(
		id,
		CMD_SENDTRANSP,
		part.payload.byte_array,
		part.payload.size
	);
	return SendPacket() == MESH_OK;
}

void setup() {
	Camera camera;
	uint32_t localUniqueId;
	uint16_t local_id, receivedId, localNet;
	uint8_t received_command;

	uint8_t buffer[APPLICATION_MAX_PAYLOAD_SIZE];
	uint8_t buffer_size = 0;

	Serial.begin(115200);

	Serial.println("Iniciando LoRaMESH");
	SerialCommandsInit(9600);  //(rx_pin,tx_pin)
    if (LocalRead(&local_id, &localNet, &localUniqueId) != MESH_OK) {
        Serial.printf("Couldn't read local ID\n\n");
    } else {
        Serial.printf("Local ID: %hu\nLocal NET: %hu\n", local_id, localNet);
    }
    delay(2000);

/*
	uint8_t img_to_send[1024];
	for (uint16_t i = 0; i < sizeof(img_to_send); i++) {
		img_to_send[i] = i % 256;
	}
	separate(img_to_send, sizeof(img_to_send));
*/
/*
	Serial.println("Iniciando SPIFFS");
	if (!SPIFFS.begin()) {
		Serial.println("Falha ao iniciar o SPIFFS");
		return ;
	}

	auto file = SPIFFS.open(img_path);
	if (!file) {
		Serial.println("Falha ao abrir o arquivo");
		return;
	}
	const String img_to_send = file.readString();
	separate((const uint8_t *)img_to_send.c_str(), img_to_send.length());
*/
	Serial.println("Iniciando Camera");
	camera.init();
	auto picture = camera.takePicture();
	if (picture == NULL) {
		printf("Picture is NULL\n");
		return;
	} else {
		printf("Picture is not NULL\n");
	}

	separate(picture->buf, picture->len);
	Serial.printf("Total de partes: %d\n", image_parts.size());
	Serial.printf("Tamanho da imagem: %d\n", picture->len);
	Serial.println("Iniciando o envio");

	size_t i = 0;
	const size_t img_parts_size = image_parts.size();
	while (i < img_parts_size) {
		Serial.printf(
			"Enviando frame %03d de %03d bytes ",
			(int)image_parts[i].fields.part,
			(int)image_parts[i].payload.size
		);
		if (sendImagePart(image_parts[i], local_id)) {
			Serial.println("enviado");
		} else {
			Serial.println("não enviado");
			continue;
		}
		Serial1.flush();
		//delay(1000);
		if (
		ReceivePacketCommand(
			&receivedId,
			&received_command,
			buffer,
			&buffer_size,
			(TIME_TO_RECEIVE_MESSAGE * 3)
		) == MESH_ERROR) {
			Serial.println("Não recebeu nada");
			continue;
		}
		Serial.printf("Recebido: %d bytes\n", buffer_size);
		Serial.printf("Comando: %02x\n", received_command);
		Serial.print("Dados: ");

		printHexBuffer(buffer, buffer_size);
		if (buffer[0] == ACK && buffer[1] == image_parts[i].fields.part) {
			Serial.println("Frame recebido corretamente");
			i++;
		}else {
			Serial.println("Mensagem recebida nao possui ACK");
		}
	}
	Serial.println("Envio finalizado");
}

void loop() {
	;
}