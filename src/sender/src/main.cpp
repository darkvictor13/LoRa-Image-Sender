#include <Arduino.h>
#include <WebServer.h>
#include <vector>
#include <HTTPClient.h>
#include <base64.h>

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

// Variáveis do LoRa
uint16_t local_id, local_net;
uint8_t buffer[APPLICATION_MAX_PAYLOAD_SIZE];
uint8_t buffer_size = 0;
uint16_t received_id;
uint8_t received_command;

Camera camera;

void printHexBuffer(const uint8_t* buffer, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        Serial.printf(" %02X", buffer[i]);
    }
    Serial.println();
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
		Serial.printf("Enviando frame %03d\n", (int)image_parts[i].fields.part);
		if (sendImagePart(image_parts[i], local_id)) {
			Serial.println("enviado");
		} else {
			Serial.println("não enviado");
			continue;
		}
		Serial1.flush();
		Serial.println("Esperando ACK");
		if (ReceivePacketCommand(
			&received_id,
			&received_command,
			buffer,
			&buffer_size,
			(TIME_TO_RECEIVE_MESSAGE * 3)
		) == MESH_ERROR) {
			Serial.println("Não recebeu nada");
			continue;
		}

		if (buffer[0] == ACK && buffer[1] == image_parts[i].fields.part) {
			Serial.println("ACK recebido corretamente");
			i++;
		}else {
			Serial.println("Mensagem recebida nao possui ACK");
		}
	}
}

void waitToTakePicture() {
	Serial.println("Esperando pela solicitação de tirar foto");
	while (ReceivePacketCommand(
		&received_id,
		&received_command,
		buffer,
		&buffer_size,
		0xFFFFFFFF
	) == MESH_ERROR) {
		Serial.println("Não recebeu solicitação");
		continue;
	}
}

void setup() {
	uint32_t local_unique_id;
	Serial.begin(115200);

/*
	Serial.println("Iniciando LoRaMESH");
	SerialCommandsInit(9600);  //(rx_pin,tx_pin)
    if (LocalRead(&local_id, &local_net, &local_unique_id) != MESH_OK) {
        Serial.println("Falha ao ler Parâmetros do LoRa");
    } else {
        Serial.println("Sucesso ao ler Parâmetros do LoRa");
        Serial.printf(
			"\tID: %hu\n\tNET: %hu\n\tUID: %u\n",
			local_id,
			local_net,
			local_unique_id
		);
    }
    delay(2000);
	*/

	Serial.println("Iniciando Camera");
	camera.init();
}

void loop() {
	std::vector<ImagePart> image_parts;
	waitToTakePicture();
	Serial.printf("Tirando foto numero %d:\n", image_id);
	auto picture = camera.takePicture();
	printHexBuffer(picture->buf, picture->len);

	separate(image_parts, picture->buf, picture->len);
	Serial.printf("Total de partes: %d\n", image_parts.size());
	camera.freePicture(picture);
	Serial.println("Iniciando o envio");
	sendStopWait(image_parts);
	Serial.println("Terminando o envio");
	image_id++;
}