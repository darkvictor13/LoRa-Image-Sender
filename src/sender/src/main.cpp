#include <Arduino.h>
#include <LoRaMESH.h>
#include <array>
#include <queue>

#include "camera.hpp"
#include "image_part.hpp"
#include "message_types.hpp"

uint8_t image_id = 1;
const uint8_t tam_buffer = MAX_PAYLOAD_SIZE - 1;
uint8_t buffer[tam_buffer];

std::vector<ImagePart> image_parts;

void separate(const camera_fb_t *frame) {
	const uint8_t total_parts = (uint8_t)ceil(frame->len / (float)max_image_size);
	for (size_t i = 0; i < frame->len; i+=max_image_size) {
		ImagePart part;
		part.fields.type = IMAGE_JPEG;
		part.fields.id = image_id;
		part.fields.part = i / max_image_size;
		part.fields.total_parts = total_parts;
		part.payload.size = std::min (
			static_cast<size_t>(max_image_size),
			frame->len - i
		);
		memcpy(part.payload.byte_array, frame->buf + i, part.payload.size);
		image_parts.push_back(part);
	}
}

uint16_t localId, receivedId, remoteId, gateway, localNet;

void setup() {
	Camera camera;
	uint32_t localUniqueId;

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

	// a partir da picture cria um vetor partes de imagem
	//for (size_t i = 0; i < picture->len; i += tam_buffer) {

	//}
	ImagePart image;
	image.fields.type = IMAGE_JPEG;
	image.fields.id = image_id;
	image.fields.part = 1;
	image.fields.total_parts = 10;
	//memset(&image.payload.byte_array[index_begin_image], 0xCC, max_image_size);
	memcpy(image.payload.byte_array + index_begin_image, picture->buf, max_image_size);
	image.payload.size = MAX_PAYLOAD_SIZE - 1;

	Serial.print("Dados da Imagem: ");
	for (uint8_t i = 0; i < image.payload.size; i++) {
		Serial.printf(" %02x", image.payload.byte_array[i]);
	}
	Serial.println();
	Serial.println("Enviando imagem");
	PrepareFrameCommand(localId, CMD_SENDTRANSP, image.payload.byte_array, image.payload.size);
	if (SendPacket() == MESH_OK) {
		Serial.println("Imagem enviada");
	} else {
		Serial.println("Imagem não enviada");
	}
	delay(10000);
}

void loop() {
	for (int i = 0; i < tam_buffer; i++) {
		buffer[i] = i;
	}
	PrepareFrameCommand(localId, CMD_SENDTRANSP, buffer, tam_buffer);
	if (SendPacket() == MESH_OK) {
		Serial.println("Packet sent\n");
	} else {
		Serial.println("Packet not sent\n");
	}
	delay(5000);
}