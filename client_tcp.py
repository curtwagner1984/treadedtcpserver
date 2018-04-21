#!/usr/bin/env python

import serial
import os
import atexit
import re
import time
from datetime import datetime
import threading
import Queue
import socket
import struct
import crc16
import numpy as np

def crc16n(data):
    '''
    CRC-16-CCITT Algorithm
    '''
    data = bytearray(data)
    poly = 0x8408
    crc = 0xFFFF
    for b in data:
        cur_byte = 0xFF & b
        for _ in range(0, 8):
            if (crc & 0x0001) ^ (cur_byte & 0x0001):
                crc = (crc >> 1) ^ poly
            else:
                crc >>= 1

            cur_byte >>= 1

    crc = (~crc & 0xFFFF)
    crc = (crc << 8) | ((crc >> 8) & 0xFF)

    return np.uint16(crc)

#fifo_arduino_control_command    = '/tmp/arduino_control_command'
#log_file                        = fifo_arduino_control_command + ".log"
#serial_path                     = "/dev/ttyUSB0"
#serial_paths                    = [ "/dev/ttyUSB0", "/dev/ttyACM0", "/dev/ttyUSB1", "/dev/ttyACM1", "/dev/ttyUSB2", "/dev/ttyACM2", "/dev/ttyUSB3", "/dev/ttyACM3" ]
#serial_baudrate                 = 9600
#serial_object                   = ""
#saved_coordinates_file          = "/var/www/html/camera_coordinates"
#serial_timeout                  = 0
#queue_commands                  = Queue.LifoQueue()
#queue_serial                    = Queue.LifoQueue()

            
class ServerThread(threading.Thread):
    def __init__(self, ip, port):
        self.ip             = ip
        self.port           = port
        self.is_stop        = False
        self.serversocket   = socket.socket(
            socket.AF_INET, socket.SOCK_STREAM)

        self.serversocket.bind((connection_ip, connection_port))
        
        self.serversocket.listen(2)
        
        threading.Thread.__init__(self)
        
        #threading.Thread.__init__(self)
        
        

    def stop(self):
        self.is_stop = True
        #self.serversocket.close();

    def run(self):
        while False == self.is_stop:
            #self.serversocket.listen(2)
            
            (clientsocket, address) = self.serversocket.accept()
            #now do something with the clientsocket
            #in this case, we'll pretend this is a threaded server
            #ct = client_thread(clientsocket)
            #ct.run()
            #ascii
            data_buffer = clientsocket.recv(2048) #.decode("ascii")
            for data in data_buffer:
                print str(data.decode("ascii")) + " "


class ClientThread(threading.Thread):
    def __init__(self, ip, port):
        self.ip             = ip
        self.port           = port
        self.is_stop        = False
        self.socket         = mysocket()
        #self.e              = threading.Event()
        
        self.socket.connect(connection_ip,connection_port)

        threading.Thread.__init__(self)


    def stop(self):
        self.is_stop = True
        #self.socket.close()

    def run(self):
        if False == self.is_stop:
            #self.socket.mysend("msg01")
            #time.sleep(5)
            #self.socket.mysend("msg02")
            header_format = "<IHHIHH"
            header_format_without_checksum = "<HIHH"
            header_format_dead_and_checksum = "<IH"
            header_magic = 0xDEADBEEF
            header_opcode = 1
            header_payload_index = 2
            header_payload_checksum = 3
            header_status = 3
            header_without_checksum = struct.pack(header_format_without_checksum, header_opcode, header_payload_index, header_payload_checksum, header_status)
            #print type(header_without_checksum)
            #print type(bytes(header_without_checksum,"ascii"))
            #print str(bytes(header_without_checksum,"ascii"))
            #print(str(b'\0'))
            header_checksum = crc16.crc16xmodem(header_without_checksum)
            header_start = struct.pack(header_format_dead_and_checksum,header_magic,header_checksum)
            
            #message = header_start + header_without_checksum
            
            
            for j in range(0,1):
                message = b''
                message = message + header_start + header_without_checksum + struct.pack("<QI", 2048, 777+j)
                for i in range(0,128):
                    message = message + struct.pack("<I", 0) 
                     
                message = message + struct.pack(header_format, header_magic, 23, 4, 2, 5, 0 )
                
                for i in range(0,128):
                    message = message + struct.pack("<I", 0xC00 + i) 
                    
                message = message + struct.pack(header_format, header_magic, 23, 4, 1, 5, 0 )
                
                for i in range(0,128):
                    message = message + struct.pack("<I",  0xB00 + i) 
                    
                message = message + struct.pack(header_format, header_magic, 23, 4, 0, 5, 0 )
                
                for i in range(0,128):
                    message = message + struct.pack("<I", 0xA00 + i) 
                    
                message = message + struct.pack(header_format, header_magic, 23, 4, 3, 5, 0 )
                
                for i in range(0,128):
                    message = message + struct.pack("<I", 0xD00 + i) 
                
                message = message + struct.pack(header_format, header_magic, 23, 4, 4, 5, 0 )
                
                for i in range(0,128):
                    message = message + struct.pack("<I", 0xE00 + i) 
                     
                self.socket.mysend(message)
                
                time.sleep(1)
                
                #print(self.socket.myreceive2())
                
                
                
            time.sleep(2)
            
            
class mysocket:
    '''demonstration class only
      - coded for clarity, not efficiency
    '''

    def __init__(self, sock=None):
        if sock is None:
            self.sock = socket.socket(
                socket.AF_INET, socket.SOCK_STREAM)
        else:
            self.sock = sock

    def connect(self, host, port):
        self.sock.connect((host, port))

    def mysend(self, msg):
        totalsent = 0
        while totalsent < len(msg):
            sent = self.sock.send(msg[totalsent:])
            if sent == 0:
                raise RuntimeError("socket connection broken")
            totalsent = totalsent + sent

    def myreceive(self):
        chunks = []
        bytes_recd = 0
        while bytes_recd < 16:
            chunk = self.sock.recv(min(16 - bytes_recd, 2048))
            if chunk == '':
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            bytes_recd = bytes_recd + len(chunk)
        return ''.join(chunks)
        
    def myreceive2(self):
        return self.sock.recv(1024)

connection_port = 4000;
connection_ip = "127.0.0.1";

        
def main():
    global connection_port
    global connection_ip
    
    #my_server = ServerThread(connection_ip, connection_port)
    #my_server.start()
    
    
    my_client = ClientThread(connection_ip, connection_port)
    my_client.start()
    #my_soc = mysocket();
    
    #my_soc.connect(connection_ip,connection_port)
    
    #my_soc.mysend("sadasdasdas")
    
    #print(5)
    #time.sleep(5)
    
    #my_server.join(5)
    my_client.join(5)
    print("done")
    #my_server.stop()
    my_client.stop()
    #my_server.stop()

main()

