#include "lora_interface.hpp"

uint16_t ComputeCRC(uint8_t* data_in, uint16_t length) {
    uint16_t i;
    uint8_t bitbang, j;
    uint16_t crc_calc;

    crc_calc = 0xC181;
    for (i = 0; i < length; i++) {
        crc_calc ^= (((uint16_t)data_in[i]) & 0x00FF);

        for (j = 0; j < 8; j++) {
            bitbang = crc_calc;
            crc_calc >>= 1;

            if (bitbang & 1) {
                crc_calc ^= 0xA001;
            }
        }
    }
	return crc_calc;
}

bool LoraInterface::write(uint8_t* data, uint16_t length) {
	uint8_t buffer[length + 2];
	const auto crc = ComputeCRC(data, length);
	memcpy(buffer, data, length);
	memcpy(buffer + length, &crc, 2);
	const auto ret = serial.write(buffer, length + 2) == length + 2;
	serial.flush();
	return ret;
}

bool LoraInterface::read(uint8_t* data, uint16_t length, uint16_t timeout_ms) {
    uint16_t i = 0;
    uint16_t waitNextByte = 500;
    while (((timeout_ms > 0) || (i > 0)) && (waitNextByte > 0)) {
        if (Serial2.available() > 0) {
            data[i++] = Serial2.read();
            waitNextByte = 500;
        }
        if (i > 0) {
            waitNextByte--;
        }
        timeout_ms--;
        delay(1);
    }
	if((timeout_ms == 0) && (i == 0)) return false;

	return true;
}

LoraInterface::LoraInterface(HardwareSerial &serial) : serial(serial) {
	memset(&conf, 0, sizeof(conf));
	const auto ret = readConfig();
	if (!ret) {
		printf("Erro ao ler configuração");
	}
}

LoraInterface::LoraInterface(const Config &config, HardwareSerial &serial)
	: serial(serial), conf(config) {

}

void LoraInterface::setId(const uint16_t id) {
	conf.id = id;
}

void LoraInterface::setPotency(const uint8_t potency) {
	conf.potency = potency;
}

void LoraInterface::setBandwidth(const Bandwidth bandwidth) {
	conf.bandwidth = bandwidth;
}

void LoraInterface::setSpreadingFactor(const SpreadFactor spreading_factor) {
	conf.spreading_factor = spreading_factor;
}

void LoraInterface::setBaudRate(const BaudRate baud_rate) {
	conf.baud_rate = baud_rate;
}

void LoraInterface::setCodingRate(const uint8_t coding_rate) {
	conf.coding_rate = coding_rate;
}

void LoraInterface::setMaskConfig(const uint8_t mask_config) {
	conf.mask_config = mask_config;
}

void LoraInterface::setBaudRate(const uint8_t baud_rate) {
	conf.baud_rate = baud_rate;
}

void LoraInterface::setPeriodicTest(const uint16_t periodic_test) {
	conf.periodic_test = periodic_test;
}

void LoraInterface::setOperationMode(const uint8_t operation_mode) {
	conf.operation_mode = operation_mode;
}

void LoraInterface::setWindow(const Window window) {
	conf.window = window;
}

void LoraInterface::writeConfig() {

}

bool LoraInterface::readConfig() {
	return
	readRadioConfig() ||
	readParameterConfig() ||
	readDiagnosis();
}

bool LoraInterface::readRadioConfig() {
	bool ret;
	uint8_t size_config = 31;
	uint8_t data[size_config];
	uint8_t data_in[] = {0, 0, CMD_LOCAL_READ, 0, 0, 0};
	ret = write(data_in, sizeof(data_in));
	if (!ret) {
		return false;
	}
	ret = read(data, sizeof(data), 5000);
	if (!ret) {
		return false;
	}

	memcpy(&conf.id, &data[0], 2);
	memcpy(&uid,     &data[5], 4);

	return true;
}

bool LoraInterface::readParameterConfig() {
	return true;
}

bool LoraInterface::readDiagnosis() {
	return true;
}

void LoraInterface::printConfig() {
	printf("id: %02x\n", conf.id);
	printf("uid: %04x\n", uid);
	printf("potency: %d\n", conf.potency);
	printf("bandwidth: %d\n", conf.bandwidth);
	printf("spreading_factor: %d\n", conf.spreading_factor);
	printf("baud_rate: %d\n", conf.baud_rate);
	printf("coding_rate: %d\n", conf.coding_rate);
	printf("mask_config: %d\n", conf.mask_config);
	printf("baud_rate: %d\n", conf.baud_rate);
	printf("periodic_test: %d\n", conf.periodic_test);
	printf("operation_mode: %d\n", conf.operation_mode);
	printf("window: %d\n", conf.window);
}

LoraInterface::~LoraInterface() {

}