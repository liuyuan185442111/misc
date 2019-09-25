#!/bin/env python 
#展示tcp自连接
import socket
import time

connected=False
while (not connected):
        try:
                sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
                sock.setsockopt(socket.IPPROTO_TCP,socket.TCP_NODELAY,1)
                sock.bind(('127.0.0.1',55555))
                sock.connect(('127.0.0.1',55555))
                connected=True
        except socket.error,(value,message):
                print message

        if not connected:
                print "reconnect"

print "tcp self connection occurs!"
print "try to run follow command : "
print "netstat -anp|grep 55555"
time.sleep(1800)
