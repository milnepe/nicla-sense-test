#include "NiclaAPI.h"

// Nicla warning data
static niclaData data;
NiclaAPI::NiclaAPI() {
}

void NiclaAPI::init() {
  state = INIT;
}

int NiclaAPI::updateState(warning_levels state) {
  static warning_levels previous_state = NONE;
  if (state != previous_state) {
    previous_state = state;
    switch (state) {
      case NONE:
        led_colour(GREEN);
        break;
      case SEVERE_FLOOD_WARNING:
        led_colour(RED);
        buzzer_on();
        break;
      case FLOOD_WARNING:
        led_colour(RED);
        buzzer_on();
        break;
      case FLOOD_ALERT:
        led_colour(AMBER);
        buzzer_on();
        break;
      case NO_LONGER:
        led_colour(GREEN);
        break;
      default:
        break;
    }
  }
  return state;
}
