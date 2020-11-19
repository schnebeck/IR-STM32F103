#ifndef LEDMODE_H
#define LEDMODE_H

#include "Arduino.h"
#include <Wire.h>

class LEDMode
{
public:
  enum blink_t
  {
    off = 0,
    on = 1,
    on50Off50 = 2,
    on20Off80 = 3,
    on20Off20On20Off60 = 4,
  };

  LEDMode(uint8_t ledPin, uint8_t mode = blink_t::off);
  void newLED(uint8_t ledPin, uint8_t mode = blink_t::off);
  void mode(uint8_t mode);
  void ctrl(void);

private:
  enum led_t
  {
    LEDon  = 1,
    LEDoff = 0,
  };

  uint8_t _ledPin;
  uint8_t _mode;
  bool _on;
  unsigned long _now;
};
#endif