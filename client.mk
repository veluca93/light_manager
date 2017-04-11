SOURCES := client.cpp
TARGET := client
SERIALDEV := /dev/ttyACM1
LIBRARIES := SPI Mirf
BOARD := uno
include arduino.mk

