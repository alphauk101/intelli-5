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

static LED_MATRIX g_led_matrix[NUMPIXELS];

void light_control::init()
{
#ifdef DEBUG
  Serial.println("Lighting starting");
#endif
  strip.begin(); // This initializes the NeoPixel library.
  //On init we may want to makesure the LEDs are in a good state

  //To show the unit is init'ing we do a little chase:
  rainbowCycle(5);
  colorWipe(strip.Color(0, 0, 0), 20); // Off

  _last_phase = HOUR_OFF_PHASE;//They are off
  //Set this to the standard effect on boot
  eve_effect = STANDARD;
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
EVE_EFFECT prev_effect;
void light_control::set_eve_mode(bool trans)
{
  if (trans) {
    //If we are transisting modes then we should do it
    this->set_rgb_level(EVE_LED_BRIGHTNESS, trans);
    //We need to make sure we are set to standard so we dont get mixed up
    eve_effect = STANDARD;
    prev_effect = STANDARD;
  } else {
    //We can change our style if we wish
    this->set_effect(prev_effect, eve_effect);
    prev_effect = eve_effect;



    //this->set_red_tint();
  }
}

void light_control::set_effect(EVE_EFFECT old_effect, EVE_EFFECT new_effect)
{
  /*
     #define STANDARD  0
    #define ROSE      1
    #define SKY       2
    #define GREEN     3
  */
  LED_MATRIX new_color;
  switch (new_effect)
  {
    case STANDARD:
      //new_color = strip.Color(RGB_LED_INTENSITY, RGB_LED_INTENSITY, RGB_LED_INTENSITY);
      new_color.red = MAX_LED_INTENSITY;
      new_color.green = MAX_LED_INTENSITY;
      new_color.blue = MAX_LED_INTENSITY;
      break;
    case ROSE:
      new_color.red =   ROSE_RED;
      new_color.green = ROSE_GREEN;
      new_color.blue =  ROSE_BLUE;
      break;
    case SKY:
      new_color.red =   SKY_RED;
      new_color.green = SKY_GREEN;
      new_color.blue =  SKY_BLUE;
      break;
    case GREEN:
      new_color.red =   GREEN_RED;
      new_color.green = GREEN_GREEN;
      new_color.blue =  GREEN_BLUE;
      break;
    default:
      new_color.red = MAX_LED_INTENSITY;
      new_color.green = MAX_LED_INTENSITY;
      new_color.blue = MAX_LED_INTENSITY;
      break;
  }

  if (new_effect != old_effect) {
    bool looped = true;
    uint32_t update_color = 0;
    while (looped) {
      looped = false;
      for (uint16_t i = 0; i < strip.numPixels(); i++) {
        //i = strip.getPixelColor(i);
        if (update_pixel_transistion(new_color, i) == false)
        {
          /*So if the tranaisition is not finished we should do loops
            until it is finished*/
          looped = true;
        }
      }
      strip.setBrightness(EVE_LED_BRIGHTNESS);
      strip.show();
      delay(TRANS_SPEED_MS);
    }
    /*
      uint32_t new_c = 0;
      switch (new_effect)
      {
      case STANDARD:
        new_c = strip.Color(RGB_LED_INTENSITY, RGB_LED_INTENSITY, RGB_LED_INTENSITY);
        break;
      case ROSE:
        new_c = strip.Color(RGB_LED_INTENSITY, 0, 0);
        break;
      case SKY:
        new_c = strip.Color(RGB_LED_INTENSITY, 150, 0);
        break;
      case GREEN:
        new_c = strip.Color(0, RGB_LED_INTENSITY, 0);
        break;
      default:
        new_c = strip.Color(RGB_LED_INTENSITY, RGB_LED_INTENSITY, RGB_LED_INTENSITY);
        break;
      }
      bool looped = true;
      uint32_t tmp_c = 0;
      while (looped) {
      looped = false;;
      for (uint16_t i = 0; i < strip.numPixels(); i++) {
        if ((i % 2) == 0) {
          tmp_c = strip.getPixelColor(i);
          if (tmp_c > new_c) {
            tmp_c--;
            looped = true;
          } else if (tmp_c < new_c) {
            tmp_c++;
            looped = true;
          }
          strip.setPixelColor(i, tmp_c);
        } else {
          //Its an odd pixel so make it default white
          strip.setPixelColor(i, strip.Color(RGB_LED_INTENSITY, RGB_LED_INTENSITY, RGB_LED_INTENSITY));
        }
      }
      strip.setBrightness(EVE_LED_BRIGHTNESS);
      strip.show();
      delay(50);
      }
    */
  }/*If its the old same effect dont do anything*/
}

/*Takes the current light value of the selected pixel compares it to the new colour trans and
  makes it one gradient closer to that colour
  returns a boolean if the transistion is complete*/
bool light_control::update_pixel_transistion(LED_MATRIX new_c, uint16_t pix)
{
  LED_MATRIX old_pixel = g_led_matrix[pix];
  LED_MATRIX apply_pixel;

  apply_pixel.red = this->compare_pix_ammend(old_pixel.red, new_c.red);
  apply_pixel.green = this->compare_pix_ammend(old_pixel.green, new_c.green);
  apply_pixel.blue = this->compare_pix_ammend(old_pixel.blue, new_c.blue);
/*
  Serial.print("Old Red: ");
  Serial.println(old_pixel.red);
  Serial.print("New Red: ");
  Serial.println(new_c.red);
  Serial.print("Trans red: ");
  Serial.println(apply_pixel.red);
  Serial.println("---------------------------------------");

  Serial.print("P: ");
  Serial.println(pix);
  Serial.print("Old Blue: ");
  Serial.println(old_pixel.blue);
  Serial.print("New Blue: ");
  Serial.println(new_c.blue);
  Serial.print("Trans Blue: ");
  Serial.println(apply_pixel.blue);
  Serial.println("---------------------------------------");
*/

  for (int a = 0; a < NUMPIXELS; a++)
  {
    g_led_matrix[a].red = apply_pixel.red;
    g_led_matrix[a].green = apply_pixel.green;
    g_led_matrix[a].blue = apply_pixel.blue;
  }
  this->apply_matrix();

  if ((new_c.red == apply_pixel.red) &&
      (new_c.green == apply_pixel.green) &&
      (new_c.blue == apply_pixel.blue))
  {
    //If they are the same then we have completed our trans
    return true;
  } else {
    return false;
  }

  /*
    uint32_t trans_color = trans_c;//Make this ther trans color and then if something goes wrong we dont end up with off
    new_c = &trans_color;
    bool flag = true; //set to positive and change on otherwise.

    uint32_t old_color = strip.getPixelColor(pix);
    //First before we do any operations we should check that the colours are not currently the same
    if (old_color != trans_c) {
    uint8_t old_red = (uint8_t)((uint32_t) old_color >> 16);
    uint8_t old_green = (uint8_t)((uint32_t)old_color >> 8);
    uint8_t old_blue = (uint8_t) old_color;

    //We have our original colours now get the trans colors
    uint8_t trans_red = (uint8_t)((uint32_t) trans_c >> 16);
    uint8_t trans_green = (uint8_t)((uint32_t) trans_c >> 8);
    uint8_t trans_blue = (uint8_t)((uint32_t) trans_c);

    //Now do the comparison
    uint8_t tmp_red = this->compare_pix_ammend(old_red, trans_red);
    if (tmp_red != 0) flag = false;

    uint8_t tmp_green = this->compare_pix_ammend(old_green, trans_green);
    if (tmp_green != 0) flag = false;

    uint8_t tmp_blue = this->compare_pix_ammend(old_blue, trans_blue);
    if (tmp_blue != 0) flag = false;

    trans_color = strip.Color(tmp_red, tmp_green, tmp_blue);
    return flag;
    } else {
    flag = true;//trans is complete.
    }
    //return result
    return flag;
  */
}

uint8_t light_control::compare_pix_ammend(uint8_t old_col, uint8_t new_col)
{
  uint8_t apply = old_col;
  if (old_col > new_col) {
    apply = (old_col-1);
    //new_col++;
  } else if (old_col < new_col) {
    apply = (old_col+1);
    //new_col--;
  } else {
    //value stay the same
  }
  return apply;
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

void light_control::apply_matrix()
{
  uint8_t row = 0;
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    /*
    strip.setPixelColor(i, strip.Color(g_led_matrix[row].red , 0, 0));
    i++;
    strip.setPixelColor(i, strip.Color(0, g_led_matrix[row].green, 0));
    i++;
    strip.setPixelColor(i, strip.Color(0, 0, g_led_matrix[row].blue));
    */
    strip.setPixelColor(i, strip.Color(g_led_matrix[row].red , g_led_matrix[row].green, g_led_matrix[row].blue));
    if ((i != 0) && ((i % 10) == 0))row++;
  }
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
  for (int a = 0 ; a < strip.numPixels(); a++) {
    g_led_matrix[a].red = MAX_LED_INTENSITY;
    g_led_matrix[a].green = MAX_LED_INTENSITY;
    g_led_matrix[a].blue = MAX_LED_INTENSITY;
  }

  if (trans) {
    b = 0;
  } else {
    b = level;
  }
  for (b; b <= level; b++) {
    apply_matrix();
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

//Used to change the evening effect driven by the RTC
void light_control::effect_shift_timer()
{
  eve_effect = random(0, 5);
  /*
  eve_effect++;
  if (eve_effect > GREEN) eve_effect = STANDARD;
  */
}

