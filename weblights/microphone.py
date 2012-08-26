import pyaudio
import sys

chunk = 1024
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 44100
RECORD_SECONDS = 5

p = pyaudio.PyAudio()

stream = p.open(format = FORMAT,
                channels = CHANNELS,
                rate = RATE,
                input = True,
                output = True,
                frames_per_buffer = chunk)

print "* recording"
for i in range(0, 44100 / chunk * RECORD_SECONDS):
    data = stream.read(chunk)
    stream.write(data, chunk)
print "* done"

stream.stop_stream()
stream.close()
p.terminate()