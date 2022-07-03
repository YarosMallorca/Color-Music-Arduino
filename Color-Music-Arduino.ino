#include <FastLED.h>
#include "VolAnalyzer.h"
#include <EEPROM.h>
#include <GyverEncoder.h>

// AUDIO SETTINGS
#define SOUND_PIN A2    // AUDIO IN PIN

// LED STRIP SETTINGS
#define STRIP_PIN 12     // LED PIN
#define LEDS_AM 240     // LED COUNT

uint8_t ledBrightness = 255; // LED Overall brightness

// EFFECT SETTINGS
uint8_t waveSpeed = 3; // Color Music wave speed
uint8_t colorStep = 30; // Color Music Color Space (multicolor mode only)
int startHue = 0; // Color Music Singlecolor (singlecolor mode only)
uint8_t lampTemp = 150; // Lamp Temperature (lamp mode only)
int solidColor = 160; // Solid Color (solid color mode only)

uint8_t encMenu = 0; // Current Menu selected via Encoder: 0 = Main; 1 = Brightness Adjustment
uint8_t encItem = 0; // Current Item selected via Encoder

CRGB leds[LEDS_AM]; // Configure LED Output
Encoder enc(5, 6, 4, TYPE2); // Configure Encoder

VolAnalyzer sound(SOUND_PIN); // Configure audio device / in pin

uint8_t curColor = 0;
uint8_t curMode = 1;
uint8_t curMenu = 1;


DEFINE_GRADIENT_PALETTE (whiteblue_gp) {
  0, 0, 0, 0,
  51, 255, 255, 255,
  102, 0, 255, 180,
  153, 0, 255, 255,
  204, 0, 0, 255,
  255, 100, 0, 255,
};

void setup() {
  attachInterrupt(0, isrCLK, CHANGE);
  attachInterrupt(1, isrDT, CHANGE);
  FastLED.addLeds<WS2812, STRIP_PIN, GRB>(leds, LEDS_AM); // Add Leds to Array
  FastLED.setBrightness(255); // Set led brightness according to variable
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 18000);

  // sound analyzer settings
  sound.setVolK(15);        // sound filtration
  sound.setVolMax(250);     // sound volume
  sound.setPulseMax(200);   // max sound pulse
  sound.setPulseMin(150);   // min sound pulse
  sound.setTrsh(100);       // noise filtration
}

void isrCLK() {
  enc.tick();
}
void isrDT() {
  enc.tick();
}

void loop() {
  enc.tick(); // Get encoder state
  
  if (curMode == 1) {
    multicolor();
    if (enc.isClick()) {
      curMode++;
    }
  }
 
  else if (curMode == 2) {
    singlecolor();
    if (enc.isRight() && curColor < 360) {
      startHue += 10;
    }
    else if (enc.isRight() && curColor >= 360) {
      startHue = 0;
    }
    else if (enc.isLeft() && curColor > 0) {
      startHue -= 10;
    }
    else if (enc.isLeft() && curColor <= 0) {
      startHue = 360;
    }
    if (enc.isClick()) {
      curMode++;
    }

  }

  else if (curMode == 3) {
    lightcolor();
    if (enc.isRight() && solidColor < 360) {
      solidColor += 10;
    }
    else if (enc.isRight() && solidColor >= 360) {
      solidColor = 0;
    }
    else if (enc.isLeft() && solidColor > 0) {
      solidColor -= 10;
    }
    else if (enc.isLeft() && curColor <= 0) {
      solidColor = 360;
    }
    if (enc.isClick()) {
      curMode++;
    }
  }

  else if (curMode == 4) {
    lampmode();
    if (enc.isRight() && lampTemp < 220) {
      lampTemp += 10;
    }

    else if (enc.isLeft() && lampTemp > 0) {
      lampTemp -= 10;
    }
    if (enc.isClick()) {
      curMode++;
    }
  }

  else if (curMode == 5) {
    wavemode();
    if (enc.isClick()) {
      curMode = 1;
    }
  }
}

void multicolor() {
  if (sound.tick()) {
    for (int k = 0; k < waveSpeed; k++) {
      for (int i = LEDS_AM - 1; i > 0; i--) leds[i] = leds[i - 1];
    }
    if (sound.pulse()) curColor += colorStep;
    int vol = sound.getVol();
    CRGB color = CHSV(curColor, 255, vol);
    for (int i = 0; i < waveSpeed; i++) leds[i] = color;
    FastLED.show();
  }
}

void singlecolor() {
  if (sound.tick()) {
    for (int k = 0; k < waveSpeed; k++) {
      for (int i = LEDS_AM - 1; i > 0; i--) leds[i] = leds[i - 1];
    }
    
    int vol = sound.getVol();
    CRGB color = CHSV(startHue + vol / 5, 255 - vol / 2, vol);
    for (int i = 0; i < waveSpeed; i++) leds[i] = color;
    FastLED.show();
  }
}

void lightcolor() {
  fill_solid( leds, LEDS_AM,  CHSV(solidColor, 255, 255));
  FastLED.show();
}

void lampmode() {
  fill_solid( leds, LEDS_AM,  CHSV(40, lampTemp, 180));
  FastLED.show();
}

void wavemode() {
  static uint8_t colorIndex = 0;
  CRGBPalette16 whitebluePalette = whiteblue_gp;
  fill_palette(leds, LEDS_AM, colorIndex, 500/LEDS_AM, whitebluePalette, 180, LINEARBLEND);
  EVERY_N_MILLISECONDS(10){colorIndex-=2;};

  FastLED.show();
}

void brightness() {
  for (int i=0; i<20; i++)
  leds[i] = CHSV( map(i % 8, 0, 8, 0, 255), 255, 255);
}

