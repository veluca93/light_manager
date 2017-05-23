#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <EEPROM.h>
#include "common.hpp"

class Config {
    static uint16_t get_addr(uint8_t switch_id, uint8_t node_id, bool for_pir = false) {
        // The compiler should change those to bit-level operations
        return (uint16_t)max_num_nodes*switch_id*2 + node_id*2 + for_pir;
    }
public:
    static void set_switch_pir_time(uint8_t switch_id, uint16_t time) {
        EEPROM.put(get_addr(switch_id, 0), time);
    }
    static uint16_t get_switch_pir_time(uint8_t switch_id) {
        uint16_t ret = 0;
        EEPROM.get(get_addr(switch_id, 0), ret);
        return ret;
    }
    static void set_switch_buttons(uint8_t switch_id, uint8_t node_id, uint8_t button_mask) {
        EEPROM.put(get_addr(switch_id, node_id), button_mask);
    }
    static void set_switch_pirs(uint8_t switch_id, uint8_t node_id, uint8_t pir_mask) {
        EEPROM.put(get_addr(switch_id, node_id, true), pir_mask);
    }
    static bool is_button_enabled(uint8_t switch_id, uint8_t node_id, uint8_t button_id) {
        uint8_t button_mask = 0;
        EEPROM.get(get_addr(switch_id, node_id), button_mask);
        return button_mask & (1<<button_id);
    }
    static bool is_pir_enabled(uint8_t switch_id, uint8_t node_id, uint8_t pir_id) {
        uint8_t pir_mask = 0;
        EEPROM.get(get_addr(switch_id, node_id), pir_mask);
        return pir_mask & (1<<pir_id);
    }
};

#endif
