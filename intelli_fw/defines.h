#ifndef __DEFINES__
#define __DEFINES__
#include "RTClib.h"

#define DAY_LED_BRIGHTNESS  50 //This is the brightness level set to ALL leds when in day mode


#define HOUR_DAY_PHASE    9/*This inidicates that the day phase starts at 9am*/
#define HOUR_EVE_PHASE    18/*eve pahase starts at 6pm*/
#define HOUR_NIGHT_PHASE  20 /*Night 8pm*/
#define HOUR_OFF_PHASE    0/*Off at midnight*/

typedef uint8_t BUTTON_PRESS;
typedef uint8_t LIGHT_PHASE;

#define SHORT 1
#define LONG  2
#define NONE  3


//Our application data structure
typedef struct {
  DateTime        current_time; //Current time

  LIGHT_PHASE     light_phase; //current light phase

  BUTTON_PRESS    button_press; //Last state of button
} INTELLI_DATA;


#endif
