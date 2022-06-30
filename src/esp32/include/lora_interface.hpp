#pragma once

#include <Arduino.h>

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
	CMD_SEND_TRANSP      = 0x28    /* Sends a packet to the device's transparent serial port */
};

enum BaudRate {
	BR9600,
	BR38400,
	BR57600,
	BR115200
};

enum OperationMode {
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

struct Config {
	uint16_t id;
	uint8_t potency;
	uint8_t bandwidth; // Bandwidth
	uint8_t spreading_factor; // SpreadFactor
	uint8_t coding_rate;

	uint8_t mask_config;
	uint8_t baud_rate; // BaudRate
	uint16_t periodic_test;
	uint8_t operation_mode; // OperationMode
	uint8_t window; // Window
	uint8_t transparent_mode;
};

uint16_t ComputeCRC(uint8_t* data_in, uint16_t length);

class LoraInterface {
private:
	HardwareSerial &serial;
	Config conf;
	uint32_t uid;
	uint8_t FW;
	uint8_t Canal;
	uint8_t VersaoFW;
	uint8_t BancoMemoria;
	//uint8_t TempMin,TempAtual,TempMax,VMin,VAtual,VMax,RuidoMin,RuidoMed,RuidoMax,RSSIIda,RSSIVolta,SNRIda,SNRVolta,IDGateway[2],IDRota[50]; // Diagnostico
	//uint8_t CRC[2];


	bool write(uint8_t* data, uint16_t length);
	bool read(uint8_t* data, uint16_t length, uint16_t timeout_ms);
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
	void setOperationMode(const uint8_t operation_mode);
	void setWindow(const Window window);

	/**
	 * @brief Escreve todas as configurações salvas
	 * na estrutura Config para o dispositivo.
	 * 
	 * @pre estrutura config com dados validos
	 * @post dispositivo configurado
	 */
	void writeConfig();

	bool readConfig();
	bool readRadioConfig();
	bool readParameterConfig();
	bool readDiagnosis();

	void printConfig();

	~LoraInterface();
};
