SOURCES := node.cpp
SERIALDEV := /dev/ttyACM1
ID := 1 # 0 is the id of the sink
TARGET := node_${ID}
include common.mk

