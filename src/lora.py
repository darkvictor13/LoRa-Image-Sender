import serial

CMD_PARAMETROS      = 0xD6 # Leitura e escrita dos parametros de radio

CMD_LEITURA_LOCAL   = 0xE2 # Leitura dos parametros do modulo (apenas pela serial local)
CMD_LEITURA_REMOTA  = 0xD4 # Realizado apenas atraves do mestre da rede
CMD_CONFIG_RADIO    = 0xCA # Escrita dos parametros de um radio (ID e Mascara bin  ́aria)

CMD_GPIO            = 0xC2 # Comando de configuracao, leitura e escrita de CMD_GPIO

CMD_DIAGNOSTICO     = 0xE7 # Adquire informacoes de operacao de um radio (local ou remoto)
CMD_RUIDO           = 0xD8 # Leitura do nivel de ruido observado por um radio (local ou remoto)
CMD_RSSI            = 0xD5 # Retorna os niveis de potencia de sinal observados (ida e volta) no enlace entre dois radios
CMD_ROTA            = 0xD2 # Retorna a rota utilizada para se comunicarcom um determinado r  ́adio
CMD_TESTE_PERIODICO = 0x01 # Teste Periodico enviado dos escravos para o mestre. Configuravel pelo comando 0xCA
CMD_TEMPO_PERIODICO = 0xCC # Configura ou le o tempo periodico no radiomestre
CMD_MODO_OPERACAO   = 0xC1 # Configura ou le a classe do dispositivo e o comando da interface transparente 

# Subcomandos TempoPeriodico
CMD_LEITURA_TEMPO_PERIODICO   = 0x02
CMD_ESCRITA_TEMPO_PERIODICO   = 0x01

# SubComandos ConfigClassnInterf

CMD_CONFIG_MODO_OPERACAO      = 0x00
CMD_CONFIG_INTERF_TRANSP      = 0x01

# SubComandosReadWriteParam

#  Escrita e Leitura
CMD_LEITURA_PARAMETROS        = 0x00
CMD_ESCRITA_PARAMETROS        = 0x01

def lerLora(serial : serial.Serial):
	serial.write(bytearray([0, 0, CMD_LEITURA_LOCAL, 0, 0, 0]))
	mensagem = serial.read(31)
	if len(mensagem) == 0:
		print("Erro ao ler o modulo")
		return
	print('Chegou a mensagem: ' + mensagem)
	id = bytearray([mensagem[0], mensagem[1]])
	uid = bytearray([mensagem[5], mensagem[6], mensagem[7], mensagem[8]])
	return id, uid

def escreverLora(serial : serial.Serial, id : bytearray, uid : bytearray):
	pass

def enviaMensagemLora(serial : serial.Serial, id : bytearray, mensagem : bytearray):
	pass

def verificaEnvioLora(serial : serial.Serial):
	pass