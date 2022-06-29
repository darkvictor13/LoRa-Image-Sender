# implementar controle de erros com CRC 16 bits
def crc16(data: bytearray) -> int:
	crc = 0
	for byte in data:
		crc = crc ^ byte
		for _ in range(8):
			if (crc & 0x0001) == 0x0001:
				crc = (crc >> 1) ^ 0xA001
			else:
				crc = (crc >> 1)

	print('CRC de', data, '= ', hex(crc))
	return crc