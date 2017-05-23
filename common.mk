ADDRESS := "mesh1"
CPPFLAGS += '-DADDRESS=${ADDRESS}' -DID=${ID}
CPPFLAGS += -Wall
LIBRARIES := SPI EEPROM
SOURCES += common.cpp
BOARD := uno
include arduino.mk
