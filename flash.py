import serial
import os

CHUNK_SIZE = 4096  # 4KB

# Open the serial port
ser = serial.Serial('COM11', baudrate=115200)

f = open('dump.bin', 'rb')

bytesRead = 0

while True :
    data = ser.readline().decode().rstrip()
    print(data)
    if data=='W':
        # Read the next chunk of data
        chunk = f.read(CHUNK_SIZE)

        # Check if we've reached the end of the file
        if not chunk:
            break

        # Send the chunk of data over the serial port
        ser.write(chunk)
        bytesRead += len(chunk)
        print(f'Sent {len(chunk)} bytes to serial port. Total bytes sent: {bytesRead}')
    
f.close()

# Close the serial port
ser.close()