#include "nrf24l01.hpp"

nrf24l01::nrf24l01<10, sizeof(unsigned long), 5, 2, true> nrf{};

void setup(){
    Serial.begin(9600);
    nrf.set_tx_addr((uint8_t*)"serv1");
    nrf.tx_mode();
    Serial.println("Beginning ... "); 
}

void loop(){
    unsigned long time = millis();
    Serial.print("Sending... ");
    if (nrf.send_sync((uint8_t *)&time)) {
        Serial.print("Ping: ");
        Serial.print((millis() - time));
        Serial.println("ms");
    } else {
        Serial.println("Packet lost");
    }
    delay(2000);
} 

