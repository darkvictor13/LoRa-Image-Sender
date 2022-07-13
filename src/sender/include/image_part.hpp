#pragma once

#include <Arduino.h>
#include <LoRaMESH.h>

/*
	Dados colocados no payload Quando deseja mandar uma imagem
	type | id | part | total_parts | image (n bytes)
	O pacote final deve ficar no formato:
	id_chip_lsb | id_chip_msb | type | id | part | total_parts | image (n bytes) | crc_lsb | crc_msb

	type: tipo do pacote (IMAGE_JPEG, ACK, NACK)
	id: o numero da mensagem (identificador)
	part: numero da parte da imagem (de 0 a total_parts)
	total_parts: numero total de partes da imagem
	image: bytes que compõem a imagem
*/
struct _payload {
	uint8_t byte_array[MAX_PAYLOAD_SIZE - 1];
	uint8_t size;
};

struct _fields {
	uint8_t type;
	uint8_t id;
	uint8_t part;
	uint8_t total_parts;
};

union ImagePart {
	_fields fields;
	_payload payload;
};

constexpr uint8_t max_image_size = MAX_PAYLOAD_SIZE - sizeof(_fields) - 1;
constexpr uint8_t index_begin_image = sizeof(_fields);