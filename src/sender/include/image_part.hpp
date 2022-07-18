/*
	Testes:
	Para enviar um frame e receber um ACK demora 2 segundos
*/
#pragma once

#include <Arduino.h>
#include <LoRaMESH.h>

constexpr uint8_t APPLICATION_MAX_PAYLOAD_SIZE = MAX_PAYLOAD_SIZE - 1;

/*
	Dados colocados no payload Quando deseja mandar uma imagem
	type | id | part | last_part | image (n bytes)
	O pacote final deve ficar no formato:
	id_chip_lsb | id_chip_msb | type | id | part | last_part | image (n bytes) | crc_lsb | crc_msb

	type: tipo do pacote (IMAGE_JPEG, ACK, NACK)
	id: o numero da mensagem (identificador)
	part: numero da parte da imagem (de 0 a last_part)
	last_part: numero total de partes da imagem
	image: bytes que compõem a imagem
*/
struct _payload {
	uint8_t byte_array[APPLICATION_MAX_PAYLOAD_SIZE];
	uint8_t size;
};

struct _fields {
	uint8_t type;
	uint8_t id;
	uint8_t part;
	uint8_t last_part;
	uint8_t message_size;
};

union ImagePart {
	_fields fields;
	_payload payload;
};

constexpr uint8_t MAX_IMAGE_SIZE = APPLICATION_MAX_PAYLOAD_SIZE - sizeof(_fields);
constexpr uint8_t INDEX_BEGIN_IMAGE = sizeof(_fields);