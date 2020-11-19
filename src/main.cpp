
#define LED PB1
#define KEY_SLEEP PB0
#define RASPI_SHUTDOWN PB10
#define RASPI_OFF PB11
#define IR_SIGNAL PC15
#define IRMP_INPUT_PIN IR_SIGNAL
#define IRMP_PROTOCOL_NAMES 1

#include "Arduino.h"
#include <EEPROM.h>
#include "LEDMode.h"
#include <irmpSelectMain15Protocols.h>
#include <irmp.c.h>

IRMP_DATA irmp_data;
IRMP_DATA ir_on;
IRMP_DATA ir_off;
uint8_t ir_on_adr = 2;
uint8_t ir_off_adr = 2 + sizeof(IRMP_DATA);

bool startShutDown = false;
uint8_t statemachine = 0;
unsigned long keyPressTime;
unsigned long now;

LEDMode statusLED(LED, LEDMode::on50Off50);

void setup()
{
  delay(100);
  Serial.begin(115200);
  Serial.println("Startup");
  Serial.println("Raspi IR-PowerCtrl V1.0");
  Serial.println("(C) 2020 by Thorsten Schnebeck");

  pinMode(LED, OUTPUT);
  pinMode(RASPI_SHUTDOWN, OUTPUT);
  pinMode(RASPI_OFF, INPUT);
  pinMode(KEY_SLEEP, INPUT_PULLUP);
  pinMode(IR_SIGNAL, INPUT);
  digitalWrite(RASPI_SHUTDOWN, HIGH);
  irmp_init();
  if (EEPROM.read(0) != 0x55 || EEPROM.read(1) != 0xaa)
  {
    EEPROM.write(0, 0x55);
    EEPROM.write(1, 0xaa);
    Serial.println("Write EEPROM!");
  }
  else
  {
    Serial.println("Do not write EEPROM!");
    EEPROM.get(ir_on_adr, ir_on);
    EEPROM.get(ir_off_adr, ir_off);
  }
  Serial.print(F("Ready to receive IR signals of protocols: "));
  irmp_print_active_protocols(&Serial);
}

void loop()
{
  statusLED.ctrl();
  //
  //  S T A T E M A C H I N E
  //

  switch (statemachine)
  {
  case 0:
    // Startup
    Serial.print("Expect ");
    Serial.print("0xaa55");
    Serial.print(", Read ");
    Serial.print(EEPROM.read(0) + (EEPROM.read(1) << 8), HEX);
    Serial.print(", ir_on ");
    Serial.print(ir_on.command, HEX);
    Serial.print(", ir_off ");
    Serial.print(ir_off.command, HEX);
    Serial.println();
    statemachine = 10;
    break;

  case 10:
    // Idle
    if (digitalRead(RASPI_OFF))
      statusLED.mode(LEDMode::off);
    else
      statusLED.mode(LEDMode::on);

    if (digitalRead(KEY_SLEEP) == 0)
    {
      delay(10);
      if (digitalRead(KEY_SLEEP) == 0)
      {
        keyPressTime = millis();
        Serial.println("Key Pressed");
        statemachine = 20;
      }
    }
    if (irmp_get_data(&irmp_data))
    {
      if (!(irmp_data.flags & IRMP_FLAG_REPETITION))
      {
        if (ir_on.protocol == irmp_data.protocol &&
            ir_on.address == irmp_data.address &&
            ir_on.command == irmp_data.command)
        {
          statemachine = 11;
        }
        if (ir_off.protocol == irmp_data.protocol &&
            ir_off.address == irmp_data.address &&
            ir_off.command == irmp_data.command)
        {
          statemachine = 11;
        }
        irmp_result_print(&irmp_data);
      }
    }
    break;

  case 11:
    Serial.println("PowerKey pressed");
    statemachine = 80;
    break;

  case 20:
    // Keypressed
    statusLED.mode(LEDMode::on50Off50);
    if ((millis() - keyPressTime) < 3000)
    {
      if (digitalRead(KEY_SLEEP) == 1)
      {
        delay(10);
        if (digitalRead(KEY_SLEEP) == 1)
        {
          Serial.println("short Key press");
          statemachine = 80;
        }
      }
    }
    else
    {
      statemachine = 30;
    }
    break;

  case 30: // longpress: learn
    statusLED.mode(LEDMode::on20Off80);
    if (digitalRead(KEY_SLEEP) == 1)
    {
      delay(10);
      if (digitalRead(KEY_SLEEP) == 1)
      {
        Serial.println("long key press");
        keyPressTime = millis();
        statemachine = 32;
      }
    }
    break;

  case 32:
    if (irmp_get_data(&irmp_data))
    {
      if (!(irmp_data.flags & IRMP_FLAG_REPETITION))
      {
        ir_on = irmp_data;
        EEPROM.put(ir_on_adr, ir_on);
        Serial.println("Learned Key 1");
        statemachine = 33;
      }
    }
    if (millis() - keyPressTime > 10000)
    {
      statemachine = 35;
    }
    break;

  case 33:
    statusLED.mode(LEDMode::on20Off20On20Off60);
    if (!irmp_get_data(&irmp_data))
      statemachine = 34;
    break;

  case 34:
    if (irmp_get_data(&irmp_data))
    {
      if (!(irmp_data.flags & IRMP_FLAG_REPETITION))
      {
        ir_off = irmp_data;
        EEPROM.put(ir_off_adr, ir_off);
        Serial.println("Learned Key 2");
        statemachine = 35;
      }
    }

    if (millis() - keyPressTime > 10000)
    {
      statemachine = 35;
    }
    break;

  case 35:
    delay(333);
    statusLED.mode(LEDMode::off);
    if (!irmp_get_data(&irmp_data))
      statemachine = 36;
    break;

  case 36:
    // store into eeprom
    Serial.println("Stop key learning");
    statemachine = 10;
    break;

  case 80:
    // On/Off-Pulse
    statusLED.mode(LEDMode::on50Off50);
    if (digitalRead(RASPI_OFF))
    {
      Serial.println("Power On");
      statemachine = 90;
    }
    else
    {
      Serial.println("Power Off");
      statemachine = 81;
    }
    break;

  case 81:
    digitalWrite(RASPI_SHUTDOWN, LOW);
    now = millis();
    statemachine = 82;
    break;

  case 82:
    if (millis() - now > 3000)
    {
      Serial.println("Send Stop Pulse");
      statemachine = 83;
    }
    break;

  case 83:
    digitalWrite(RASPI_SHUTDOWN, HIGH);
    statemachine = 84;
    break;

  case 84:
    if (digitalRead(RASPI_OFF))
      statemachine = 10;
    break;

  case 90:
    digitalWrite(RASPI_SHUTDOWN, LOW);
    delay(3000);
    digitalWrite(RASPI_SHUTDOWN, HIGH);
    Serial.println("Send Start Pulse");
    statemachine = 10;
    break;

  default:
    statemachine = 0;
    break;
  }
}
