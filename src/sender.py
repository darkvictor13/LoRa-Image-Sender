#!/usr/bin/env python
import time
import serial
from time import sleep
from lora import *

def main():
	mensagem = bytearray([1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1])

	ser = serial.Serial (
		port='/dev/ttyS0',
		baudrate = 9600,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS,
		timeout=1
	)
	if ser.closed:
		print("Erro ao abrir a porta serial")
		return

	id, uid = lerLora(ser)
	print(id, uid)
	#escreverLora(ser, _id, _uid)
	#while True:
		#enviaMensagemLora(ser, _id, mensagem)
		#print("Chegou mensagem " + str(verificaEnvioLora(ser)))
		#sleep(1)

if __name__ == "__main__":
	main()