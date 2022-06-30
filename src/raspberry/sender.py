#!/usr/bin/env python
import time
import serial
from time import sleep
from lora import *

def main():
	mensagem = bytearray([1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1])

	ser = criaLora()
	if ser.closed:
		print("Erro ao abrir a porta serial")
		return
	else:
		print("Porta serial aberta")

	id, uid = lerLora(ser)
	print(id, uid)
	#while True:
		#enviaMensagemLora(ser, _id, mensagem)
		#print("Chegou mensagem " + str(verificaEnvioLora(ser)))
		#sleep(1)

if __name__ == "__main__":
	main()