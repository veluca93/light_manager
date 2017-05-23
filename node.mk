SOURCES := node.cpp
TARGET := node
SERIALDEV := /dev/ttyACM1
ID := 1 # 0 is the id of the sink
include common.mk

