#!/usr/bin/env python3
 # coding=utf-8


import os


fd = os.open("/dev/solution_node", os.O_RDWR | os.O_NOCTTY)
test = open(fd, "wb+", buffering=0)
test.write(b"Hello")
test.seek(0, 0)
data = test.read()
print(data)
test.seek(0, 0)
data = test.read()
print(data)
