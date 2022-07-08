#include "lora_joao.hpp"

#define SerialLoRa Serial

void LoRa::CalculaCRC(uint8_t* data_in, uint16_t length) {
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
    crc_calc = crc_calc & 0xFFFF;
    CRC[MSB] = uint8_t((crc_calc & 0xFF00) >> 8);
    CRC[LSB] = uint8_t(crc_calc & 0x00FF);
}

void LoRa::PrintLora(uint8_t* BytesLoRa, int Tamanho) {
    CalculaCRC(BytesLoRa, Tamanho);
    BytesLoRa[Tamanho] = CRC[LSB];
    BytesLoRa[Tamanho + 1] = CRC[MSB];

    int i;
    for (i = 0; i < Tamanho + 2; i++) {
        SerialLoRa.write(BytesLoRa[i]);  // 02- Codigo
    }
    // SerialLoRa.write(CRC[LSB]);       // CRC (LSB)
    // SerialLoRa.write(CRC[MSB]);       // CRC (MSB)
    delay(1000);
    Serial.println();
    Serial.print("Escrita: ");
    for (int i = 0; i < Tamanho + 2; i++) {
        Serial.print(" 0x");
        Serial.print(BytesLoRa[i], HEX);
    }
    Serial.println();
}

bool LoRa::ReadLoRa(uint8_t* MensagemRecebida, int Tamanho, uint8_t comando) {
    int erro = 0;

    while (SerialLoRa.available() == 0) {
        delay(10);
    }

    for (int i = 0; i < Tamanho; i++) {
        int a = SerialLoRa.read();
        if (a == -1) {
            i--;
        } else {
            MensagemRecebida[i] = a;
            delay(1);
        }
    }

    delay(1000);
    Serial.println();
    Serial.print("Leitura: ");
    for (int i = 0; i < Tamanho; i++) {
        Serial.print(" 0x");
        Serial.print(MensagemRecebida[i], HEX);
    }
    Serial.println();

    SerialLoRa.flush();
    if (MensagemRecebida[2] == comando) {
        return false;
    }
    return true;
}

void LoRa::InterfaceLoRa(uint8_t* data_in, int LengthDI, uint8_t* MensagemRecebida, int LengthMR) {
    bool erro = true;
    bool erroCRC = true;

    while (erroCRC && erro) {

        // Envia comando para o LoRa
        PrintLora(data_in, LengthDI);

        // Le Resposta do LoRa
        erro = ReadLoRa(MensagemRecebida, LengthMR, data_in[2]);

        CalculaCRC(MensagemRecebida, LengthMR);

        // Salva Variaveis LoRa
        if ((CRC[LSB] == MensagemRecebida[LengthMR - 2]) &&
            (CRC[MSB] = MensagemRecebida[LengthMR - 1])) {
            erroCRC = true;
        }
        delay(100);
    }
}

void LoRa::LeituraConfiguracaoRadio() {
    uint8_t data_in[] = {0x00, 0x00, CMD_LeituraLocal, 0x00, 0x00, 0x00};

    int LengthMR = 31;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    ID[LSB] = MensagemRecebida[0];
    ID[MSB] = MensagemRecebida[1];
    UID[0] = MensagemRecebida[5];                // 05- UID (MSB)
    UID[1] = MensagemRecebida[6];                // 06- UID
    UID[2] = MensagemRecebida[7];                // 07- UID
    UID[3] = MensagemRecebida[8];                // 08- UID (LSB)
    FW = MensagemRecebida[11];                   // 11- Revisao de FW
    Canal = MensagemRecebida[12];                // 12- Canal
    BAUD = MensagemRecebida[16];                 // 16- BAUDRATE Serial
    VersaoFW = MensagemRecebida[17];             // 17- Versao de FW
    BancoMemoria = MensagemRecebida[18];         // 18- Banco de memoria
    TestePeriodico[MSB] = MensagemRecebida[20];  // 20- Teste Periodico (MSB)
    TestePeriodico[LSB] = MensagemRecebida[21];  // 21- Teste Periodico (LSB)
    MascaraConfig = MensagemRecebida[27];        // 27- Mascara de configuracao
}

void LoRa::LeituraParametrosRadio() {
    uint8_t data_in[] = {
        ID[LSB], ID[MSB], CMD_Parametros, CMD_LeituraParametros, 0x01, 0x00};

    int LengthMR = 10;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    POT = MensagemRecebida[4];
    BW = MensagemRecebida[5];
    SF = MensagemRecebida[6];
    CR = MensagemRecebida[7];
}

void LoRa::LeituraConfiguracaoTestePeriodico() {
    uint8_t data_in[] = {
        ID[LSB], ID[MSB], CMD_TempoPeriodico, CMD_LeituraTempoPeriodico,
        0x00,    0x00};

    int LengthMR = 8;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    TestePeriodico[LSB] = MensagemRecebida[4];  // 04- ID do Gateway (LSB)
    TestePeriodico[MSB] = MensagemRecebida[4];  // 05- ID do Gateway (MSB)
}

void LoRa::LeituraModoOperacao() {
    uint8_t data_in[] = {
        ID[LSB], ID[MSB], CMD_ModoOperacao, CMD_ConfigModoOperacao, 0xFF, 0xFF};

    int LengthMR = 9;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    Classe = MensagemRecebida[5];  // 04- ID do Gateway (LSB)
    Janela = MensagemRecebida[6];  // 05- ID do Gateway (MSB)
}

void LoRa::LeituraInterfaceTransparente() {
    uint8_t data_in[] = {ID[LSB], ID[MSB], CMD_ModoOperacao,
                         CMD_ConfigInterfTransp, 0xFF};

    int LengthMR = 8;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    Transp = MensagemRecebida[5];  // 05 - Transp
}

void LoRa::LeituraRemota(uint8_t IDremoto[2]) {
    uint8_t data_in[] = {IDremoto[LSB], IDremoto[MSB], CMD_LeituraRemota,
                         0x00,          0x00,          0x00};

    int LengthMR = 31;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    UID[0] = MensagemRecebida[5];                // 05- UID (MSB)
    UID[1] = MensagemRecebida[6];                // 06- UID
    UID[2] = MensagemRecebida[7];                // 07- UID
    UID[3] = MensagemRecebida[8];                // 08- UID (LSB)
    FW = MensagemRecebida[11];                   // 11- Revisao de FW
    Canal = MensagemRecebida[12];                // 12- Canal
    BAUD = MensagemRecebida[16];                 // 16- BAUDRATE Serial
    VersaoFW = MensagemRecebida[17];             // 17- Versao de FW
    BancoMemoria = MensagemRecebida[18];         // 18- Banco de memoria
    TestePeriodico[MSB] = MensagemRecebida[20];  // 20- Teste Periodico (MSB)
    TestePeriodico[LSB] = MensagemRecebida[21];  // 21- Teste Periodico (LSB)
    MascaraConfig = MensagemRecebida[27];        // 27- Mascara de configuracao
}

void LoRa::EscritaParametrosRadio() {
    uint8_t data_in[] = {
        ID[LSB], ID[MSB], CMD_Parametros, CMD_EscritaParametros, POT, BW,
        SF,      CR};

    int LengthMR = 10;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    POT = MensagemRecebida[4];
    BW = MensagemRecebida[5];
    SF = MensagemRecebida[6];
    CR = MensagemRecebida[7];
}

void LoRa::EscritaConfiguracaoRadio() {
    uint8_t data_in[] = {
        ID[LSB], ID[MSB], CMD_ConfigRadio, 0x00, 0x00, UID[0], UID[1],
        UID[2],  UID[3],  MascaraConfig,   0x00, 0x00, BAUD};

    int LengthMR = 31;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    UID[0] = MensagemRecebida[5];                // 05- UID (MSB)
    UID[1] = MensagemRecebida[6];                // 06- UID
    UID[2] = MensagemRecebida[7];                // 07- UID
    UID[3] = MensagemRecebida[8];                // 08- UID (LSB)
    FW = MensagemRecebida[11];                   // 11- Revisao de FW
    Canal = MensagemRecebida[12];                // 12- Canal
    BAUD = MensagemRecebida[16];                 // 16- BAUDRATE Serial
    VersaoFW = MensagemRecebida[17];             // 17- Versao de FW
    BancoMemoria = MensagemRecebida[18];         // 18- Banco de memoria
    TestePeriodico[MSB] = MensagemRecebida[20];  // 20- Teste Periodico (MSB)
    TestePeriodico[LSB] = MensagemRecebida[21];  // 21- Teste Periodico (LSB)
    MascaraConfig = MensagemRecebida[27];        // 27- Mascara de configuracao
}

void LoRa::EscritaConfiguracaoTestePeriodico() {
    uint8_t data_in[] = {ID[LSB],
                         ID[MSB],
                         CMD_TempoPeriodico,
                         CMD_EscritaTempoPeriodico,
                         TestePeriodico[LSB],
                         TestePeriodico[MSB]};

    int LengthMR = 8;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    TestePeriodico[LSB] = MensagemRecebida[4];  // 04- ID do Gateway (LSB)
    TestePeriodico[MSB] = MensagemRecebida[4];  // 05- ID do Gateway (MSB)
}

void LoRa::EscritaModoOperacao() {
    uint8_t data_in[] = {ID[LSB],          ID[MSB],
                         CMD_ModoOperacao, CMD_ConfigModoOperacao,
                         Classe,           Janela};

    int LengthMR = 9;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    Classe = MensagemRecebida[5];  // 04- ID do Gateway (LSB)
    Janela = MensagemRecebida[6];  // 05- ID do Gateway (MSB)
}

void LoRa::EscritaInterfaceTransparente() {
    uint8_t data_in[] = {ID[LSB], ID[MSB], CMD_ModoOperacao,
                         CMD_ConfigInterfTransp, Transp};

    int LengthMR = 8;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    Transp = MensagemRecebida[5];  // 05 - Transp
}

void LoRa::LeituraDiagnostico() {
    uint8_t data_in[] = {ID[LSB], ID[MSB], CMD_Diagnostico, 0x00, 0x00, 0x00};

    int LengthMR = 29;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    TempMin = MensagemRecebida[3];    // 03- Temperatura minima
    TempAtual = MensagemRecebida[4];  // 04- Temperatura Atual
    TempMax = MensagemRecebida[5];    // 05- Temperatura Maxima
    VMin = MensagemRecebida[6];       // 06- Tensao minima
    VAtual = MensagemRecebida[7];     // 07- Tensao Atual
    VMax = MensagemRecebida[8];       // 08- Tensao Maxima
}

void LoRa::LeituraRuido() {
    uint8_t data_in[] = {ID[LSB], ID[MSB], CMD_Ruido, 0x00, 0x00, 0x00};

    int LengthMR = 8;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    RuidoMin = MensagemRecebida[3];  // 03- Ruido Minimo
    RuidoMed = MensagemRecebida[4];  // 04- Ruido Medio
    RuidoMax = MensagemRecebida[5];  // 05- Ruido Maximo
}

void LoRa::LeituraRSSI() {
    uint8_t data_in[] = {ID[LSB], ID[MSB], CMD_RSSI, 0x00, 0x00, 0x00};

    int LengthMR = 13;
    uint8_t MensagemRecebida[LengthMR];

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    IDGateway[LSB] = MensagemRecebida[3];  // 03- ID do Gateway (LSB)
    IDGateway[MSB] = MensagemRecebida[4];  // 04- ID do Gateway (MSB)
    RSSIIda = MensagemRecebida[5];         // 05- RSSI IDA
    RSSIVolta = MensagemRecebida[6];       // 06- RSSI VOLTA
    SNRIda = MensagemRecebida[7];          // 07- SNR IDA
    SNRVolta = MensagemRecebida[8];        // 08- SNR VOLTA
}

void LoRa::LeituraRota() {
    uint8_t data_in[] = {ID[LSB], ID[MSB], CMD_Rota, 0x00, 0x00, 0x00};

    int LengthMR = 100;
    uint8_t MensagemRecebida[LengthMR];

    for (int i = 0; i < LengthMR; i++) {
        MensagemRecebida[i] = 0;
    }

    InterfaceLoRa(data_in, sizeof(data_in), MensagemRecebida, LengthMR);

    for (int i = 0; MensagemRecebida[i] == 0; i++) {
        IDRota[i] = (uint16_t(MensagemRecebida[i * 2 + 3]) &
                     0x00ff);  // 03- ID do Gateway (LSB)
        IDRota[i] += (uint16_t(MensagemRecebida[i * 2 + 4]) & 0x00ff)
                     << 8;  // 04- ID do Gateway (MSB)
    }
}

void LoRa::debugDadosLoRa() {
    Serial.println();
    Serial.println("========================================================");
    Serial.println();
    Serial.println("ID:              \t0x" + String(ID[MSB], HEX) + " " +
                   String(ID[LSB], HEX));
    Serial.println("UID:             \t0x" + String(UID[0], HEX) + " " +
                   String(UID[1], HEX) + " " + String(UID[2], HEX) + " " +
                   String(UID[3], HEX));
    Serial.println();  // Parametros
    Serial.println("Potencia:        \t0x" + String(POT, HEX));
    Serial.println("BandWidth:       \t0x" + String(BW, HEX));
    Serial.println("SpreadingFactor: \t0x" + String(SF, HEX));
    Serial.println("Coding Rate:     \t0x" + String(CR, HEX));
    Serial.println();  // Configuracao
    Serial.println("Revisao FW:      \t0x" + String(FW, HEX));
    Serial.println("Canal:           \t0x" + String(Canal, HEX));
    Serial.println("BAUD:            \t0x" + String(BAUD, HEX));
    Serial.println("Versao FW:       \t0x" + String(VersaoFW, HEX));
    Serial.println("Banco de Memoria:\t0x" + String(BancoMemoria, HEX));
    Serial.println("TestePeriodico:  \t0x" + String(TestePeriodico[0], HEX) +
                   " " + String(TestePeriodico[1], HEX));
    Serial.println("MascaraConfig:   \t0x" + String(MascaraConfig, HEX));
    Serial.println();  // Modo de Operacao
    Serial.println("Classe:          \t0x" + String(Classe, HEX));
    Serial.println("Janela:          \t0x" + String(Janela, HEX));
    Serial.println("Transp:          \t0x" + String(Transp, HEX));
    Serial.println();
    Serial.println();  // Diagnostico
    Serial.println();
    Serial.println("TempMin:\t\t0x" + String(TempMin, HEX));
    Serial.println("TempAtual:\t\t0x" + String(TempAtual, HEX));
    Serial.println("TempMax:\t\t0x" + String(TempMax, HEX));
    Serial.println();
    Serial.println("VMin:\t\t\t0x" + String(VMin, HEX));
    Serial.println("VAtual:\t\t\t0x" + String(VAtual, HEX));
    Serial.println("VMax:\t\t\t0x" + String(VMax, HEX));
    Serial.println();  // Ruido
    Serial.println("RuidoMin:\t\t0x" + String(RuidoMin, HEX));
    Serial.println("RuidoMed:\t\t0x" + String(RuidoMed, HEX));
    Serial.println("RuidoMax:\t\t0x" + String(RuidoMax, HEX));
    Serial.println();  // RSSI
    Serial.println("RSSIIda:\t\t0x" + String(RSSIIda, HEX));
    Serial.println("RSSIVolta:\t\t0x" + String(RSSIVolta, HEX));
    Serial.println("SNRIda:\t\t\t0x" + String(SNRIda, HEX));
    Serial.println("SNRVolta:\t\t0x" + String(SNRVolta, HEX));
    Serial.println("IDGateway:\t\t0x" + String(IDGateway[0], HEX) + " " +
                   String(IDGateway[1], HEX));
    Serial.println();
    Serial.println("========================================================");
    Serial.println();
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
}

LoRa::LoRa() {
	SerialLoRa.begin(9600);
}

void LoRa::debugConfiguracoesLoRa() {
    Serial.println();
    Serial.println("========================================================");
    Serial.println();
    Serial.println("ID:              \t0x" + String(ID[MSB], HEX) + " " +
                   String(ID[LSB], HEX));
    Serial.println("UID:             \t0x" + String(UID[0], HEX) + " " +
                   String(UID[1], HEX) + " " + String(UID[2], HEX) + " " +
                   String(UID[3], HEX));
    Serial.println();  // Parametros
    Serial.println("Potencia:        \t0x" + String(POT, HEX));
    Serial.println("BandWidth:       \t0x" + String(BW, HEX));
    Serial.println("SpreadingFactor: \t0x" + String(SF, HEX));
    Serial.println("Coding Rate:     \t0x" + String(CR, HEX));
    Serial.println();  // Configuracao
    Serial.println("Revisao FW:      \t0x" + String(FW, HEX));
    Serial.println("Canal:           \t0x" + String(Canal, HEX));
    Serial.println("BAUD:            \t0x" + String(BAUD, HEX));
    Serial.println("Versao FW:       \t0x" + String(VersaoFW, HEX));
    Serial.println("Banco de Memoria:\t0x" + String(BancoMemoria, HEX));
    Serial.println("TestePeriodico:  \t0x" + String(TestePeriodico[0], HEX) +
                   " " + String(TestePeriodico[1], HEX));
    Serial.println("MascaraConfig:   \t0x" + String(MascaraConfig, HEX));
    Serial.println();  // Modo de Operacao
    Serial.println("Classe:          \t0x" + String(Classe, HEX));
    Serial.println("Janela:          \t0x" + String(Janela, HEX));
    Serial.println("Transp:          \t0x" + String(Transp, HEX));
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(0x00);
}

void LoRa::LeituraConfiguracoesLoRa() {
    LeituraConfiguracaoRadio();
    LeituraParametrosRadio();
    LeituraModoOperacao();
    LeituraInterfaceTransparente();
    // LeituraConfiguracaoTestePeriodico(); // Ja lido no LeituraLocal()
}

void LoRa::EscreveConfiguracoesLoRa() {
    //while(!(TestePeriodico == testePeriodico)){
      //TestePeriodico[LSB] = testePeriodico[LSB];
      //TestePeriodico[MSB] = testePeriodico[MSB];
      //EscritaConfiguracaoTestePeriodico();


      //while(!((MascaraConfig == mascaraConfig) && (BAUD = baud))){
        //uint8_t id[2] = {ID[MSB],ID[LSB]};
        //uint8_t mascaraConfig = MascaraConfig;
        //uint8_t baud = BAUD;
        EscritaConfiguracaoRadio();
      //}

      //while(!((POT == pot) && (BW = bw) && (SF = sf) && (CR = cr))){
        //uint8_t pot = POT;
        //uint8_t bw = BW;
        //uint8_t sf = SF;
        //uint8_t cr = CR;
        EscritaParametrosRadio();
      //}

      //while(!((Classe == classe) && (Janela = janela))){
        //uint8_t classe = Classe;
        //uint8_t janela = Janela;
        EscritaModoOperacao();
      //}

      //while(!(Transp == transp)){
        //uint8_t transp = Transp;
        EscritaInterfaceTransparente();
}

LoRa::~LoRa() {
}
