#pragma once

#include <Arduino.h>

#define CRC_POLY (0xA001)

#define LengthACK 0

#define MSB 0
#define LSB 1

#define CMD_Parametros      0xD6 // Leitura e escrita dos parametros de radio

#define CMD_LeituraLocal    0xE2 // Leitura dos parametros do modulo (apenas pela serial local)
#define CMD_LeituraRemota   0xD4 // Realizado apenas atraves do mestre da rede
#define CMD_ConfigRadio     0xCA // Escrita dos parametros de um radio (ID e Mascara bin  ́aria)

#define CMD_GPIO            0xC2 // Comando de configuracao, leitura e escrita de CMD_GPIO

#define CMD_Diagnostico     0xE7 // Adquire informacoes de operacao de um radio (local ou remoto)
#define CMD_Ruido           0xD8 // Leitura do nivel de ruido observado por um radio (local ou remoto)
#define CMD_RSSI            0xD5 // Retorna os niveis de potencia de sinal observados (ida e volta) no enlace entre dois radios
#define CMD_Rota            0xD2 // Retorna a rota utilizada para se comunicarcom um determinado r  ́adio
#define CMD_TestePeriodico  0x01 // Teste Periodico enviado dos escravos para o mestre. Configuravel pelo comando 0xCA
#define CMD_TempoPeriodico  0xCC // Configura ou le o tempo periodico no radiomestre
#define CMD_ModoOperacao    0xC1 // Configura ou le a classe do dispositivo e o comando da interface transparente

//Subcomandos GPIO

#define CMD_GPIOConf        0x02
#define CMD_GPIORead        0x00
#define CMD_GPIOWrite       0x01


//Subcomandos TempoPeriodico

#define CMD_LeituraTempoPeriodico   0x02
#define CMD_EscritaTempoPeriodico   0x01

//SubComandos ConfigClassnInterf

#define CMD_ConfigModoOperacao  0x00
#define CMD_ConfigInterfTransp  0x01


//SubComandosReadWriteParam

//  Escrita e Leitura
#define CMD_LeituraParametros   0x00
#define CMD_EscritaParametros   0x01



//      BW
#define _125kHz 0x00
#define _250kHz 0x01
#define _500kHz 0x02

//      SF
#define SF7   0x07
#define SF8   0x08
#define SF9   0x09
#define SF10  0x0A
#define SF11  0x0B
#define SF12  0x0C
#define SFK   0x00

//      CR
#define CR4_5 0x01
#define CR4_6 0x02
#define CR4_7 0x03
#define CR4_8 0x04

//      BaudRate

#define baud9600   0x00
#define baud38400  0x01
#define baud57600  0x02
#define baud115200 0x03



//Modo de Operacao
#define ClasseA 0x00
#define ClasseC 0x02
#define ERROModoOperacao 0x01

#define  Janela5s 0x00
#define Janela10s 0x01
#define Janela15s 0x02


class LoRa {
  private:
    uint8_t UID[4];
    uint8_t FW,Canal,VersaoFW,BancoMemoria; // ConfiguracaoLoRa
    uint8_t TempMin,TempAtual,TempMax,VMin,VAtual,VMax,RuidoMin,RuidoMed,RuidoMax,RSSIIda,RSSIVolta,SNRIda,SNRVolta,IDGateway[2],IDRota[50]; // Diagnostico
    uint8_t CRC[2];
  public:
    uint8_t ID[2];
    uint8_t POT,BW,SF,CR;  // ParametrosLoRa
    uint8_t MascaraConfig,BAUD,TestePeriodico[2],Classe,Janela,Transp; // ConfiguracaoLoRa

  private:
    void CalculaCRC(uint8_t* data_in, uint16_t length);

    void PrintLora(uint8_t* BytesLoRa, int Tamanho);

    bool ReadLoRa(uint8_t* MensagemRecebida, int Tamanho, uint8_t comando);

    void InterfaceLoRa(uint8_t* data_in, int LengthDI, uint8_t* MensagemRecebida, int LengthMR);

    // Funcoes de Leitura

    void LeituraConfiguracaoRadio();

    void LeituraParametrosRadio();

    void LeituraConfiguracaoTestePeriodico();

    void LeituraModoOperacao();

    void LeituraInterfaceTransparente();

    void LeituraRemota(uint8_t IDremoto[2]);

    // Funcoes de Escrita

    void EscritaParametrosRadio();

    void EscritaConfiguracaoRadio();

    void EscritaConfiguracaoTestePeriodico();

    void EscritaModoOperacao();

    void EscritaInterfaceTransparente();

    // Diagnosticos

    void LeituraDiagnostico();

    void LeituraRuido();

    void LeituraRSSI();

    void LeituraRota();

    void debugDadosLoRa();

  public:
	LoRa();
    void debugConfiguracoesLoRa();

    void LeituraConfiguracoesLoRa();

    void EscreveConfiguracoesLoRa();
	~LoRa();
};
