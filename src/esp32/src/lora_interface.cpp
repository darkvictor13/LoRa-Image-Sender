#include "lora_interface.hpp"
#include "scoped_timer.hpp"

static const auto BAUD_RATE = 9600;
static const auto READ_TIMEOUT = 1000;
static const auto SEND_TIMEOUT = 1000;

uint16_t ComputeCRC(const uint8_t* data_in, uint16_t length) {
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

bool LoraInterface::write(const uint8_t* data, uint16_t length) {
	const auto crc = ComputeCRC(data, length);
	const uint8_t crc_bytes[2] = {crc & 0xFF, (crc >> 8) & 0xFF};
	const auto ret_data = serial.write(data, length) == length;
	const auto ret_crc = serial.write(crc_bytes, sizeof(crc_bytes)) == sizeof(crc_bytes);

	serial.printf("CRC = %04x\n", crc);
	serial.print("Dados: ");
	for (auto i = 0; i < length; i++) {
		serial.printf("%02x ", data[i]);
	}
	serial.println();
	serial.flush();
	delay(1000);

/*
	serial.flush();
	delay(1000);
	serial.print("Dados escritos: ");
	for(auto i = 0; i < length; i++) {
		serial.printf("%02x ", data[i]);
	}
	serial.printf("%04x\n", crc);
*/
	return ret_data && ret_crc;
}

bool LoraInterface::write(const Packet &packet) {
	return write(packet.buffer, packet.size);
}

bool LoraInterface::read(uint8_t* data, uint16_t length, uint16_t timeout_ms) {
	/*
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
	serial.flush();
	delay(1000);
	if((timeout_ms == 0) && (i == 0)){
		serial.printf("Timeout de leitura\n");
		return false;
	}

	serial.printf("Dados lidos %04x ", ComputeCRC(data, i));
	for(int i = 0; i < length; i++) {
		serial.printf("%02x ", data[i]);
	}
	serial.println();

	return true;
	*/
    while (!serial.available()) {
        delay(10);
    }

    for (int i = 0; i < length; i++) {
		// read retorna -1 quando não há dados disponíveis
		// uartRead retorna 0 em caso de erro
        int a = serial.read();
        if (a == -1) {
            i--;
        } else {
            data[i] = a;
            delay(1);
        }
    }
	serial.flush();

/*
    delay(1000);
    serial.println();
    serial.print("Leitura: ");
    for (int i = 0; i < length; i++) {
        serial.print(" 0x");
        serial.print(data[i], HEX);
    }
    serial.println();
    serial.flush();
*/
    return true;
}

bool LoraInterface::read(Packet &packet, uint16_t timeout_ms) {
	return read(packet.buffer, packet.size, timeout_ms);
}

LoraInterface::LoraInterface(HardwareSerial &serial) : serial(serial) {
	SET_TIMER_DEFAULT;
	serial.begin(BAUD_RATE);
	memset(&conf, 0, sizeof(conf));
	const auto ret = readConfig();
	if (!ret) {
		serial.printf("Erro ao ler configuração");
	}
}

LoraInterface::LoraInterface(const Config &config, HardwareSerial &serial)
	: serial(serial), conf(config) {
	serial.begin(BAUD_RATE);
}

void LoraInterface::setId(const uint16_t id) {
	conf.id.value = id;
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
	conf.periodic_test.value = periodic_test;
}

void LoraInterface::setClass(const uint8_t lora_class) {
	conf.lora_class = lora_class;
}

void LoraInterface::setWindow(const Window window) {
	conf.window = window;
}

bool LoraInterface::writeRadioConfig() {
	const uint8_t size_data = 31;
	uint8_t data[size_data] = {0};
    uint8_t data_in[] = {
		conf.id.bytes[0],
        conf.id.bytes[1],
        CMD_WRITE_CONFIG,
        0,
		0,
        uid.bytes[0],
        uid.bytes[1],
        uid.bytes[2],
        uid.bytes[3],
        conf.mask_config,
        0,
        0,
        conf.baud_rate
	};

	if (!commandInterface(data_in, sizeof(data_in), data, size_data)) {
		return false;
	}

	uint16_t temp_id, temp_periodic_test;
	uint32_t temp_uid;
	memcpy(&temp_id,  &data[0], 2);
	memcpy(&temp_uid, &data[5], 4);
	memcpy(&temp_periodic_test, &data[20], 2);

	return
		temp_id == conf.id.value &&
		temp_uid == uid.value &&
		firmware_revision == data[11] &&
		channel_number == data[12] &&
		conf.baud_rate == data[16] &&
		memory == data[18] &&
		conf.periodic_test.value == temp_periodic_test &&
		conf.mask_config == data[27];
}

bool LoraInterface::writeParameterConfig() {
	const uint8_t size_data = 10;
	uint8_t data[size_data] = {0};
    uint8_t data_in[] = {
		conf.id.bytes[0],
		conf.id.bytes[1],
		CMD_LORA_PARAMETER,
		WRITE_PARAMETER,
		conf.potency,
		conf.bandwidth,
		conf.spreading_factor,
		conf.coding_rate
	};

	if (!commandInterface(data_in, sizeof(data_in), data, size_data)) {
		return false;
	}

	return
		conf.potency == data[4] &&
		conf.bandwidth == data[5] &&
		conf.spreading_factor == data[6] &&
		conf.coding_rate == data[7];
}

bool LoraInterface::writePeriodicTestConfig() {
	const uint8_t size_data = 8;
	uint8_t data[size_data] = {0};
    uint8_t data_in[] = {
		conf.id.bytes[0],
		conf.id.bytes[1],
		CMD_PERIODIC_TIME,
		WRITE_PERIODIC_TIME,
		conf.periodic_test.bytes[0],
		conf.periodic_test.bytes[1]
	};

	if (!commandInterface(data_in, sizeof(data_in), data, size_data)) {
		return false;
	}

	return
		conf.periodic_test.bytes[0] == data[4] &&
		conf.periodic_test.bytes[1] == data[5];
}

bool LoraInterface::writeOperationMode() {
	const uint8_t size_data = 9;
	uint8_t data[size_data] = {0};
    uint8_t data_in[] = {
		conf.id.bytes[0],
		conf.id.bytes[1],
		CMD_OPERATION_MODE,
		OPERATION_INTERFACE,
		conf.lora_class,
		conf.window
	};

	if (!commandInterface(data_in, sizeof(data_in), data, size_data)) {
		return false;
	}

	return
		conf.lora_class == data[5] &&
		conf.window == data[6];
}

bool LoraInterface::writeTransparentInterface() {
	const uint8_t size_data = 8;
	uint8_t data[size_data] = {0};
    uint8_t data_in[] = {
		conf.id.bytes[0],
		conf.id.bytes[1],
		CMD_OPERATION_MODE,
		TRANSPARENT_INTERFACE,
		conf.transparent_mode
	};

	if (!commandInterface(data_in, sizeof(data_in), data, size_data)) {
		return false;
	}

	return conf.transparent_mode == data[5];
}

bool LoraInterface::writeConfig() {
	return writeRadioConfig() &&
	writeParameterConfig() &&
	writeOperationMode() &&
	writeTransparentInterface();
}

bool LoraInterface::commandInterface(uint8_t *command, uint16_t length_command,
    uint8_t *response, uint16_t length_response) {
	if (!write(command, length_command)) {
		return false;
	}
	return read(response, length_response, READ_TIMEOUT);
}

bool LoraInterface::readRadioConfig() {
	const uint8_t size_data = 31;
	uint8_t data[size_data] = {0};
	uint8_t data_in[] = {0, 0, CMD_LOCAL_READ, 0, 0, 0};

	if (!commandInterface(data_in, sizeof(data_in), data, size_data)) {
		return false;
	}

	memcpy(&conf.id, &data[0], 2);
	memcpy(&uid,     &data[5], 4);

	firmware_revision = data[11];
	channel_number = data[12];
	conf.baud_rate = data[16];
	firmware_version = data[17];
	memory = data[18];
	memcpy(&conf.periodic_test, &data[20], 2);
	conf.mask_config = data[27];

	return true;
}

bool LoraInterface::readParameterConfig() {
	const uint8_t size_data = 10;
	uint8_t data[size_data] = {0};
	uint8_t data_in[] = {conf.id.bytes[0], conf.id.bytes[1], CMD_LORA_PARAMETER, READ_PARAMETER, 1, 0};

	if (!commandInterface(data_in, sizeof(data_in), data, size_data)) {
		return false;
	}

	conf.potency = data[4];
	conf.bandwidth = data[5];
	conf.spreading_factor = data[6];
	conf.coding_rate = data[7];

	return true;
}

bool LoraInterface::readPeriodicTestConfig() {
	const uint8_t size_data = 8;
	uint8_t data[size_data];
	const uint8_t msb = conf.id.value >> 8 & 0xFF;
	const uint8_t lsb = conf.id.value & 0xFF;
	uint8_t data_in[] = {lsb, msb, CMD_PERIODIC_TIME, READ_PERIODIC_TIME, 0, 0};

	if (!commandInterface(data_in, sizeof(data_in), data, size_data)) {
		return false;
	}

	memcpy(&conf.periodic_test, &data[4], 2);
	return true;
}

bool LoraInterface::readOperationMode() {
	const uint8_t size_data = 9;
	uint8_t data[size_data];
	uint8_t data_in[] = {conf.id.bytes[0], conf.id.bytes[1], CMD_OPERATION_MODE, OPERATION_INTERFACE, 255, 255};

	if (!commandInterface(data_in, sizeof(data_in), data, size_data)) {
		return false;
	}

	conf.lora_class = data[5];
	conf.window = data[6];

	return true;
}

bool LoraInterface::readConfig() {
	return readRadioConfig() &&
	readParameterConfig() &&
	readPeriodicTestConfig() &&
	readOperationMode();
}

void LoraInterface::printConfig() const {
	//serial.printf("%c%c%c", 0, 0, 10); // id e comando invalido
	serial.printf("id: %02X\n", conf.id.value);
	serial.printf("uid: %04X\n", uid.value);
	serial.printf("potency: %d\n", conf.potency);
	serial.printf("bandwidth: %d\n", conf.bandwidth);
	serial.printf("spreading_factor: %d\n", conf.spreading_factor);
	serial.printf("baud_rate: %d\n", conf.baud_rate);
	serial.printf("coding_rate: %d\n", conf.coding_rate);
	serial.printf("mask_config: %d\n", conf.mask_config);
	serial.printf("periodic_test: %d\n", conf.periodic_test.value);
	serial.printf("operation_mode: %d\n", conf.lora_class);
	serial.printf("window: %d\n", conf.window);
	serial.flush();
}

LoraInterface::~LoraInterface() {

}