#include "nrf24l01.hpp"

nrf24l01::nrf24l01<10, sizeof(unsigned long), 5, 2, true> nrf{};

void setup(){
    Serial.begin(9600);
    nrf.set_rx_addr(0, (uint8_t*)"serv1");
    nrf.rx_mode();
    Serial.println("Starting up");
}

void loop(){
    uint8_t data[nrf.payload_size];
    nrf.receive_sync(data);
    Serial.println("Packet received");
}

