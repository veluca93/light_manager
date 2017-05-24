#include "nrfmesh.hpp"
#include "message.hpp"
#include "common.hpp"

nrf24l01::nrfmesh<10, sizeof(generic_message)> mesh{(const uint8_t*)ADDRESS, max_num_nodes};

void setup(){
    Serial.begin(9600);
}

void handle_status(status_message* msg) {
    Serial.write(123); // magic byte
    Serial.write(1);
    Serial.write(msg->node);
    Serial.write(msg->bat_lvl);
    Serial.write(msg->switch_status);
    Serial.write(0); // Padding
}

void handle_peer_event(peer_event_message* msg) {
    Serial.write(123); // magic byte
    Serial.write(0);
    Serial.write(msg->node);
    Serial.write(msg->bat_lvl);
    Serial.write(msg->is_pir);
    Serial.write(msg->id);
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
    if (Serial.available() > 6) {
        uint8_t magic = Serial.read();
        if (magic != 123) {
            return;
        }
        uint8_t kind = Serial.read();
        uint8_t target_node = Serial.read();
        uint8_t switch_id = Serial.read();
        switch (kind) {
        case 0: {
            uint8_t turn_on = Serial.read();
            // Skip two bytes sent as padding
            Serial.read();
            Serial.read();
            master_command_message* msg = (master_command_message*) mesh.get_next_packet();
            *msg = master_command_message{target_node, turn_on, switch_id};
            break;
        }
        case 1: {
            uint8_t source_node = Serial.read();
            uint8_t button_mask = Serial.read();
            uint8_t is_for_pir = Serial.read();
            config_buttons_message* msg = (config_buttons_message*) mesh.get_next_packet();
            *msg = config_buttons_message{target_node, switch_id, button_mask, source_node, is_for_pir};
            break;
        }
        case 2: {
            uint16_t time = (uint16_t)Serial.read() << 8; // MSB first
            time |= Serial.read();
            Serial.read(); // Skip one byte of padding
            config_time_message* msg = (config_time_message*) mesh.get_next_packet();
            *msg = config_time_message{target_node, switch_id, time};
            break;
        }
        default: {
            return;
        }
        }
        mesh.send();
    }
}

