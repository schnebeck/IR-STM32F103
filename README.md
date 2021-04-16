# IR-STM32F103 (PlatformIO repository for STM32/Arduino)

Switching your Raspberry PI on and off by key or IR remote control
==================================================================

A simple BluePill setup (https://www.amazon.com/CANADUINO-STM32-Original-STM32F103C8T6-Correct/dp/B07RSTMMDL/ ) to control a raspberry pi with an IR remote control
Setup utilies some pins:

#define RASPI_OFF PB11
Input, a script on the Raspberry Pi sets this PIN to LOW when running.

#define KEY_SLEEP PB0
Input, simple ON/OFF key switch as an alternative to the remote control. Long pressing key starts learning ON/OFF IR pulse.

#define IR_SIGNAL PC15
Input, IR pulses frome the IR diode (here: Vishay TSOP4838). This controller only reacts to the learned ON/OFF pulses. All other IR pulses are wire copied to the raspberry pi and utilized using ir remote overlay

#define LED PB1
Output, LED feedback: Off when RASPI_OFF is High, On when RASPI_OFF is LOW, slow blinking during shutdown, fast blinkiung when learning ON/OFF IR pulse 

#define RASPI_SHUTDOWN PB10
Output, sends start/stop signal to the raspberry pi


Learning IR Code
================
When pessing KEY_SLEEP for more than 3 seconds the controller enters a code learning mode.
Here you have to press two times the on/off key on your remote control to learn the IR code. The code is stored in flash and can be re-programed.
