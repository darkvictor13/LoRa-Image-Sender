#include <Arduino.h>
#include <LoRaMESH.h>
#include <array>
#include <queue>

#include "camera.hpp"
#include "image_part.hpp"
#include "message_types.hpp"

#define ACK_MAX_TIME 10000

uint8_t image_id = 1;

std::vector<ImagePart> image_parts;

void printHexBuffer(const uint8_t* buffer, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        Serial.printf(" %02X", buffer[i]);
    }
    Serial.println();
}

void separate(const camera_fb_t *frame) {
	const uint8_t total_parts = (uint8_t)ceil(frame->len / (float)MAX_IMAGE_SIZE);
	for (size_t i = 0; i < frame->len; i+=MAX_IMAGE_SIZE) {
		ImagePart part;
		//memset(part.payload.byte_array, 0, sizeof(part.payload.byte_array));
		part.fields.type = IMAGE_JPEG;
		part.fields.id = image_id;
		part.fields.part = (i / MAX_IMAGE_SIZE) & 0xFF;
		part.fields.total_parts = total_parts;
		Serial.printf("Gerando frame %d de %d\n", part.fields.part, total_parts);
		part.payload.size = std::min (
			static_cast<size_t>(MAX_IMAGE_SIZE),
			frame->len - i
		);
		memcpy(part.payload.byte_array + INDEX_BEGIN_IMAGE, frame->buf + i, part.payload.size);
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
	Serial.println("Iniciando\n");
	SerialCommandsInit(9600);  //(rx_pin,tx_pin)
    if (LocalRead(&local_id, &localNet, &localUniqueId) != MESH_OK) {
        Serial.printf("Couldn't read local ID\n\n");
    } else {
        Serial.printf("Local ID: %hu\nLocal NET: %hu\n", local_id, localNet);
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

	separate(picture);
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
		
		if (
		ReceivePacketCommand(
			&receivedId,
			&received_command,
			buffer,
			&buffer_size,
			ACK_MAX_TIME 
		) == MESH_ERROR) {
			Serial.println("Não recebeu nada");
			continue;
		}
		/*
		Serial.printf("Recebido: %d bytes\n", buffer_size);
		Serial.printf("Comando: %02x\n", received_command);
		Serial.print("Dados: ");
		printHexBuffer(buffer, buffer_size);
		*/
		if (buffer[0] == ACK && buffer[1] == image_parts[i].fields.part) {
			Serial.println("Frame recebido corretamente");
			i++;
		}else {
			Serial.println("Mensagem recebida nao possui ACK");
		}
	}
}

void loop() {
	;
}