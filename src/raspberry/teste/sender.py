from io import BytesIO
from time import sleep
from picamera import PiCamera
from frame import Frame, FRAME_SIZE

def frames_from_bytes(bytes_io: BytesIO, frame_size: int) -> list[Frame]:
	print('Estou na func frames_from_bytes')
	frames: list[Frame] = []

	buffer = bytes_io.getbuffer().tobytes('C')
	# i vai de 0 a len(buffer) de frame_size em frame_size
	# o ultimo frame fica sempre com o resto do buffer, nao necessariamente o tamanho máximo
	for i in range(0, len(buffer), frame_size):
		frames.append(Frame(i // frame_size, buffer[i:i + frame_size]))
	return frames

def main() -> None:
	print('Iniciando a camera')
	try:
		# Resoluções possíveis:
		# 4:3
		# VGA (640x480)
		# SVGA (800x600)
		# 16:9
		# HD (1280x720)
		camera = PiCamera(resolution='VGA')
		sleep(2)
		print('Camera iniciada')

		img_buffer = BytesIO()
		camera.capture(img_buffer, format='jpeg')
		print('Captura realizada')
		frames = frames_from_bytes(img_buffer, FRAME_SIZE)
		print('Informações dos frames:')
		print('Frames Gerados', len(frames))
		for frame in frames:
			print('Frame id', frame.frame_id, 'tamanho', frame.size)
	except Exception as e:
		print(e)

if __name__ == '__main__':
	main()
