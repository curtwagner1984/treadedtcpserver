#!/usr/bin/env python
# -*- coding: utf-8 -*-

import threading

import socket
import struct
import time


class ClientThread(threading.Thread):
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
        self.is_stop = False
        self.socket = mysocket()

        self.socket.connect(connection_ip, connection_port)

        threading.Thread.__init__(self)

    def stop(self):
        self.is_stop = True
        # self.socket.close()

    def run(self):
        if False == self.is_stop:
            header_format = "<IHHIHH"
            header_format_without_checksum = "<HIHH"
            header_format_dead_and_checksum = "<IH"
            header_magic = 0xDEADBEEF
            header_opcode = 1
            header_payload_index = 2
            header_payload_checksum = 3
            header_status = 3
            header_without_checksum = struct.pack(header_format_without_checksum, header_opcode, header_payload_index,
                                                  header_payload_checksum, header_status)
            # print type(header_without_checksum)
            # print type(bytes(header_without_checksum,"ascii"))
            # print str(bytes(header_without_checksum,"ascii"))
            # print(str(b'\0'))

            # https://docs.python.org/3/library/struct.html
            header_checksum = 15
            header_start = struct.pack(header_format_dead_and_checksum, header_magic, header_checksum)

            # message = header_start + header_without_checksum

            for j in range(0, 1):
                message = b''
                message = message + header_start + header_without_checksum + struct.pack("<QI", 2048, 777 + j)
                for i in range(0, 128):
                    message = message + struct.pack("<I", 0)

                message = message + struct.pack(header_format, header_magic, 23, 4, 2, 5, 0)

                for i in range(0, 128):
                    message = message + struct.pack("<I", 0xC00 + i)

                message = message + struct.pack(header_format, header_magic, 23, 4, 1, 5, 0)

                for i in range(0, 128):
                    message = message + struct.pack("<I", 0xB00 + i)

                message = message + struct.pack(header_format, header_magic, 23, 4, 0, 5, 0)

                for i in range(0, 128):
                    message = message + struct.pack("<I", 0xA00 + i)

                message = message + struct.pack(header_format, header_magic, 23, 4, 3, 5, 0)

                for i in range(0, 128):
                    message = message + struct.pack("<I", 0xD00 + i)

                message = message + struct.pack(header_format, header_magic, 23, 4, 4, 5, 0)

                for i in range(0, 128):
                    message = message + struct.pack("<I", 0xE00 + i)

                self.socket.mysend(message)

                time.sleep(1)

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
        # self.sock.connect((host, port))

    def mysend(self, msg):
        totalsent = 0
        while totalsent < len(msg):
            sent = self.sock.send(msg[totalsent:])
            if sent == 0:
                raise RuntimeError("socket connection broken")
            totalsent = totalsent + sent

    def myreceive(self):
        MSGLEN = 100
        chunks = []
        bytes_recd = 0
        while bytes_recd < MSGLEN:
            chunk = self.sock.recv(min(MSGLEN - bytes_recd, 2048))
            if chunk == '':
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            bytes_recd = bytes_recd + len(chunk)
        return ''.join(chunks)


connection_port = 4000
connection_ip = "127.0.0.1"


def main():
    global connection_port
    global connection_ip

    my_client = ClientThread(connection_ip, connection_port)
    my_client.start()
    my_client.join(5)
    print("done")

    my_client.stop()
    # my_server.stop()


main()
