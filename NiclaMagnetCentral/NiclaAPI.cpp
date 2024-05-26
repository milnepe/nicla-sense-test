#include "NiclaAPI.h"

// Nicla warning data
static niclaData data;
NiclaAPI::NiclaAPI() {
}

void NiclaAPI::init() {
  data.severityLevel = NORMAL;
}

int NiclaAPI::updateWarning(warning_levels level) {
  static warning_levels previous_level = NORMAL;
  if (level != previous_level) {
    previous_level = level;
    switch (level) {
      case NORMAL:
        led_colour(GREEN);
        break;
      case REPLACE:
        led_colour(RED);
        // buzzer_on();
        break;
      case MONITOR:
        led_colour(AMBER);
        // buzzer_on();
        break;
      default:
        break;
    }
  }
  return level;
}
