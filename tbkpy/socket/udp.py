import sys
import socket
import struct

class UDPMultiCastReceiver:
    def __init__(self, group, port):
        self.group = group
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(('', port))
        mreq = struct.pack("4sl", socket.inet_aton(group), socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    
    def recv(self):
        try:
            return True, self.sock.recvfrom(65536)
        except socket.error as e:
            return False, None

    def setblocking(self, blocking):
        self.sock.setblocking(blocking)

class UDPMultiCastSender:
    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)
        # self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, 1)
        # self.sock.bind(('', 0))
    
    def send(self, msg, ep):
        self.sock.sendto(msg, ep)

if __name__ == "__main__":
    if len(sys.argv)>1 and sys.argv[1] == "send":
        s = UDPMultiCastSender()
        s.send("Hello".encode(), ("233.233.233.233", 12321))
    else:
        d = UDPMultiCastReceiver("233.233.233.233", 12321)
        while True:
            print(d.recv())