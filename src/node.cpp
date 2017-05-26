#include "nrfmesh.hpp"
#include "common.hpp"
#include "config.hpp"

nrf24l01::nrfmesh<10, sizeof(generic_message)> mesh{(const uint8_t*)ADDRESS, ID};

bool is_switch_on[board_max_switches] = {};
bool is_switch_on_for_pir[board_max_switches] = {};
unsigned long pir_on_time[board_max_switches] = {};
unsigned long last_button_press[board_max_buttons] = {};
unsigned long last_pir_event[board_max_pirs] = {};
bool last_pir_status[board_max_pirs] = {};
unsigned long last_status_message = 0;

rng rnd(ID);

void setup(){
    for (uint8_t i=0; i<board_max_switches; i++) {
        pinMode(switch_base_pin+i, OUTPUT);
        digitalWrite(switch_base_pin+i, LOW);
        is_switch_on[i] = false;
        is_switch_on_for_pir[i] = false;
    }
    for (uint8_t i=0; i<board_max_buttons; i++) {
        pinMode(button_base_pin+i, INPUT_PULLUP);
        last_button_press[i] = 0;
    }
    for (uint8_t i=0; i<board_max_pirs; i++) {
        pinMode(pir_base_pin+i, INPUT_PULLUP);
        last_pir_event[i] = 0;
        last_pir_status[i] = true;
    }
    pinMode(light_pin, INPUT_PULLUP);
}

void send_status() {
    last_status_message = millis();
    status_message* pkt = (status_message*) mesh.get_next_packet();
    uint8_t switch_status = 0;
    for (uint8_t i=0; i<board_max_switches; i++)
        if (is_switch_on[i])
            switch_status |= 1<<i;
    *pkt = status_message{ID, get_battery_level(), switch_status};
    mesh.send();
}

void flip_switch(uint8_t switch_id) {
    is_switch_on[switch_id] = !is_switch_on[switch_id];
    digitalWrite(switch_base_pin + switch_id, is_switch_on[switch_id] ? HIGH : LOW);
    is_switch_on_for_pir[switch_id] = false;
}

void handle_peer_event(const peer_event_message* msg) {
    if (msg->is_pir) {
        for (unsigned i=0; i<board_max_switches; i++) {
            if (Config::is_pir_enabled(i, msg->node, msg->id)) {
                if (!is_switch_on[i]) {
                    flip_switch(i);
                    is_switch_on_for_pir[i] = true;
                }
                pir_on_time[i] = millis();
            }
        }
    } else {
        for (unsigned i=0; i<board_max_switches; i++) {
            if (Config::is_button_enabled(i, msg->node, msg->id)) {
                flip_switch(i);
            }
        }
    }
    send_status();
}

void handle_status(const status_message* msg) {}
void handle_master_command(const master_command_message* msg) {
    if (msg->target_node == ID && msg->switch_id < board_max_switches) {
        if (msg->turn_on != is_switch_on[msg->switch_id]) {
            flip_switch(msg->switch_id);
        } else {
            is_switch_on_for_pir[msg->switch_id] = false;
        }
    }
    send_status();
}
void handle_config_buttons(const config_buttons_message* msg) {
    if (msg->target_node != ID) return;
    if (msg->is_for_pir) Config::set_switch_pirs(msg->switch_id, msg->source_node, msg->button_mask);
    else Config::set_switch_buttons(msg->switch_id, msg->source_node, msg->button_mask);
}
void handle_config_time(const config_time_message* msg) {
    if (msg->target_node != ID) return;
    Config::set_switch_pir_time(msg->switch_id, msg->time);
}

void loop(){
    uint8_t* pkt = mesh.try_receive();
    if (pkt != nullptr) {
        generic_message* msg = (generic_message*) pkt;
        if (msg->from_master) {
            master_message* mmsg = (master_message*) msg;
            if (mmsg->is_config) {
                config_message* cmsg = (config_message*) mmsg;
                if (cmsg->is_time) handle_config_time((config_time_message*) cmsg);
                else handle_config_buttons((config_buttons_message*) cmsg);
            } else handle_master_command((master_command_message*) msg);
        } else {
            peer_message* pmsg = (peer_message*) msg;
            if (pmsg->is_status) handle_status((status_message*) pmsg);
            else handle_peer_event((peer_event_message*) pmsg);
        }
    }

    // Check if a switch turned on by a PIR has reached timeout
    for (uint8_t i=0; i<board_max_switches; i++) {
        if (!is_switch_on_for_pir[i]) continue;
        if (millis() - pir_on_time[i] > Config::get_switch_pir_time(i)*1000) {
            flip_switch(i);
            send_status();
        }
    }

    // Check if a button is being pressed
    for (uint8_t i=0; i<board_max_buttons; i++) {
        if (digitalRead(button_base_pin+i) == LOW) {
            unsigned long interval = millis() - last_button_press[i];
            if (interval < button_interval) continue;
            last_button_press[i] = millis();
            peer_event_message* pkt = (peer_event_message*) mesh.get_next_packet();
            *pkt = peer_event_message{ID, get_battery_level(), false, i};
            mesh.send();
            handle_peer_event(pkt);
        }
    }

    // Check for current PIR events
    for (uint8_t i=0; i<board_max_pirs; i++) {
        bool status = digitalRead(pir_base_pin+i);
        if (status && !last_pir_status[i] && digitalRead(light_pin)) {
            if (millis() - last_pir_event[i] >= pir_interval) {
                last_pir_event[i] = millis();
                peer_event_message* pkt = (peer_event_message*) mesh.get_next_packet();
                *pkt = peer_event_message{ID, get_battery_level(), true, i};
                mesh.send();
                handle_peer_event(pkt);
            }
        }
        last_pir_status[i] = status;
    }

    // Check if we need to send a status packet
    unsigned long status_interval = status_min_interval + ((unsigned long) rnd() >> 2);
    if (millis() - last_status_message > status_interval) {
        send_status();
    }
}

