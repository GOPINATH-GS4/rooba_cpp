import serial
import struct
from time import sleep

port = serial.Serial("/dev/ttyUSB0", baudrate=115200, timeout=3.0)
str = ''
str = struct.pack('!B',148);
port.write(str);
sleep(.5);
str = struct.pack('!B',1);
port.write(str);
sleep(.5);
str = struct.pack('!B',7);
port.write(str);
sleep(.5);

while True:
    rcv = port.read(10)
    print rcv

port.close();
