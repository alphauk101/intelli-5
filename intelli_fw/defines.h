#ifndef __DEFINES__
#define __DEFINES__
#include "RTClib.h"

#define DEBUG

#define LONG_PRESS_COUNT    100 //how long we wait before considering a long press 100 = 5seconds (ish)

#define DAY_LED_BRIGHTNESS      35  //This is the brightness level set to ALL leds when in day mode
#define EVE_LED_BRIGHTNESS      200 //Evening brightness - I'm trying to avoid full power for heat/power reasons
#define NIGHT_LED_BRIGHTNESS    200 //The night brightness
#define OFF_LED_BRIGHTNESS      0   //off mode 

#define RGB_LED_INTENSITY       250 //Individual led intensity (not brightness) during none night modes
#define MAX_LED_INTENSITY       255
#define LOW_LED_INTENSITY       30
#define NIGHT_BLUE_INTENSITY    250
#define NIGHT_WHITE_INTENSITY   40  // The red and green led intens during night 

/*Rose*/
#define ROSE_RED                255
#define ROSE_BLUE               50
#define ROSE_GREEN              50
/*SKY*/
#define SKY_RED                 110
#define SKY_GREEN               110
#define SKY_BLUE                255
/*GREEN*/
#define GREEN_RED               50
#define GREEN_GREEN             255
#define GREEN_BLUE              50

#define TRANS_SPEED_MS      50 //Time of steps during transistion

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
