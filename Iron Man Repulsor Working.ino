/*
  Another attempt at having a button press cycle through various patterns.
  Can now cycle between STANDBY and SPARKY but needs better button debouncing.
  Which is something we are attempting to do with this update.
*/

#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif

enum  pattern { NONE, STARTUP, STANDBY, SPARKY };
enum  direction { FORWARD, REVERSE };

class ArcPatterns : public Adafruit_NeoPixel {
  public:
    pattern   ActivePattern;
    direction Direction;
    uint32_t  Interval;
    uint32_t  lastUpdate;
    uint8_t   lastPixel;
    uint32_t  Color1, Color2;
    uint16_t  TotalSteps;
    uint16_t  Index;

  void (*OnComplete)();

  ArcPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)()) :Adafruit_NeoPixel(pixels, pin, type) {
    OnComplete = callback;
  }

  void Update() {
    if( (millis() - lastUpdate) > Interval) {
      lastUpdate = millis();
      switch(ActivePattern) {
        case STARTUP:
          StartupUpdate();
          break;
        case STANDBY:
          StandbyUpdate();
          break;
        case SPARKY:
          SparkyUpdate();
          break;
        default:
          break;
      }
    }
  }

  void Increment() {
    if (Direction == FORWARD) {
      Index++;
      if (Index >= TotalSteps) {
        Index = 0;
        if (OnComplete != NULL) {
          OnComplete(); // call the completion callback
        }
      }
    }
    else { // Direction == REVERSE
      --Index;
      if (Index <= 0) {
        Index = TotalSteps-1;
        if (OnComplete != NULL) {
          OnComplete(); // call the completion callback
        }
      }
    }
  }

  void Reverse() {
    if (Direction == FORWARD) {
      Direction = REVERSE;
      Index = TotalSteps-1;
    }
    else {
      Direction = FORWARD;
      Index = 0;
    }
  }

  // Set all pixels to a color (synchronously)
  void ColorSet(uint32_t color)  {
    for (int i = 0; i < numPixels(); i++) {
      setPixelColor(i, color);
    }
    show();
  }

  // Returns the Red component of a 32-bit color
  uint8_t Red(uint32_t color) {
    return (color >> 16) & 0xFF;
  }

  // Returns the Green component of a 32-bit color
  uint8_t Green(uint32_t color) {
    return (color >> 8) & 0xFF;
  }

  // Returns the Blue component of a 32-bit color
  uint8_t Blue(uint32_t color) {
    return color & 0xFF;
  }

// Initialize Start-up
  void Startup(uint32_t color1) {
    ActivePattern = STARTUP;
    Interval     = 250;
    TotalSteps   = 9;
    Index        = 0;
    Direction    = FORWARD;
    Color1       = color1;
  }

// Updater for Start-up
  void StartupUpdate() {
    if(Index % 2 == 0) { // Check for even Index
      for( uint8_t i=0; i <= numPixels(); i++ ) {
        setPixelColor(i, 0); // NeoPixels Off
      }
    }
    else { // Index is odd
      for( uint8_t i=0; i <= numPixels(); i++ ) {
        setPixelColor(i, Color1); // NeoPixels On
      }
    }
    show();
    Increment();
  }

// Initialize for a Standby, aka Fade
  void Standby(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD) {
    ActivePattern = STANDBY;
    Interval      = interval;
    TotalSteps    = steps;
    Color1        = color1;
    Color2        = color2;
    Index         = 0;
    Direction     = dir;
  }

// Update the Standby / Fade Pattern
  void StandbyUpdate() {
    // Calculate linear interpolation between Color1 and Color2
    // Optimise order of operations to minimize truncation error
    uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
    uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
    uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

    ColorSet(Color(red, green, blue));
    show();
    Increment();
  }

// Initialize for Sparky
  void Sparky(uint32_t color1, uint8_t interval) {
    ActivePattern = SPARKY;
    TotalSteps    = numPixels()*2;
    lastPixel     = 0;
    Interval      = interval;
    Color1        = color1;
    Direction     = FORWARD;
  }

// Update for Sparky
  void SparkyUpdate() {
    for(uint8_t i=0;i<numPixels();i++){
      setPixelColor(i, 0);
    }
    uint8_t i = random(numPixels());
    setPixelColor(i, Color1);
    //lastPixel = i;
    lastUpdate=millis();
    show();
    Increment();
  }
}; // End Derived Class

// Definitions / Initializations
//Starting with left hand and control button
#define leftLedPin  6
#define rightBtnPin 9

// Button debouncing variables
uint16_t debounceDelay  = 50;
uint8_t  rightBtnState;
uint8_t  rightLastState = HIGH;
uint32_t rightLastDebounce;

// NeoPixel setup
ArcPatterns LeftHand(12, leftLedPin, NEO_GRB + NEO_KHZ800, &LeftHandComplete);

void setup() {
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pinMode(rightBtnPin, INPUT_PULLUP);
  digitalWrite(rightBtnPin, HIGH);

  LeftHand.begin();
  LeftHand.setBrightness(85); // 1/3 brightness to reduce heat and sight interference
  LeftHand.Startup(0x6A6AFF);
}

void loop() {
  LeftHand.Update();
} // loop() end

void LeftHandComplete() {
  RightButtonCheck();
  switch(LeftHand.ActivePattern) {
    case STARTUP:
      LeftHand.Color1 = 0x6A6AFF;
      LeftHand.Color2 = 0xFAFAFA;
      LeftHand.TotalSteps = 5;
      LeftHand.Interval = 25;
      LeftHand.ActivePattern = STANDBY;
      break;
    case STANDBY:
      LeftHand.Reverse();
      LeftHand.Update();
      break;
    default:
      LeftHand.Update();
      break;
  } // end Switch statement
}

void RightButtonCheck() {
  // Attempt at better debounce detection...
  int rightBtnRead = digitalRead(rightBtnPin);
  if(rightBtnRead != rightLastState) {
    rightLastDebounce = millis();
  }
  if( (millis()-rightLastDebounce) > debounceDelay) {
    if (rightBtnRead != rightBtnState) {
      rightBtnState = rightBtnRead;
      if (rightBtnState == LOW) {
        if(LeftHand.ActivePattern==STANDBY) {
          LeftHand.Color1 = 0xAAAAFF; // To be changed after testing
          LeftHand.TotalSteps = 2;
          LeftHand.Interval = 50;
          LeftHand.ActivePattern = SPARKY;
        }
        else {
          LeftHand.Color1 = 0x6A6AFF;
          LeftHand.Color2 = 0xFAFAFA;
          LeftHand.TotalSteps = 5;
          LeftHand.Interval = 25;
          LeftHand.ActivePattern = STANDBY;        
        }
      }
    }
  }
  rightLastState = rightBtnRead;
} // end RightButtonCheck

/*
 * End of File
 */
