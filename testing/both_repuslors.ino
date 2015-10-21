/*******************************************************************************
  Project     : Iron Man Costume Effects
  Author(s)   : Mike Stover and Drake Stover
  Description : Trinket Pro (5v) powered lighting effects for Drake's Iron Man
                costume. Current code controls the hand repulsors and will
                switch between 2 different pattern modes. Chest Arc Reactor
                and sound effects are planned for future additions.
 *******************************************************************************/

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
#define rightBtnPin 9
#define leftLedPin  6

// Right hand and control button
#define leftBtnPin  8
#define rightLedPin 5

// Button debouncing variables
uint16_t debounceDelay  = 50;

// Button on Right Hand
uint8_t  rightBtnState;
uint8_t  rightLastState = HIGH;
uint32_t rightLastDebounce;

// Button on Left Hand
uint8_t  leftBtnState;
uint8_t  leftLastState = HIGH;
uint32_t leftLastDebounce;

// NeoPixel setup
// NeoPixel on Left Hand
ArcPatterns LeftHand(12, leftLedPin, NEO_GRB + NEO_KHZ800, &LeftHandComplete);

// NeoPixel on Right Hand
ArcPatterns RightHand(12, rightLedPin, NEO_GRB + NEO_KHZ800, &RightHandComplete);

void setup() {
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pinMode(rightBtnPin, INPUT_PULLUP);
  digitalWrite(rightBtnPin, HIGH);

  pinMode(leftBtnPin, INPUT_PULLUP);
  digitalWrite(leftBtnPin, HIGH);

  LeftHand.begin(); // Initialize Left Hand NeoPixels
 // 1/3 brightness to reduce heat and sight interference
  LeftHand.setBrightness(85);
  LeftHand.Startup(0x6A6AFF); // Set color to blueish-white

  RightHand.begin(); // Initialize Right Hand NeoPixels
 // 1/3 brightness to reduce heat and sight interference
  RightHand.setBrightness(85);
  RightHand.Startup(0x6A6AFF); // Set color to blueish-white
}

void loop() {
  LeftHand.Update();
  RightHand.Update();
  RightButtonCheck();
  LeftButtonCheck();
} // loop() end

// OnComplete function for L. Hand
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
} // End RightHandComplete function

// OnComplete function for R. Hand
void LeftHandComplete() {
  LeftButtonCheck();
  switch(RightHand.ActivePattern) {
    case STARTUP:
      RightHand.Color1 = 0x6A6AFF;
      RightHand.Color2 = 0xFAFAFA;
      RightHand.TotalSteps = 5;
      RightHand.Interval = 25;
      RightHand.ActivePattern = STANDBY;
      break;
    case STANDBY:
      RightHand.Reverse();
      RightHand.Update();
      break;
    default:
      RightHand.Update();
      break;
  } // end Switch statement
} // End LeftHandComplete function

// Check Right Button Status and if needed change NeoPixel pattern on L. Hand
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

// Check Left Button Status and if needed change NeoPixel pattern on R. Hand
void LeftButtonCheck() {
  // Attempt at better debounce detection...
  int leftBtnRead = digitalRead(leftBtnPin);
  if(leftBtnRead != leftLastState) {
    rightLastDebounce = millis();
  }
  if( (millis()-rightLastDebounce) > debounceDelay) {
    if (leftBtnRead != leftBtnState) {
      leftBtnState = leftBtnRead;
      if (leftBtnState == LOW) {
        if(RightHand.ActivePattern==STANDBY) {
          RightHand.Color1 = 0xAAAAFF; // To be changed after testing
          RightHand.TotalSteps = 2;
          RightHand.Interval = 50;
          RightHand.ActivePattern = SPARKY;
        }
        else {
          RightHand.Color1 = 0x6A6AFF;
          RightHand.Color2 = 0xFAFAFA;
          RightHand.TotalSteps = 5;
          RightHand.Interval = 25;
          RightHand.ActivePattern = STANDBY;        
        }
      }
    }
  }
  leftLastState = leftBtnRead;
} // end LeftButtonCheck

/*************************************************************************************
 This needs tested, but could allow for 1 function to respond to 
 both buttons and control both hands. IE:
 ButtonCheck(LeftHand, rightBtnPin, rightBtnState, rightLastState, rightLastDebounce);
 ButtonCheck(RightHand, leftBtnPin, leftBtnState, leftLastState, leftLastDebounce);
 This could avoid unneeded duplicated code.

void ButtonCheck(Adafruit_NeoPixel &ring, uint8_t btnPin, uint8_t btnState, uint8_t lastState, uint32_t lastDebounce) {
  uint8_t btnRead = digitalRead(btnPin);
  if(btnRead != lastState) {
    lastDebounce = millis();
  }
  if( (millis() - lastDebounce) > debounceDelay ) {
    if(btnRead != btnState) {
      btnState = btnRead;
      if (btnState==LOW) {
        if(ring.ActivePattern==STANDBY) {
          ring.Color1     = 0xAAAAFF;
          ring.TotalSteps = 2;
          ring.Interval   = 50;
          ring.ActivePatten = SPARKY;
        }
        else {
          ring.Color1 = 0x6A6AFF;
          ring.Color2 = 0xFAFAFA;
          ring.TotalSteps = 5;
          ring.Interval = 25;
          ring.ActivePattern = STANDBY;        
        }
      }
    }
  }
  lastState = btnRead;
}
 *************************************************************************************/

/**************
 * End of File
 **************/
