#ifndef COMMON_HPP
#define COMMON_HPP

#include <Arduino.h>
#include "message.hpp"

static const uint8_t max_num_nodes = 128;
static const uint8_t max_num_switches = 4;
static const uint8_t max_num_pirs = 8;
static const uint8_t max_num_buttons = 8;

static const uint8_t switch_base_pin = 9;
static const uint8_t board_max_switches = 2;

static const uint8_t pir_base_pin = 4;
static const uint8_t board_max_pirs = 2;

static const uint8_t button_base_pin = A1;
static const uint8_t board_max_buttons = 5;

static const uint8_t light_pin = 6;

static const uint8_t battery_lvl_pin = A0;

static const unsigned long button_interval = 300;
static const unsigned long pir_interval = 1000;

static const unsigned long status_min_interval = 60000;

uint8_t get_battery_level();

#endif
