#include "NiclaMagnetDisplay.h"

// Nicla warning text
static char warning_text[4][12] = { "NONE", "NORMAL", "MONITOR", "REPLACE" };

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

void NiclaMagnetDisplay::updateState() {
  Serial.println("Updating state image...");
  int severityLevel = _magnet->data.severityLevel;

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
    case REPLACE:
      _epd.SetFrameMemory_Base(epd_flood_warning_severe);
      break;
    case MONITOR:
      _epd.SetFrameMemory_Base(epd_flood_warning);
      break;
    case NORMAL:
      _epd.SetFrameMemory_Base(epd_flood_alert);
      break;
    default:
      break;
  }

  _epd.DisplayFrame();
}

void NiclaMagnetDisplay::updateReadings() {
  Serial.println("Updating sensor readings...");
  int severityLevel = _magnet->data.severityLevel;

  // Static text
  _paint.SetWidth(120);
  // _paint.SetHeight(40);
  _paint.SetHeight(20);
  _paint.SetRotate(ROTATE_180);

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 0, warning_text[severityLevel], &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 170, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 0, "CO2", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 140, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  int co2 = _magnet->data.co2;
  char co2_str[] = { '0', '0', '0', 'p', 'p', 'm', '\0' };
  // co2_str[0] = co2 / 100 / 10 + '0';
  co2_str[0] = co2 / 100 % 10 + '0';
  co2_str[1] = co2 % 100 / 10 + '0';
  co2_str[2] = co2 % 100 % 10 + '0';
  _paint.DrawStringAt(0, 0, co2_str, &Font12, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 120, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 0, "Pressure", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 100, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  int pressure = (int)(_magnet->data.pressure);
  char pressure_str[] = { '0', '0', '0', '0', 'k', 'P', '\0' };
  pressure_str[0] = pressure / 100 / 10 + '0';
  pressure_str[1] = pressure / 100 % 10 + '0';
  pressure_str[2] = pressure % 100 / 10 + '0';
  pressure_str[3] = pressure % 100 % 10 + '0';
  _paint.DrawStringAt(0, 0, pressure_str, &Font12, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 80, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 0, "Temperature", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 60, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  int temp = (int)(_magnet->data.temperature * 100);
  char temperature_str[] = { '0', '0', '.', '0', '0', 'C', '\0' };
  temperature_str[0] = temp / 100 / 10 + '0';
  temperature_str[1] = temp / 100 % 10 + '0';
  temperature_str[3] = temp % 100 / 10 + '0';
  temperature_str[4] = temp % 100 % 10 + '0';
  _paint.DrawStringAt(0, 0, temperature_str, &Font12, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 40, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  _paint.DrawStringAt(0, 0, "Air Quality", &Font16, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 20, _paint.GetWidth(), _paint.GetHeight());

  _paint.Clear(UNCOLORED);
  int air_quality = (int)(_magnet->data.air_quality);
  char air_quality_str[] = { '0', '0', '\0' };
  air_quality_str[0] = air_quality % 100 / 10 + '0';
  air_quality_str[1] = air_quality % 100 % 10 + '0';
  _paint.DrawStringAt(0, 0, air_quality_str, &Font12, COLORED);
  _epd.SetFrameMemory_Partial(_paint.GetImage(), 0, 0, _paint.GetWidth(), _paint.GetHeight());

  _epd.DisplayFrame_Partial();
}
