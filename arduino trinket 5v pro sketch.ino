#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif

// Pin definitions
#define pinBtn          9
#define pinHands        6
#define handPixelCount  12

// Pixel Variables
uint8_t brightDefault = 84;
uint8_t brightMin     = 20;
uint8_t brightMax     = 168;
uint8_t brightValue   = 20;
uint8_t fadeState     = 0; // 0: In, 1: out
uint8_t lastRandomPix;

// Button Variables
uint8_t  btnState      = LOW;
uint8_t  btnLastState  = LOW;
uint8_t  mainMode      = 0;

uint32_t prevTime;

uint32_t handColor = 0x6464FA;

// lastDebounceTime and debounceDelay for the button
uint32_t lastDebounceTime = 0;
uint8_t debounceDelay    = 250;

// Initialize the NeopixelHands
Adafruit_NeoPixel pixelHands = Adafruit_NeoPixel(handPixelCount, pinHands);

void setup() {
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pinMode(pinBtn, INPUT);
  digitalWrite(pinBtn, LOW);
  pixelHands.begin();
  pixelHands.setBrightness(brightDefault); // 1/3 brightness
}

void loop() {
  btnState = digitalRead(pinBtn);
  if( (millis() - lastDebounceTime) > debounceDelay) {
    if( (btnState == HIGH) && btnState != btnLastState ) {
      if(mainMode >= 1) {
        mainMode = 0;
      } else {
        mainMode++;
      }
    } else {
      btnLastState = LOW;
    }
    lastDebounceTime = millis();
  }
  switch(mainMode){
    case 0:
      standbyEffect(handColor, 5);
    case 1:
      theRandomSparks(handColor, 250);
  }

}

void standbyEffect(uint32_t color, uint32_t wait) {
  if(fadeState==0) {
    if( (millis()-prevTime) > wait ) {
      if(brightValue < brightMax + 1) {
        pixelHands.setBrightness(brightValue);
        for(int16_t p = 0; p < handPixelCount; p++) {
          pixelHands.setPixelColor(p, color);
        }
        pixelHands.show();
        brightValue++;
        prevTime = millis();
      }
      else {
        fadeState = 1;
      }
    }
  }

  else {
    if( (millis()-prevTime) > wait ) {
      if(brightValue > brightMin -1) {
        pixelHands.setBrightness(brightValue);
        for(int16_t p=0; p < handPixelCount; p++) {
          pixelHands.setPixelColor(p, color);
        }
        pixelHands.show();
        brightValue--;
        prevTime = millis();
      }
      else{
        fadeState = 0;
      }
    }
  }
}

void theRandomSparks(uint32_t c, uint8_t wait) {
  if(millis() - prevTime >= wait) {
    int16_t i = random(handPixelCount);
    pixelHands.setBrightness(brightMax);
    pixelHands.setPixelColor(lastRandomPix, 0);
    pixelHands.show();
    lastRandomPix = i;
    pixelHands.setPixelColor(i, c);
    pixelHands.show();
  }
}

/*************
  End of file
 *************/
