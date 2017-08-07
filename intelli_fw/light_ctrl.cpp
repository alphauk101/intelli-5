#include "light_ctrl.h"

//We use this to determine what the last phase was primarily so we know when to transist and when not to do anything
static LIGHT_PHASE _last_phase;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);

void light_control::init()
{
  strip.begin(); // This initializes the NeoPixel library.
  //On init we may want to makesure the LEDs are in a good state

  //To show the unit is init'ing we do a little chase:
  rainbowCycle(5);
  colorWipe(strip.Color(0, 0, 0), 50); // Off

  _last_phase = HOUR_OFF_PHASE;//They are off
}

void light_control::set_light_phase(INTELLI_DATA * light_data_ptr)
{
  bool trans = false;
  //First thing we do is check whether we are changing phase
  /*
    if (light_data_ptr->light_phase != _last_phase)
    {
    //Its time to transpose the lighting
    fade_current_lights();
    //Now we need to go to whichever lighting is next
    trans = true; // we do a transistion
    //Lastly update of current light state
    _last_phase = light_data_ptr->light_phase;
    } else {
    trans = false;
    }
  */
  //now set the appropriate mode
  /*
    switch (light_data_ptr->light_phase)
    {
    case HOUR_DAY_PHASE:
      set_day_mode(false);
      break;
    case HOUR_EVE_PHASE:

      break;
    case HOUR_NIGHT_PHASE:
      break;
    case HOUR_OFF_PHASE:
      break;
    default:
      break;
    }
  */
  set_day_mode(false);
}

/**build to day mode
  pass true to the function causes a transistion**/
void light_control::set_day_mode(bool trans)
{
  uint8_t b;
  if(trans){
    b = 0;
  }else{
    b = DAY_LED_BRIGHTNESS-1;
  }
  for (b; b < DAY_LED_BRIGHTNESS; b++) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(255, 0, 0));
      i++;
      strip.setPixelColor(i, strip.Color(0, 255, 0));
      i++;
      strip.setPixelColor(i, strip.Color(0, 0, 255));
      
    }
    strip.setBrightness(b);
    strip.show();
    delay(10);
  }


  //Now set all the leds to the appropraite brighntess
  //(strip.Color(155, 155, 155), 1);
}



/*Fades the current lights down to nothings*/
void light_control::fade_current_lights()
{
  uint8_t bright = strip.getBrightness();
  while (bright > 0)
  {
    bright--;
    strip.setBrightness(bright);
    delay(30);
  }

}



void light_control::show_error(byte error_code)
{
  switch (error_code)
  {
    case ERROR_RTC_FAIL:
      colorWipe(strip.Color(255, 0, 0), 50); // Red
      break;
    default:
      break;

  }
}

// Fill the dots one after the other with a color
void light_control::colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void light_control::rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t light_control::Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

