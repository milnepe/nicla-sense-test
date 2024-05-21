#ifndef _NICLA_API_H_
#define _NICLA_API_H_

#include "magnet_config.h"
#include "led.h"
#include "buzzer.h"

enum warning_levels { NONE,
                      NORMAL,
                      MONITOR,
                      REPLACE };

struct niclaData {
  warning_levels severityLevel = NONE;
  float pressure = 0;
  float temperature = 0;
  float air_quality = 0;
  uint32_t co2 = 0;
};

class NiclaAPI {
public:
  niclaData data;  // Nicla sensor data
  // warning_levels level;
  NiclaAPI();
public:
  void init();
  int updateWarning(warning_levels level);
  void getData();
};

#endif
