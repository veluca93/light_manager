SOURCES := server.cpp
TARGET := server
SERIALDEV := /dev/ttyACM0
LIBRARIES := SPI Mirf
BOARD := uno
include arduino.mk

