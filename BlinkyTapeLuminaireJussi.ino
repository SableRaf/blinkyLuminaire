// JUSSI'S lighting design with slow colour sweep, gradient for static light and a party mode.
// 160325

#include <FastLED.h>
#include <Button.h> // http://github.com/virgildisgr4ce/Button

#define LED_COUNT 60
struct CRGB leds[LED_COUNT];

#define LED_OUT      13
#define BUTTON_IN    10
#define ANALOG_INPUT A9
#define IO_A         7
#define IO_B         11

#define COLOR_MODE    0
#define POWERUP_MODE  1
#define LIGHTON_MODE    2
#define DIM_MODE 3
#define PARTY_MODE 5
#define OFF_MODE 4


uint8_t draw_mode = COLOR_MODE;
//uint8_t draw_mode = PARTY_MODE;

uint8_t pixel_index;
long last_time;
long powerup_start;
long powerup_length = 2500;
long cooldown_start;
long cooldown_length;
boolean party_on = false;

long delayTime = 10;
long delayCounts = 10;

uint8_t brightness = random(0, 128);
uint8_t red =   brightness + random(0, 64);
uint8_t green = brightness + random(0, 64);
uint8_t blue =  brightness + random(0, 64);
long fader = 0;


Button button = Button(BUTTON_IN, BUTTON_PULLUP_INTERNAL, true, 50);

void onPress(Button& b) {

  if (draw_mode == COLOR_MODE) {
    powerup_start = millis();
    draw_mode = POWERUP_MODE;
  }

  if (draw_mode == OFF_MODE) {
    draw_mode = COLOR_MODE;
  }
  if (draw_mode == PARTY_MODE) {
    draw_mode = OFF_MODE;
  }
  if (draw_mode == DIM_MODE) {
    draw_mode = PARTY_MODE;

  }

}

void onRelease(Button& b) {
  if (draw_mode == POWERUP_MODE) {
    cooldown_start = millis();
    cooldown_length = cooldown_start - powerup_start;
    draw_mode = DIM_MODE;
  }



}

void onHold(Button& b) {
  //  if(draw_mode == PARTY_MODE)
  //    draw_mode = COLOR_MODE;
}

void setup()
{
  LEDS.addLeds<WS2812B, LED_OUT, GRB>(leds, LED_COUNT);
  LEDS.showColor(CRGB(0, 0, 0));
  LEDS.setBrightness(93); // Limit max current draw to 1A
  LEDS.show();

  Serial.begin(57600);

  last_time = millis();

  button.pressHandler(onPress);
  button.releaseHandler(onRelease);
  button.holdHandler(onHold, 1000); // must be held for at least 1000 ms to trigger
}


uint8_t i = 0;
int j = 0;
int f = 0;
int k = 0;

int count;

void color_loop() {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    uint8_t red =   64 * (1 + sin(i / 20.0 + j / 240.0 )) + i / 3;
    uint8_t green = 62 * (1 + sin(i / 20.0 + j / 240.0 )) + i / 7.2;
    uint8_t blue =  48 * (1 + sin(i / 20.0 + f / 240.0 ));

    leds[i] = CRGB(red, green, blue);

    if ((millis() - last_time > 1005) && pixel_index <= LED_COUNT + 1) {
      last_time = millis();
      count = LED_COUNT - pixel_index;
      pixel_index++;
    }
  }
  LEDS.show();
  j = j + random(0, 2);
  f = f + random(0, 2);
  k = k + random(0, 3);
}


void powerup_loop() {
  long elapsed = millis() - powerup_start;
  if (elapsed < powerup_length) {
    draw_progress(elapsed, powerup_length);
  }
}

void cooldown_loop() {
  /*
    long elapsed = millis() - cooldown_start;
    if(elapsed > cooldown_length) {
      draw_mode = COLOR_MODE;
      last_time =  millis() ;
    } else {
      draw_progress(cooldown_length - elapsed, powerup_length);
    }
  */
}

void party_mode() {

  if (millis() > last_time + delayTime * delayCounts) {
    brightness = random(0, 128);
    red =   brightness + random(0, 64);
    green = brightness + random(0, 64);
    blue =  brightness + random(0, 64);
    if (delayCounts > 300 / delayTime) {
      delayTime = random(10, 100);
      delayCounts = 0;
      fader = random(0, 2);
    }
    delayCounts++;

    for (int i = 0; i < LED_COUNT; i++) {
      leds[i] = CRGB(red - i, green - i, blue - i);
    }
    last_time = millis();
    LEDS.show();
  }
  //fade out slowly (if the fader is greater than 0)
  red -= fader;
  green -= fader;
  blue -= fader;
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = CRGB(red - i, green - i, blue - i);
  }
  LEDS.show();

  if (delayTime > 96) {
    for (int i = 0; i < 10; i++) { //blink 100 times

      for (int i = 0; i < LED_COUNT; i++) {
        leds[i] = CRGB(255,255,255);
      }
      LEDS.show();
      delay(5);
      for (int i = 0; i < LED_COUNT; i++) {
        leds[i] = CRGB(0,0,0);
      }
      LEDS.show();
      delay(50);

    }
  }
}

void draw_progress(long elapsed, long powerup_length) {
  float progress = (float) elapsed / (float) powerup_length;
  int count = LED_COUNT - (int)((float)LED_COUNT * progress);
  for (int x = 0; x < count; x++) {
    leds[x] = CRGB(0, 0, 0);
  }
  for (int x = count + 1; x < LED_COUNT; x++) {
    leds[x] = CRGB(255, 205, 80);
  }
  LEDS.show();
}



void off() {
  for (int x = 0; x < LED_COUNT; x++) {
    leds[x] = CRGB(0, 0, 0);
  }
  LEDS.show();

}

void serialLoop() {
  static int pixelIndex;

  unsigned long lastReceiveTime = millis();

  while (true) {

    if (Serial.available() > 2) {
      lastReceiveTime = millis();

      uint8_t buffer[3]; // Buffer to store three incoming bytes used to compile a single LED color

      for (uint8_t x = 0; x < 3; x++) { // Read three incoming bytes
        uint8_t c = Serial.read();

        if (c < 255) {
          buffer[x] = c; // Using 255 as a latch semaphore
        }
        else {
          LEDS.show();
          pixelIndex = 0;
          break;
        }

        if (x == 2) {   // If we received three serial bytes
          if (pixelIndex == LED_COUNT) break; // Prevent overflow by ignoring the pixel data beyond LED_COUNT
          leds[pixelIndex] = CRGB(buffer[0], buffer[1], buffer[2]);
          pixelIndex++;
        }
      }
    }

    // If we haven't received data in 4 seconds, return to playing back our animation
    if (millis() > lastReceiveTime + 4000) {
      // TODO: Somehow the serial port gets trashed here, how to reset it?
      return;
    }
  }
}

void loop()
{
  // If'n we get some data, switch to passthrough mode
  if (Serial.available() > 2) {
    serialLoop();
  }
  switch (draw_mode) {
    case COLOR_MODE:
      color_loop();
      break;
    case POWERUP_MODE:
      powerup_loop();
      break;
    case DIM_MODE:
      cooldown_loop();
      break;
    case PARTY_MODE:
      party_mode();
      break;
    case OFF_MODE:
      off();
      break;
  }
  button.process();
}



