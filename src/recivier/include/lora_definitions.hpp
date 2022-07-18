#pragma once

// Definicoes da taxa de transmissão
#define BITS_PER_SECOND 21875
static constexpr uint8_t BITS_PER_MS   = BITS_PER_SECOND / 1000;
static constexpr uint8_t MS_PER_PACKET = MAX_BUFFER_SIZE / BITS_PER_MS;
static constexpr uint8_t TIME_TO_RECEIVE_MESSAGE = MS_PER_PACKET * 2;