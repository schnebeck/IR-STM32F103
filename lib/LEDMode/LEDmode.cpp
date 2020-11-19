#include "LEDMode.h"

LEDMode::LEDMode(uint8_t ledPin, uint8_t mode)
{
  _ledPin = ledPin;
  _mode = mode;
  _now = millis();
}

void LEDMode::newLED(uint8_t ledPin, uint8_t mode)
{
  _ledPin = ledPin;
  _mode = mode;
}

void LEDMode::mode(uint8_t mode)
{
  _mode = mode;
}

void LEDMode::ctrl(void)
{
  ulong delta;
  if ((millis() - _now) > 1000)
    _now = millis();
  delta = millis() - _now;

  switch (_mode)
  {
  case blink_t::off:
    _on = led_t::LEDoff;
    break;

  case blink_t::on:
    _on = led_t::LEDon;
    break;

  case blink_t::on50Off50:
    if (delta < 500)
    {
      _on = led_t::LEDon;
    }
    else
    {
      _on = led_t::LEDoff;
    }
    break;

  case blink_t::on20Off80:
    if (delta < 200)
    {
      _on = led_t::LEDon;
    }
    else
    {
      _on = led_t::LEDoff;
    }
    break;

  case blink_t::on20Off20On20Off60:
    if (delta < 200)
    {
      _on = led_t::LEDon;
    }
    else if (delta >= 200 && delta < 400)
    {
      _on = led_t::LEDoff;
    }
    else if (delta >= 400 && delta < 600)
    {
      _on = led_t::LEDon;
    }
    else if (delta >= 600)
    {
      _on = led_t::LEDoff;
    }
    break;
  }
  digitalWrite(_ledPin, _on);
}