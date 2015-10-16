#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif

// Pattern types supported:
enum  pattern { NONE, SCANNER, STANDBY, SPARKY };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel {
  public:

  // Member Variables:  
  pattern   ActivePattern;  // which pattern is running
  direction Direction;     // direction to run the pattern

  uint32_t  Interval;   // milliseconds between updates
  uint32_t  lastUpdate; // last update of position
  uint8_t   lastPixel;  // last pixel selected (for random sparks function)

  uint32_t  Color1, Color2;  // What colors are in use
  uint16_t  TotalSteps;  // total number of steps in the pattern
  uint16_t  Index;  // current step within the pattern

  void (*OnComplete)();  // Callback on completion of pattern

  // Constructor - calls base-class constructor to initialize strip
  NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
  :Adafruit_NeoPixel(pixels, pin, type) {
    OnComplete = callback;
  }

  // Update the pattern
  void Update() {
    if((millis() - lastUpdate) > Interval) { // time to update
      lastUpdate = millis();
      switch(ActivePattern) {
        case SCANNER:
          ScannerUpdate();
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

  // Increment the Index and reset at the end
  void Increment() {
    if (Direction == FORWARD) {
      Index++;
      if (Index >= TotalSteps) {
        Index = 0;
        if (OnComplete != NULL) {
          OnComplete(); // call the comlpetion callback
        }
      }
    }
    else { // Direction == REVERSE
      --Index;
      if (Index <= 0) {
        Index = TotalSteps-1;
        if (OnComplete != NULL) {
          OnComplete(); // call the comlpetion callback
        }
      }
    }
  }

  // Reverse pattern direction
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

  // Calculate 50% dimmed version of a color (used by ScannerUpdate)
  uint32_t DimColor(uint32_t color) {
    // Shift R, G and B components one bit to the right
    uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
      return dimColor;
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

  // Input a value 0 to 255 to get a color value.
  // The colours are a transition r - g - b - back to r.
  uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
      return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if(WheelPos < 170) {
      WheelPos -= 85;
      return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else {
      WheelPos -= 170;
      return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
  }

  // Initialize for a SCANNNER
  void Scanner(uint32_t color1, uint8_t interval) {
    ActivePattern = SCANNER;
    Interval = interval;
    TotalSteps = (numPixels() - 1) * 2;
    Color1 = color1;
    Index = 0;
  }

  // Update the Scanner Pattern
  void ScannerUpdate() { 
    for (int i = 0; i < numPixels(); i++) {
      if (i == Index) {  // Scan Pixel to the right
        setPixelColor(i, Color1);
      }
      else if (i == TotalSteps - Index) { // Scan Pixel to the left
        setPixelColor(i, Color1);
      }
      else { // Fading tail
        setPixelColor(i, DimColor(getPixelColor(i)));
      }
    }
    show();
    Increment();
  }

  // Initialize for a Standby, aka Fade
  void Standby(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD) {
    ActivePattern = STANDBY;
    Interval = interval;
    TotalSteps = steps;
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = dir;
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
  void Sparky(uint32_t color1, uint32_t color2, uint8_t interval) {
    ActivePattern = SPARKY;
    Interval = interval;
    TotalSteps = numPixels();
    Color1 = color1;
    Color2 = color2;
    lastPixel = 0;
  }

  // Update for Sparky
  void SparkyUpdate() {
    setPixelColor(lastPixel, 0);
    uint8_t i = random(numPixels());
    setPixelColor(i, Color1);
    lastPixel = i;
    lastUpdate=millis();
    show();
  }
};

// Let's try placing defines here
// Will need to update code so that LeftBtn controls RightHand and vice versa..
#define leftBtnPin 9
#define leftLedPin 6

uint8_t leftBtnState;

// Set up the NeoPixel Rings
NeoPatterns LeftHand(12, leftLedPin, NEO_GRB + NEO_KHZ800, &LeftHandComplete);

  // These were added to delay printing to Serial.
  //uint16_t printDelay = 2048; // ~ 2 seconds
  //uint32_t lastPrint;

// Initialize everything and prepare to start
void setup() {
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  //Serial.begin(115200);
  pinMode(leftBtnPin, INPUT_PULLUP);
  digitalWrite(leftBtnPin, HIGH);

  // Initialize all the pixelStrips
  LeftHand.begin();
  LeftHand.setBrightness(85); // Set to 1/3 brightness to keep heat down and prevent accidental blinding
  LeftHand.Standby(0x6A6AFF, 0xFAFAFA, 5, 25);
}
 
// Main loop
void loop() {
  LeftHand.Update();
  leftBtnState = digitalRead(leftBtnPin);
  if(leftBtnState==LOW){
    if(LeftHand.ActivePattern==STANDBY) {
      LeftHand.Color1 = 0xFF0000;
      LeftHand.Interval=250;
      LeftHand.ActivePattern=SPARKY;
    }
    else {
      LeftHand.ActivePattern==STANDBY;
      LeftHand.Color1 = 0x6A6AFF;
      LeftHand.Interval=25;
    }
  }
  //if(millis()-lastPrint >= printDelay){
  //  Serial.println(leftBtnState);
  //}
}

//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Ring 1 Completion Callback
void LeftHandComplete() {
  if(LeftHand.ActivePattern==SPARKY) {
    LeftHand.Reverse();
  } else {
    LeftHand.Update();
  }
}

/*************
  End of File
 *************/
