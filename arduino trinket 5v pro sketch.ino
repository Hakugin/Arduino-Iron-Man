#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif

// Pin definitions
// Only 1 pin to define for now
#define pinChestPixel 9
// Define the number of NeoPixels on the Arc Reactor
#define cntChestPixel 12

// Various variables
uint8_t brightDefault = 84;
uint8_t brightMin     = 20;
uint8_t brightMax     = 168;
uint8_t brightValue   = 20;
uint8_t fadeState     = 0; // 0: In, 1: out
uint8_t lastRandomPix;

// Last time the LEDs were updated
uint32_t prevTime;

// Delay between changes
uint32_t standbyDelay = 15;

// Default color
uint32_t chestColor = 0x6464FA;

// Initialize the NeoPixels in the "Chest Reactor"
Adafruit_NeoPixel pixelsChest = Adafruit_NeoPixel(cntChestPixel, pinChestPixel);

void setup() {
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixelsChest.begin();
  pixelsChest.setBrightness(brightDefault);
  
  initializeChest();
  
}

void loop() {
  standbyEffect(chestColor, standbyDelay);

}

void initializeChest() {
  for(int16_t p = 0; p < cntChestPixel; p++) {
    pixelsChest.setPixelColor(p, 0xFF0000);
  }
  pixelsChest.show();
  delay(1000);
  
  for(int16_t p = 0; p < cntChestPixel; p++) {
    pixelsChest.setPixelColor(p, 0x00FF00);
  }
  pixelsChest.show();
  delay(1000);
  
  for(int16_t p = 0; p < cntChestPixel; p++) {
    pixelsChest.setPixelColor(p, 0x0000FF);
  }
  pixelsChest.show();
  delay(1000);
  
  for(int16_t p = 0; p < cntChestPixel; p++) {
    pixelsChest.setPixelColor(p, 0x000000);
  }
  pixelsChest.show();
  delay(1000);
}

void standbyEffect(uint32_t color, uint32_t wait) {
  if(fadeState==0) {
    if( (millis()-prevTime) > wait ) {
      if(brightValue < brightMax + 1) {
        pixelsChest.setBrightness(brightValue);
        for(int16_t p = 0; p < cntChestPixel; p++) {
          pixelsChest.setPixelColor(p, color);
        }
        pixelsChest.show();
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
        pixelsChest.setBrightness(brightValue);
        for(int16_t p=0; p < cntChestPixel; p++) {
          pixelsChest.setPixelColor(p, color);
        }
        pixelsChest.show();
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
    int16_t i = random(cntChestPixel);
    pixelsChest.setBrightness(brightMax);
    pixelsChest.setPixelColor(lastRandomPix, 0);
    pixelsChest.show();
    lastRandomPix = i;
    pixelsChest.setPixelColor(i, c);
    pixelsChest.show();
  }
}

/*************
  End of file
 *************/
