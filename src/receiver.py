#!/usr/bin/env python
from email import message
import time
from lora import *
from crc import crc16

def main():
	ser = criaLora()
	if ser.closed:
		print("Erro ao abrir a porta serial")
		return

	while True:
		message = ser.read(9)
		print(message.decode('ascii'))
		#if len(message) > 0:
			#print('Chegou', type(message), message)

if __name__ == "__main__":
	main()