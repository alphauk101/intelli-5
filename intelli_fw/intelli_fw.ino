#include <Wire.h>
#include "RTClib.h"
#include "defines.h"
#include "light_ctrl.h"

#define TEST_MODE


//Local app data
static INTELLI_DATA intel_data;


#define BUTTON_PIN    2 //our button pio

light_control lighting;
RTC_DS1307 rtc;

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Intelli 5 - debugging enabled");
#endif
  //We need to make sure our lighting is up and running before doing anything else
  lighting.init();

  //First init the RTC:
  setup_rtc();

  //Our button int handler
  pinMode(BUTTON_PIN, INPUT_PULLUP);
#ifndef TEST_MODE
  //disable during testing - primarily because we have no button attached
  //attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), pin_change_isr, CHANGE);
#endif




  //Make sure our application data is in the right state
  init_app_data();
  lighting.set_light_phase(&intel_data);
}

void loop() {

  //We need to check the time and return what time of the day it is
  intel_data.current_time = rtc.now();//update or time so its current
  //Once we have the time we need to check what lighting phase is required for this time
  get_day_phase();

  //intel_data.light_phase = HOUR_NIGHT_PHASE;
  lighting.set_light_phase(&intel_data);
  lighting.effect_shift_timer();
  //Now we have our phase we should pass it through to the lighting class.
#ifdef NO_BUTTON
  //Part of the main loop is checking whether the button has been pressed
  if (intel_data.button_press != NONE)
  {
    //The button has been pressed so do something then set it back
    if (intel_data.button_press == LONG) {
      //Set the time to a default of 8pm we dont care about the date
      rtc.adjust(DateTime(2017, 1, 1, 20, 0, 0));
    } else {
      //Its a short press we may consider an off option for certan occasions
      //Turn the lights off until next phase
      intel_data.light_phase = HOUR_OFF_PHASE;
    }

    intel_data.button_press = NONE;
  }
#endif
  delay(250);
#ifdef DEBUG
  DateTime now = rtc.now();
  Serial.print(now.year(), HEX);
  Serial.print('/');
  Serial.print(now.month(), HEX);
  Serial.print('/');
  Serial.print(now.day(), HEX);
  Serial.print(" (");
  Serial.print(now.dayOfTheWeek(),HEX);
  Serial.print(") ");
  Serial.print(now.hour(), HEX);
  Serial.print(':');
  Serial.print(now.minute(), HEX);
  Serial.print(':');
  Serial.print(now.second(), HEX);
  Serial.println();
#endif
}

void init_app_data()
{
  /*#define HOUR_DAY_PHASE    9
    #define HOUR_EVE_PHASE    18
    #define HOUR_NIGHT_PHASE  20
    #define HOUR_OFF_PHASE    0*/
  intel_data.current_time = rtc.now();//update or time so its current
  intel_data.light_phase = HOUR_EVE_PHASE;//Light phase is defaulted to off and then update when the code runs
  intel_data.button_press = NONE;
}

void pin_change_isr()
{
  //A long press dictates a time set
  uint16_t count = 0;
  while (digitalRead(BUTTON_PIN) == LOW) { //down to gnd when pressed
    //We know blocking in an ISR is bad but this is a simple program
    count++;
    delay(50);
  }

  if (count > LONG_PRESS_COUNT) {
    //We have performed a long press
    intel_data.button_press = LONG;
  } else {
    //we performed a short
    intel_data.button_press = SHORT;
  }
}

static void setup_rtc()
{
  Wire.begin();
  rtc.begin();


  if (! rtc.isrunning())
  {
    //If the clock is not running then this means something has gone wrong. Its batt backup so in theory once its up its good to go forever.
    //We should tell the user the clock is not set as they will be able to set the clock with a button hold.
    //In test mode always sets the time
#ifdef TEST_MODE
    //when in test mode actaul set the real time and date
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
#else
    lighting.show_error(ERROR_RTC_FAIL);
#endif
  }
}

static void get_day_phase()
{
  //We will simplily use hours at the moment to change phase
  //We olny want to action the change at the start of the hour, we do this to solve one simple issue
  //Pressing the button effectively cancels the current phase - we dont want the phase to then in turn be cancelled by this
  if (intel_data.current_time.minute() == 0) {
    switch (intel_data.current_time.hour())
    {
      case HOUR_DAY_PHASE:
        intel_data.light_phase = HOUR_DAY_PHASE;
        break;
      case HOUR_EVE_PHASE:
        intel_data.light_phase = HOUR_EVE_PHASE;
        break;
      case HOUR_NIGHT_PHASE:
        intel_data.light_phase = HOUR_NIGHT_PHASE;
        break;
      case HOUR_OFF_PHASE:
        intel_data.light_phase = HOUR_OFF_PHASE;
        break;
      default:
        //Do nothing as we dont want to change the phase until next phase shift - hehe
        break;
    }
  }/*This is not the 0 minute so do nothing*/


  //We need to use the minutes change the eve mode
  if ( (intel_data.current_time.minute() == 0) ||
       (intel_data.current_time.minute() == 15) ||
       (intel_data.current_time.minute() == 30) ||
       (intel_data.current_time.minute() == 45)) {
    lighting.effect_shift_timer();
  }

}

