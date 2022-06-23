#!/usr/bin/env python
import time
import serial
from crc import crc16

def main():
	ser = serial.Serial (
		port='/dev/ttyS0',
		baudrate = 9600,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS,
		timeout=1
	)

	data = bytearray([1,1,1,1,1,1,1,1,1,1,1,1,1])
	print(crc16(data))

	while True:
		x=ser.readline()
		print(x.decode('ascii'), end='')

if __name__ == "__main__":
	main()