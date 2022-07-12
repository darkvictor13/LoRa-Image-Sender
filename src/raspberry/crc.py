# implementar controle de erros com CRC 16 bits
def crc16(data):
    tamanho = len(data)
    bitbang = 0
    crc_calc = 0xC181
    for x in range(0, tamanho):
        crc_calc ^= data[x] & 0x00FF
        for j in range(0, 8):
            bitbang=crc_calc
            crc_calc>>=1
            if(int(bitbang) & 1):
                crc_calc ^= 0xA001

    print('CRC de', data, '= ', hex(crc_calc))
    return crc_calc