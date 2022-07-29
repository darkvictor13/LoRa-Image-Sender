#include <Arduino.h>
#include <WebServer.h>
#include <vector>
#include <HTTPClient.h>
#include <base64.h>

#include "camera.hpp"
#include "image_part.hpp"
#include "message_types.hpp"
#include "lora_definitions.hpp"
#include "debug.h"

/*
	Como esta configurado o Lora:
	Largura de banda: 2, 500 kHz
	Spreading Factor: 7 
	Coding rate: 1, 4/5

	Taxa de dados segundo a pagina 9 da documentação = 21875 bits
*/

#define img_path "/img.jpg"

uint8_t image_id = 1;

// Variáveis do LoRa
uint16_t local_id, local_net;
uint8_t buffer[APPLICATION_MAX_PAYLOAD_SIZE];
uint8_t buffer_size = 0;
uint16_t received_id;
uint8_t received_command;

Camera camera;

void printHexBuffer(const uint8_t* buffer, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        DEBUG_PRINTF(" %02X", buffer[i]);
    }
    DEBUG_PRINTF("\n");
}

void separate(std::vector<ImagePart> &image_parts, const uint8_t *buffer, size_t size) {
	const uint8_t last_part = ((uint8_t)ceil(size / (float)MAX_IMAGE_SIZE)) - 1;
	image_parts.reserve(last_part);
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
		image_parts.emplace_back(part);
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

void sendStopWait(std::vector<ImagePart> &image_parts) {
	size_t i = 0;
	const size_t img_parts_size = image_parts.size();
	while (i < img_parts_size) {
		DEBUG_PRINTF("Enviando frame %03d\n", (int)image_parts[i].fields.part);
		if (sendImagePart(image_parts[i], local_id)) {
			DEBUG_PRINTF("enviado\n");
		} else {
			DEBUG_PRINTF("não enviado\n");
			continue;
		}
		Serial1.flush();
		DEBUG_PRINTF("Esperando ACK\n");
		if (ReceivePacketCommand(
			&received_id,
			&received_command,
			buffer,
			&buffer_size,
			(TIME_TO_RECEIVE_MESSAGE * 3)
		) == MESH_ERROR) {
			DEBUG_PRINTF("Não recebeu nada\n");
			continue;
		}

		if (buffer[0] == ACK && buffer[1] == image_parts[i].fields.part) {
			DEBUG_PRINTF("ACK recebido corretamente\n");
			i++;
		}else {
			DEBUG_PRINTF("Mensagem recebida nao possui ACK\n");
		}
	}
}

void waitToTakePicture() {
	DEBUG_PRINTF("Esperando pela solicitação de tirar foto\n");
	while (ReceivePacketCommand(
		&received_id,
		&received_command,
		buffer,
		&buffer_size,
		0xFFFFFFFF
	) == MESH_ERROR) {
		DEBUG_PRINTF("Não recebeu solicitação\n");
		continue;
	}
}

void setup() {
	uint32_t local_unique_id;
	BEGIN_DEBUG;

	DEBUG_PRINTF("Iniciando LoRaMESH\n");
	SerialCommandsInit(9600);  //(rx_pin,tx_pin)
    if (LocalRead(&local_id, &local_net, &local_unique_id) != MESH_OK) {
        DEBUG_PRINTF("Falha ao ler Parâmetros do LoRa\n");
    } else {
        DEBUG_PRINTF("Sucesso ao ler Parâmetros do LoRa\n");
        DEBUG_PRINTF(
			"\tID: %hu\n\tNET: %hu\n\tUID: %u\n",
			local_id,
			local_net,
			local_unique_id
		);
    }
    delay(2000);

	DEBUG_PRINTF("Iniciando Camera\n");
	camera.init();
}

void loop() {
	std::vector<ImagePart> image_parts;
	waitToTakePicture();
	DEBUG_PRINTF("Tirando foto numero %d:\n", image_id);
	auto picture = camera.takePicture();
	printHexBuffer(picture->buf, picture->len);

	separate(image_parts, picture->buf, picture->len);
	DEBUG_PRINTF("Total de partes: %d\n", image_parts.size());
	camera.freePicture(picture);
	DEBUG_PRINTF("Iniciando o envio\n");
	sendStopWait(image_parts);
	DEBUG_PRINTF("Terminando o envio\n");
	image_id++;
}
