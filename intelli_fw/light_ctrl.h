#ifndef __LIGHTING_H__
#define __LIGHTING_H__
#include <arduino.h>
#include "defines.h"
#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
  #include <avr/power.h>
#endif



#define LED_DATA_PIN      5
#define NUMPIXELS         50 /*Even though we have more the blocks of 50 are parrelled up*/

#define ERROR_RTC_FAIL    0xE1

class light_control
{
  public:
  void init();
  void show_error(byte);
  void set_light_phase(INTELLI_DATA *);
  private:
  //We use this to determine what the last phase was primarily so we know when to transist and when not to do anything
  LIGHT_PHASE _last_phase;
  void set_night_mode(bool);
  void fade_current_lights(void);
  void rainbowCycle(uint8_t);
  uint32_t Wheel(byte);
  void colorWipe(uint32_t , uint8_t);
  void set_day_mode(bool);
  void set_eve_mode(bool);
  void set_off_mode(bool);
  void set_rgb_level(uint8_t , bool );
  void set_night_step(uint8_t);
  
};



#endif /*__LIGHTING_H__*/
