#include <Arduino.h>
#include <WebServer.h>
#include <vector>

#include "camera.hpp"
#include "image_part.hpp"
#include "message_types.hpp"
#include "lora_definitions.hpp"

/*
	Como esta configurado o Lora:
	Largura de banda: 2, 500 kHz
	Spreading Factor: 7 
	Coding rate: 1, 4/5

	Taxa de dados segundo a pagina 9 da documentação = 21875 bits
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

void separate(const uint8_t *buffer, size_t size) {
	const uint8_t last_part = ((uint8_t)ceil(size / (float)MAX_IMAGE_SIZE)) - 1;
	for (size_t i = 0; i < size; i+=MAX_IMAGE_SIZE) {
		ImagePart part;
		part.fields.type = MessageTypes::IMAGE_JPEG;
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
		image_parts.push_back(part);
	}
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
        Serial.println("Falha ao ler Parâmetros do LoRa");
    } else {
        Serial.println("Sucesso ao ler Parâmetros do LoRa");
        Serial.printf(
			"\tID: %hu\n\tNET: %hu\n\tUID: %u\n",
			local_id,
			localNet,
			localUniqueId
		);
    }
    delay(2000);

	Serial.println("Iniciando Camera");
	camera.init();
	auto picture = camera.takePicture();
	if (picture == NULL) {
		printf("Picture is NULL\n");
		return;
	} else {
		printf("Picture is not NULL\n");
	}
	Serial.println("Imagem:");
	printHexBuffer(picture->buf, picture->len);
	Serial.println();

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