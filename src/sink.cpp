#include "nrfmesh.hpp"
#include "message.hpp"
#include "common.hpp"

nrf24l01::nrfmesh<10, sizeof(generic_message)> mesh{(const uint8_t*)ADDRESS, max_num_nodes};

void setup(){
    Serial.begin(9600);
    Serial.println("Starting up");
    config_buttons_message* msg = (config_buttons_message*) mesh.get_next_packet();
    *msg = config_buttons_message(1, 0, (1<<1) | (1<<3), 1, false);
    mesh.send();
    *msg = config_buttons_message(1, 1, (1<<2) | (1<<3), 1, false);
    mesh.send();
}

void handle_status(status_message* pmsg) {
    Serial.println("Received status message");
}

void handle_peer_event(peer_event_message* pmsg) {
    Serial.println("Received peer event message");
}

void loop(){
    uint8_t* pkt = mesh.try_receive();
    if (pkt != nullptr) {
       generic_message* msg = (generic_message*) pkt;
        if (!msg->from_master) {
            peer_message* pmsg = (peer_message*) msg;
            if (pmsg->is_status) handle_status((status_message*) pmsg);
            else handle_peer_event((peer_event_message*) pmsg);
        }
    }
}

