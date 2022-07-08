#pragma once

#include <Arduino.h>
#include "packet.hpp"

enum Cmd {
	CMD_LORA_PARAMETER   = 0xD6,   /* Gets or Sets the LoRa modulation parameters */
	CMD_LOCAL_READ       = 0xE2,   /* Gets the ID, NET and UNIQUE ID info from the local device */
	CMD_REMOTE_READ      = 0xD4,   /* Gets the ID, NET and UNIQUE ID info from a remote device */
	CMD_WRITE_CONFIG     = 0xCA,   /* Writes configuration info to the device, i.e. NET and ID */
	CMD_GPIO_CONFIG      = 0xC2,   /* Configures a given GPIO pin to a desired mode, gets or sets its value */
	CMD_DIAGNOSIS        = 0xE7,   /* Gets diagnosis information from the device */
	CMD_READ_NOISE       = 0xD8,   /* Reads the noise floor on the current channel */
	CMD_READ_RSSI        = 0xD5,   /* Reads the RSSI between the device and a neighbour */
	CMD_TRACEROUTE       = 0xD2,   /* Traces the hops from the device to the master */
	CMD_SEND_TRANSP      = 0x28,   /* Sends a packet to the device's transparent serial port */
	CMD_PERIODIC_TIME	 = 0xCC,   /* Gets or sets the periodic time */
	CMD_OPERATION_MODE   = 0xC1    /* Gets or sets the operation mode */
};

enum LoraParameter {
	READ_PARAMETER,
	WRITE_PARAMETER
};

enum PeriodicTime {
	WRITE_PERIODIC_TIME = 1,
	READ_PERIODIC_TIME
};

enum OperationMode {
	OPERATION_INTERFACE,
	TRANSPARENT_INTERFACE
};

enum BaudRate {
	BR9600,
	BR38400,
	BR57600,
	BR115200
};

enum LoraClass {
	CLASS_A,
	ERROR,
	CLASS_C
};

enum Bandwidth {
	BW_125,
	BW_250,
	BW_500
};

enum SpreadFactor {
	FSK = 0,
	SF7 = 7,
	SF8,
	SF9,
	SF10,
	SF11,
	SF12
};

enum Window {
	WINDOW_5S,
	WINDOW_10S,
	WINDOW_15S
};

union id_type {
	uint16_t value;
	uint8_t bytes[2];
};

union uid_type {
	uint32_t value;
	uint8_t bytes[4];
};

union periodic_test_time_type {
	uint16_t value;
	uint8_t bytes[2];
};

struct Config {
	id_type id;
	uint8_t potency;
	uint8_t bandwidth; // Bandwidth
	uint8_t spreading_factor; // SpreadFactor
	uint8_t coding_rate;

	uint8_t mask_config;
	uint8_t baud_rate; // BaudRate
	periodic_test_time_type periodic_test;
	uint8_t lora_class;
	uint8_t window; // Window
	uint8_t transparent_mode;
};

uint16_t ComputeCRC(const uint8_t* data_in, uint16_t length);

class LoraInterface {
private:
	HardwareSerial &serial;

	Config conf;
	uid_type uid; // Identificador único do dispositivo
	uint8_t firmware_revision;
	uint8_t channel_number;
	uint8_t firmware_version;
	uint8_t memory; /// em qual banco de memoria o firmware esta gravado
	//uint8_t TempMin,TempAtual,TempMax,VMin,VAtual,VMax,RuidoMin,RuidoMed,RuidoMax,RSSIIda,RSSIVolta,SNRIda,SNRVolta,IDGateway[2],IDRota[50]; // Diagnostico
	//uint8_t CRC[2];


	bool write(const uint8_t* data, uint16_t length);
	bool write(const Packet &packet);
	bool read(uint8_t* data, uint16_t length, uint16_t timeout_ms);
	bool read(Packet &packet, uint16_t timeout_ms);
public:
	LoraInterface(HardwareSerial &serial = Serial);
	LoraInterface(const Config &config, HardwareSerial &serial = Serial);

	void setId(const uint16_t id);
	void setPotency(const uint8_t potency);
	void setBandwidth(const Bandwidth bandwidth);
	void setSpreadingFactor(const SpreadFactor spreading_factor);
	void setBaudRate(const BaudRate baud_rate);
	void setCodingRate(const uint8_t coding_rate);
	void setMaskConfig(const uint8_t mask_config);
	void setBaudRate(const uint8_t baud_rate);
	void setPeriodicTest(const uint16_t periodic_test);
	void setClass(const uint8_t lora_class);
	void setWindow(const Window window);

	bool writeRadioConfig();
	bool writeParameterConfig();
	bool writePeriodicTestConfig();
	bool writeOperationMode();
	bool writeTransparentInterface();

	/**
	 * @brief Escreve todas as configurações salvas
	 * na estrutura Config para o dispositivo.
	 * 
	 * @return true Se todas as configurações foram salvas com sucesso.
	 * @return false Se alguma configuração não foi salva.
	 * @pre estrutura config com dados validos
	 * @post dispositivo configurado
	 */
	bool writeConfig();

	bool commandInterface(uint8_t *command, uint16_t length_command, uint8_t *response, uint16_t length_response);

	bool readRadioConfig();
	bool readParameterConfig();
	bool readPeriodicTestConfig();
	bool readOperationMode();
	bool readRemote(const uint16_t id);
	bool readConfig();

	void printConfig() const;

	bool writeData(const uint8_t* data, uint16_t length);
	bool readData(uint8_t* data, uint16_t length, uint16_t timeout_ms);

	~LoraInterface();
};
