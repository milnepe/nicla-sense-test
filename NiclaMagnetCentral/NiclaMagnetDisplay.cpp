#include "NiclaMagnetDisplay.h"

// Nicla warning text
static char w1[5][12] = { "NORMAL", "DANGER TO", "Niclaing is", "Niclaing is", "Warning no" };
static char w2[5][12] = { "Warnings", "LIFE", "Expected", "Possible", "Longer in" };
static char w3[5][12] = { "", "", "", "", "Force" };

void NiclaMagnetDisplay::initDisplay(void) {
  if (_epd.Init() != 0) {
    return;
  }
  Serial.println("EDP attached");

  _epd.ClearFrameMemory(0xFF);  // bit set = white, bit reset = black
  _epd.DisplayFrame();

  delay(2000);

  _epd.SetFrameMemory_Base(RSLOGO);
  _epd.DisplayFrame();
}

void NiclaMagnetDisplay::showGreeting(void) {
  _paint.SetWidth(120);
  _paint.SetHeight(32);
  _paint.SetRotate(ROTATE_180);

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 4, "  Nicla  ", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 140, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 4, "  Magnet  ", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 120, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 4, "Pete Wood", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 80, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 4, "Jude Pullen", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 60, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 4, "Pete Milne", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 40, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 4, "Nat Ibagon", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 20, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 4, "Simon Desir", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 0, _paint.GetWidth(), _paint.GetHeight());

  _epd.DisplayFrame_Partial();
}

void NiclaMagnetDisplay::updateDisplay() {
  Serial.println("Updating display...");
  int severityLevel = _magnet->warning.severityLevel;
  // Index warning string based on severity level
  //int warning_idx = severityLevel ? severityLevel : 0;
  //  char single_digit[] = {'0', '\0'};
  //  char double_digit[] = {'0', '0', '\0'};
  //  char three_digit[] = {'0', '/', '0', '\0'};
  //  char four_digit[] = {'0', '/', '0', '0',  '\0'};
  //
  // Set background
  if (_epd.Init() != 0) {
    return;
  }
  _epd.ClearFrameMemory(0xFF);  // bit set = white, bit reset = black
  _epd.DisplayFrame();

  delay(500);
  // Map background image .h files to levels
  switch (severityLevel) {
    case NONE:
      _epd.SetFrameMemory_Base(RSLOGO);
      break;
    case SEVERE_FLOOD_WARNING:
      _epd.SetFrameMemory_Base(epd_flood_warning_severe);
      break;
    case FLOOD_WARNING:
      _epd.SetFrameMemory_Base(epd_flood_warning);
      break;
    case FLOOD_ALERT:
      _epd.SetFrameMemory_Base(epd_flood_alert);
      break;
    case NO_LONGER:
      _epd.SetFrameMemory_Base(epd_flood_warning_removed);
      break;
    default:
      break;
  }

  _epd.DisplayFrame();

  // Static text
  _paint.SetWidth(120);
  _paint.SetHeight(40);
  _paint.SetRotate(ROTATE_180);

  // _paint.Clear(UNCOLORED);
  // _paint.DrawStringAt(0, 0, "UV Max", &Font16, COLORED);
  // _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 140, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 0, w1[severityLevel], &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 120, _paint.GetWidth(), _paint.GetHeight());

  // _paint.Clear(UNCOLORED);
  // _paint.DrawStringAt(0, 0, w2[severityLevel], &Font16, COLORED);
  // _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 100, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 0, "Pressure", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 80, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  int pressure = (int)(_magnet->warning.pressure);
  char pressure_str[] = {'0', '0', '0', '0', 'k', 'P', '\0'};
  pressure_str[0] = pressure / 100 / 10 + '0';
  pressure_str[1] = pressure / 100 % 10 + '0';
  pressure_str[2] = pressure % 100 / 10 + '0';
  pressure_str[3] = pressure % 100 % 10 + '0';
  _paint.DrawStringAt(0, 0, pressure_str, &Font12, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 60, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 0, "Temperature", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 40, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  int temp = (int)(_magnet->warning.temperature * 100);
  char temperature_str[] = {'0', '0', '.', '0', '0', 'C', '\0'};
  temperature_str[0] = temp / 100 / 10 + '0';
  temperature_str[1] = temp / 100 % 10 + '0';
  temperature_str[3] = temp % 100 / 10 + '0';
  temperature_str[4] = temp % 100 % 10 + '0';
  _paint.DrawStringAt(0, 0, temperature_str, &Font12, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 20, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 0, "Air Quality", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 0, _paint.GetWidth(), _paint.GetHeight());

  // _paint.Clear(UNCOLORED);
  // int air_quality = (int)(_magnet->warning.air_quality);
  // char air_quality_str[] = {'0', '0', '.', '0', '0', '\0'};
  // air_quality_str[0] = temp / 100 / 10 + '0';
  // air_quality_str[1] = temp / 100 % 10 + '0';
  // air_quality_str[3] = temp % 100 / 10 + '0';
  // air_quality_str[4] = temp % 100 % 10 + '0';
  // _paint.DrawStringAt(0, 0, air_quality_str, &Font12, COLORED);
  // _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 0, _paint.GetWidth(), _paint.GetHeight());

  // _paint.Clear(UNCOLORED);
  // _paint.DrawStringAt(0, 0, LINE_7, &Font16, COLORED);
  // _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 10, _paint.GetWidth(), _paint.GetHeight());

  // Status indicators
  // _paint.SetWidth(120);
  // _paint.SetHeight(40);
  // _paint.SetRotate(ROTATE_180);

  // _paint.Clear(UNCOLORED);

  // if (bleOn) {
  //   _paint.DrawStringAt(0, 0, "Wifi", &Font16, COLORED);
  // } else {
  //   _paint.DrawStringAt(0, 0, "", &Font16, COLORED);
  // }
  // _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 0, _paint.GetWidth(), _paint.GetHeight());

  _epd.DisplayFrame_Partial();
}
