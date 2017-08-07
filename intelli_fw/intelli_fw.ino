#include <Wire.h>
#include "RTClib.h"
#include "defines.h"
#include "light_ctrl.h"

//Local app data
static INTELLI_DATA intel_data;


#define LONG_PRESS_COUNT    1000 //how long we wait before considering a long press
#define BUTTON_PIN    2 //our button pio

light_control lighting;
RTC_DS1307 rtc;

void setup() {

  //We need to make sure our lighting is up and running before doing anything else
  lighting.init();

  //First init the RTC:
  setup_rtc();

  //Our button int handler
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), pin_change_isr, CHANGE);


  //Make sure our application data is in the right state
  init_app_data();
}

void loop() {

  //We need to check the time and return what time of the day it is
  intel_data.current_time = rtc.now();//update or time so its current
  //Once we have the time we need to check what lighting phase is required for this time
  get_day_phase();

  //Now we have our phase we should pass it through to the lighting class.



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
}

void init_app_data()
{
  intel_data.current_time = rtc.now();//update or time so its current
  intel_data.light_phase = HOUR_OFF_PHASE;//Light phase is defaulted to off and then update when the code runs
  intel_data.button_press = NONE;
}

void pin_change_isr()
{
  //A long press dictates a time set
  uint16_t count = 0;
  while (digitalRead(BUTTON_PIN) == LOW) { //down to gnd when pressed
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
    lighting.show_error(ERROR_RTC_FAIL);
  }
}

static void get_day_phase()
{
  //We will simplily use hours at the moment to change phase
  switch (intel_data.current_time.hour())
  {
    case HOUR_DAY_PHASE:
      //its 9 in the morning
      intel_data.light_phase = HOUR_DAY_PHASE;
      break;
    case HOUR_EVE_PHASE:
      //its 9 in the morning
      intel_data.light_phase = HOUR_EVE_PHASE;
      break;
    case HOUR_NIGHT_PHASE:
      //its 9 in the morning
      intel_data.light_phase = HOUR_NIGHT_PHASE;
      break;
    case HOUR_OFF_PHASE:
      //its 9 in the morning
      intel_data.light_phase = HOUR_OFF_PHASE;
      break;
      default:
      //Do nothing as we dont want to change the phase until next phase shift - hehe
      break;
  }
}
