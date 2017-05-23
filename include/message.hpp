#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <Arduino.h>

struct __attribute__((packed)) generic_message {
    uint8_t from_master:1;
    uint8_t node:7;
    uint8_t data1;
    uint8_t data2;
    uint8_t data3;
private:
    generic_message() {}
};

struct __attribute__((packed)) peer_message {
    uint8_t from_master:1;
    uint8_t node:7;
    uint8_t bat_lvl;
    uint8_t is_status:1;
    uint8_t data2:7;
    uint8_t data3;
private:
    peer_message() {}
};

struct __attribute__((packed)) status_message {
    uint8_t from_master:1;
    uint8_t node:7;
    uint8_t bat_lvl;
    uint8_t is_status:1;
    uint8_t reserved:7;
    uint8_t unused:4;
    uint8_t switch_status:4;
    status_message(uint8_t my_id, uint8_t bat_lvl, uint8_t switch_status):
        from_master(0), node(my_id), bat_lvl(bat_lvl), is_status(1),
        reserved(0), switch_status(switch_status) {}
};

struct __attribute__((packed)) peer_event_message {
    uint8_t from_master:1;
    uint8_t node:7;
    uint8_t bat_lvl;
    uint8_t is_status:1;
    uint8_t reserved:3;
    uint8_t is_pir:1;
    uint8_t id:3;
    uint8_t unused;
    peer_event_message(uint8_t my_id, uint8_t bat_lvl, bool is_pir, uint8_t id):
        from_master(0), node(my_id), bat_lvl(bat_lvl), is_status(0),
        reserved(0), is_pir(is_pir), id(id) {}
};

struct __attribute__((packed)) master_message {
    uint8_t from_master:1;
    uint8_t target_node:7;
    uint8_t is_config:1;
    uint8_t data1:7;
    uint8_t data2;
    uint8_t data3;
private:
    master_message() {}
};

struct __attribute__((packed)) master_command_message {
    uint8_t from_master:1;
    uint8_t target_node:7;
    uint8_t is_config:1;
    uint8_t turn_on:1;
    uint8_t reserved:4;
    uint8_t switch_id:2;
    uint8_t unused1;
    uint8_t unused2;
    master_command_message(uint8_t target_node, bool turn_on, uint8_t switch_id):
        from_master(1), target_node(target_node), is_config(0),
        turn_on(turn_on), reserved(0), switch_id(switch_id) {}
};

struct __attribute__((packed)) config_message {
    uint8_t from_master:1;
    uint8_t target_node:7;
    uint8_t is_config:1;
    uint8_t is_time:1;
    uint8_t data1:6;
    uint8_t data2;
    uint8_t data3;
private:
    config_message() {}
};

struct __attribute__((packed)) config_buttons_message {
    uint8_t from_master:1;
    uint8_t target_node:7;
    uint8_t is_config:1;
    uint8_t is_time:1;
    uint8_t reserved:4;
    uint8_t switch_id:2;
    uint8_t button_mask;
    uint8_t source_node:7;
    uint8_t is_for_pir:1;
    config_buttons_message(
        uint8_t target_node, uint8_t switch_id, uint8_t button_mask,
        uint8_t source_node, bool is_for_pir
    ): from_master(1), target_node(target_node), is_config(1), is_time(0),
       reserved(0), switch_id(switch_id), button_mask(button_mask),
       source_node(source_node), is_for_pir(is_for_pir) {}
};

struct __attribute__((packed)) config_time_message {
    uint8_t from_master:1;
    uint8_t target_node:7;
    uint8_t is_config:1;
    uint8_t is_time:1;
    uint8_t reserved:4;
    uint8_t switch_id:2;
    uint16_t time;
    config_time_message(uint8_t target_node, uint8_t switch_id, uint16_t time):
        from_master(1), target_node(target_node), is_config(1), is_time(1),
        reserved(0), time(time) {}
};

static inline void check_() {
    static_assert(sizeof(peer_message) == sizeof(generic_message), "Invalid peer message size!");
    static_assert(sizeof(peer_message) == sizeof(status_message), "Invalid status message size!");
    static_assert(sizeof(peer_message) == sizeof(peer_event_message), "Invalid peer event message size!");
    static_assert(sizeof(master_message) == sizeof(generic_message), "Invalid master message size!");
    static_assert(sizeof(master_message) == sizeof(master_command_message), "Invalid master command message size!");
    static_assert(sizeof(master_message) == sizeof(config_message), "Invalid config message size!");
    static_assert(sizeof(config_message) == sizeof(config_buttons_message), "Invalid config buttons message size!");
    static_assert(sizeof(config_message) == sizeof(config_time_message), "Invalid config time message size!");
}

#endif
