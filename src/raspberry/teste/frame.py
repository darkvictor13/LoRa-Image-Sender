from crc import crc16
FRAME_SIZE = 2048
ACK  = 1
NACK = 0

class Frame:
	def __init__(self, frame_id: int, buffer: bytes) -> None:
		self.frame_id = frame_id
		self.buffer: bytes = buffer
		self.size = len(buffer)
		self.crc = crc16(buffer)