#include "light_ctrl.h"



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
  colorWipe(strip.Color(0, 0, 0), 20); // Off

  _last_phase = HOUR_OFF_PHASE;//They are off
}

void light_control::set_light_phase(INTELLI_DATA * light_data_ptr)
{
  bool trans = false;
  //First thing we do is check whether we are changing phase

  if (light_data_ptr->light_phase != this->_last_phase)
  {
    //Its time to transpose the lighting
    fade_current_lights();
    //Now we need to go to whichever lighting is next
    trans = true; // we do a transistion
    //Lastly update of current light state
    this->_last_phase = light_data_ptr->light_phase;
  } else {
    trans = false;
  }

  //now set the appropriate mode

  switch (light_data_ptr->light_phase)
  {
    case HOUR_DAY_PHASE:
      this->set_day_mode(trans);
      break;
    case HOUR_EVE_PHASE:
      this->set_eve_mode(trans);
      break;
    case HOUR_NIGHT_PHASE:
      this->set_night_mode(trans);
      break;
    case HOUR_OFF_PHASE:
      this->set_off_mode(trans);
      break;
    default:
      break;
  }
}

void light_control::set_night_mode(bool trans) {
  //If we are trans'ing then we need to fade up to out night mode problem is our rg lights are duller than the b lights
  //For now (and this will change) we will fade them up indepedantly
  if (trans) { //were transitioning
    for (int i = 0 ; i < NIGHT_LED_BRIGHTNESS; i++) {
      /*For each step of b led we are increasing the brightness*/
      this->set_night_step(i);
      delay(TRANS_SPEED_MS);
    }
  } else {
    //Were not transitioning so just set lights to full brightness
    this->set_night_step(NIGHT_LED_BRIGHTNESS);
  }
}

/*Allows all leds to be set in night mode at the given brightness*/
void light_control::set_night_step(uint8_t brightness)
{
  for (uint8_t j = 0; j < strip.numPixels(); j++) {
    strip.setPixelColor(j, strip.Color(NIGHT_WHITE_INTENSITY, NIGHT_WHITE_INTENSITY, NIGHT_WHITE_INTENSITY));
    j++;
    strip.setPixelColor(j, strip.Color(0, 0, NIGHT_BLUE_INTENSITY));
    j++;
    strip.setPixelColor(j, strip.Color(0, 0, NIGHT_BLUE_INTENSITY));
  }
  //Once all the leds have been stored we can update the display
  strip.setBrightness(brightness);
  strip.show();
}

/*set day mode
  pass true to the function causes a transistion**/
void light_control::set_day_mode(bool trans)
{
  this->set_rgb_level(DAY_LED_BRIGHTNESS, trans);
}

/*set eve mode*/
void light_control::set_eve_mode(bool trans)
{
  if (trans) {
    //If we are transisting modes then we should do it
    this->set_rgb_level(EVE_LED_BRIGHTNESS, trans);
  } else {
    //We can change our style if we wish
    this->set_red_tint();
  }
}

void light_control::set_red_tint()
{
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
    i++;
    strip.setPixelColor(i, strip.Color(255, 0, 0));
    i++;
    strip.setPixelColor(i, strip.Color(RGB_LED_INTENSITY, RGB_LED_INTENSITY, RGB_LED_INTENSITY));
  }
  strip.setBrightness(EVE_LED_BRIGHTNESS);
  strip.show();
}

/*set off mode*/
void light_control::set_off_mode(bool trans)
{
  this->set_rgb_level(OFF_LED_BRIGHTNESS, trans);
}


/*This allows for this action to set from different functions*/
uint8_t b;
void light_control::set_rgb_level(uint8_t level, bool trans)
{
  /*
    If we choose to not have a transistion then the loop only goes round once ie. setting b to the loop exit value -1
    this will have the action of setting the lights once.
    If we transist this will swell the lights up slowly.
    This is not necessarily a good transistion but its a start.
  */

  if (trans) {
    b = 0;
  } else {
    b = level;
  }
  for (b; b <= level; b++) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(RGB_LED_INTENSITY, 0, 0));
      i++;
      strip.setPixelColor(i, strip.Color(0, RGB_LED_INTENSITY, 0));
      i++;
      strip.setPixelColor(i, strip.Color(0, 0, RGB_LED_INTENSITY));

    }
    strip.setBrightness(b);
    strip.show();
    delay(TRANS_SPEED_MS);
  }
}



/*Fades the current lights down to nothings*/
void light_control::fade_current_lights()
{
  uint8_t bright = strip.getBrightness();
  while (bright > 0)
  {
    bright--;
    strip.setBrightness(bright);
    strip.show();
    delay(TRANS_SPEED_MS);
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

  for (j = 0; j < 256 * 2; j++) { // 5 cycles of all colors on wheel
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

