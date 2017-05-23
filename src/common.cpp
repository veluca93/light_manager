#include "common.hpp"

uint8_t get_battery_level() {
    return analogRead(battery_lvl_pin)/4;
}
