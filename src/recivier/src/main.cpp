#include <Arduino.h>
#include <LoRaMESH.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <base64.h>
#include <uri/UriBraces.h>

#include "message_types.hpp"
#include "lora_definitions.hpp"
#include "image_part.hpp"

#define WINDOW_SIZE 1

#define img_path "/img.jpg"

uint16_t img_size = 0;
uint8_t img[0xFFFF];

uint8_t buffer[MAX_PAYLOAD_SIZE];
uint8_t buffer_size;
uint16_t local_id, received_id, remoteId, gateway, localNet;
uint32_t localUniqueId;
uint8_t command, receivedCommand;

uint8_t response[2 * WINDOW_SIZE];

WebServer server;

#define TCC_LEVI 0

#if TCC_LEVI
#include <HTTPClient.h>
static const char server_hostname[] = "http://192.168.1.110:3001/upload";
#endif

void printHexBuffer(const uint8_t* buffer, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        Serial.printf(" %02X", buffer[i]);
    }
    Serial.println();
}

void setupAp() {
	const char* ssid_ap = "redeDoEsp";
	const char* password_ap = "senhaSegura.";

#if TCC_LEVI
	const char* ssid_sta = "LabIoT";
	const char* password_sta = "labiot2020.";
#endif

	// IP Address details
	IPAddress local_ip(192, 168, 1, 1);
	IPAddress gateway(192, 168, 1, 1);
	IPAddress subnet(255, 255, 255, 0);

	// faz o esp crar um AP e conecta ao wifi
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(ssid_ap, password_ap);
	WiFi.softAPConfig(local_ip, gateway, subnet);
#if TCC_LEVI
	WiFi.begin(ssid_sta, password_sta);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("Falha ao conectar ao Wifi");
		ESP.restart();
	}else {
		Serial.println("Conectado ao Wifi");
	}
#endif
}

void serveImg() {
    auto file = SPIFFS.open(img_path);
    if (!file) {
		Serial.println("Falha ao abrir o arquivo");
		server.send(200, "text/plain", "Falha ao abrir o arquivo");
		return;
	}
	const String img = file.readString();
	Serial.printf("Servindo arquivo %s de %u bytes\n", img_path, file.size());
	file.close();
	server.send(200, "image/jpeg", img);
}

void requestImg() {
	const auto id = static_cast<uint16_t>(server.pathArg(0).toInt());
	if (!id) {
		server.send(200, "text/plain", "Id invalido");
	}
	uint8_t req = MessageTypes::TAKE_PICTURE;
	PrepareFrameCommand(id, CMD_SENDTRANSP, &req, sizeof(req));
	Serial.printf("Enviando pedido de imagem para %d: ", (int)id);
	if (SendPacket() == MESH_OK) {
		Serial.println("enviado");
	} else {
		Serial.println("não enviado");
	}
	server.send(200, "text/plain", String("Requisitando imagem para ") + id);
}

void taskHandleServer(void * pvParameters) {
	while (true) {
		server.handleClient();
		delay(2);
	}
}

void sendACK() {
	response[0] = ACK;
	response[1] = buffer[2];
	Serial.print("Resposta: ");
	PrepareFrameCommand(received_id, CMD_SENDTRANSP, response, 2);
	if (SendPacket() == MESH_OK) {
		Serial.println("enviado");
	} else {
		Serial.println("não enviado");
	}
}

void setup() {
	memset(buffer, 0, sizeof(buffer));
	memset(response, 0, sizeof(response));
	Serial.begin(115200);

	Serial.println("Iniciando SPIFFS");
	if (!SPIFFS.begin()) {
		Serial.println("Falha ao iniciar o SPIFFS");
		return ;
	}
	if (SPIFFS.exists(img_path)) {
		Serial.println("Arquivo existe");
	}else {
		Serial.println("Arquivo não existe");
	}
	setupAp();
    server.on("/lora_img", serveImg);
    server.on(UriBraces("/req_img/{}"), requestImg);
	server.begin();
	TaskHandle_t handle_server = NULL;
	xTaskCreate(taskHandleServer, "server", 20000, NULL, 1, &handle_server);

	Serial.println("Iniciando LoRaMESH");
    SerialCommandsInit(9600);  //(rx_pin,tx_pin)
    if (LocalRead(&local_id, &localNet, &localUniqueId) != MESH_OK) {
        Serial.printf("Couldn't read local ID\n\n");
    } else {
        Serial.printf("Local ID: %hu\nLocal NET: %hu\n", local_id, localNet);
    }
    delay(2000);
}

void loop() {
	buffer_size = 0;
    if (
	ReceivePacketCommand(
		&received_id,
		&receivedCommand,
		buffer,
		&buffer_size,
		TIME_TO_RECEIVE_MESSAGE
	) == MESH_ERROR) {
		Serial.println("Não recebeu nenhum pacote");
		return;
    }

	Serial.printf("Mensagem %d recebida de %hu\n",buffer[2], received_id);

	const auto size_to_write = buffer_size - INDEX_BEGIN_IMAGE;
	memcpy(img + img_size, buffer + INDEX_BEGIN_IMAGE, size_to_write);
	img_size += size_to_write;

    Serial.printf("Dados recebido: ");
    for (int i = 0; i < buffer_size; i++) {
        Serial.printf("%02X ", buffer[i]);
    }
    Serial.println();

	sendACK();

	if (buffer[2] == buffer[3]) {
		Serial.println("Imagem completa:");
		printHexBuffer(img, img_size);
		Serial.printf("Image size: %d, bytes:\n", img_size);

		auto file = SPIFFS.open(img_path, "w");
		if (!file) {
			Serial.println("Falha ao abrir o arquivo");
			return;
		}
		if (file.write(img, img_size) != img_size) {
			Serial.println("Falha ao escrever imagem");
			return;
		}
		#if TCC_LEVI
		HTTPClient http;
		if (http.begin(server_hostname)) {
			Serial.println("Iniciando requisição");
		}else {
			Serial.println("Falha ao iniciar requisição");
		}
		String json = "{\"image\": \"";
		http.addHeader("Content-Type", "application/json");
		json += base64::encode(img, img_size);
		json += "\"}";
		Serial.println("Json: " + json);
		Serial.println(http.POST(json));
		#endif

		file.close();
		img_size = 0;
	}
}