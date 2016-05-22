//
// Arduino Software for Table Light
// --------------------------------------------------------------------------
// (c)2016 by Lucky Resistor
//
// Hardware and Schemas:
// http://luckyresistor.me/projects/table-light/
//
// This Software is written for the Adafruit Trinket 5V. The trinket is a board
// using the ATtiny85 microcontroller at 8MHz.
//
#include <Adafruit_NeoPixel.h>


/// The number of pixels
const uint8_t numberOfPixels = 24;

/// The pin for the data output.
const uint8_t dataPin = 0;

/// The global object to access the NeoPixels
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numberOfPixels, dataPin, NEO_GRBW + NEO_KHZ800);

/// A gamme correction (from NeoPixel example file).
const uint8_t gamma[] PROGMEM = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255};


/// A own color class to simplify calculations
class Color
{
public:
  /// Create black color.
  Color()
    : r(0), g(0), b(0), w(0) {    
  }
  /// Create a color with the given RGBW values.
  Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    : r(r), g(g), b(b), w(w) {
  }
  /// Create a color with the given 16bit value. WBGR
  Color(uint16_t value) {
    r = ((value & 0x000f) >> 0) * 0x10;
    g = ((value & 0x00f0) >> 4) * 0x10;
    b = ((value & 0x0f00) >> 8) * 0x10;
    w = ((value & 0xf000) >> 12) * 0x10;
  }
  /// Mix two colors
  Color mix(const Color &other, uint8_t shift) const {
    const uint16_t aShift = 0x100-(uint16_t)shift;
    const uint16_t bShift = shift;
    const uint8_t newR = (((uint16_t)r * aShift) + ((uint16_t)other.r * bShift))/0x100;
    const uint8_t newG = (((uint16_t)g * aShift) + ((uint16_t)other.g * bShift))/0x100;
    const uint8_t newB = (((uint16_t)b * aShift) + ((uint16_t)other.b * bShift))/0x100;
    const uint8_t newW = (((uint16_t)w * aShift) + ((uint16_t)other.w * bShift))/0x100;
    return Color(newR, newG, newB, newW);
  }
  /// Set the brightness
  Color dim(uint8_t level) const {
    const uint16_t level16 = (uint16_t)level+1;
    const uint8_t newR = ((uint16_t)r * level16)/0x100;
    const uint8_t newG = ((uint16_t)g * level16)/0x100;
    const uint8_t newB = ((uint16_t)b * level16)/0x100;
    const uint8_t newW = ((uint16_t)w * level16)/0x100;
    return Color(newR, newG, newB, newW);
  }
  /// Get a color value. color = 0-191
  static Color wheel(uint8_t color, uint8_t white) {
    if (color < 64) {
      const uint8_t aShift = 63-(uint16_t)color;
      const uint8_t bShift = color;
      const uint8_t newR = aShift * 4;
      const uint8_t newG = bShift * 4;
      const uint8_t newB = 0;
      return Color(newR, newG, newB, white);
    } else if (color < 128) {
      const uint8_t aShift = 63-(uint16_t)(color - 64);
      const uint8_t bShift = (color - 64);
      const uint8_t newR = 0;
      const uint8_t newG = aShift * 4;
      const uint8_t newB = bShift * 4;
      return Color(newR, newG, newB, white);      
    } else {
      const uint8_t aShift = 63-(uint16_t)(color - 128);
      const uint8_t bShift = (color - 128);
      const uint8_t newR = bShift * 4;
      const uint8_t newG = 0;
      const uint8_t newB = aShift * 4;
      return Color(newR, newG, newB, white);
    }
  }
  /// Calculate the 32bit value for the NeoPixel library.
  uint32_t getValue() const {
    uint32_t result = 0;
    result |= (uint32_t)pgm_read_byte_near(gamma + b);
    result |= (uint32_t)pgm_read_byte_near(gamma + g) << 8;
    result |= (uint32_t)pgm_read_byte_near(gamma + r) << 16;
    result |= (uint32_t)pgm_read_byte_near(gamma + w) << 24;
    return result;
  }
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t w;  
};


/// The configurations
uint8_t configurations[] = {};

/// The base colors
Color baseColors[numberOfPixels];

/// The blend colors (for fire effects)
Color blendColors[numberOfPixels];

/// The begin of the random color
Color randomBegin;

/// The end of the random color.
Color randomEnd;

/// The random phase.
uint8_t randomPhase = 0;

/// The current random phase speed.
uint8_t randomSpeed = 0;

/// The rotation
uint16_t colorRotation = 0;

/// THe rotation speed
uint8_t colorRotationSpeed = 0;

/// The brightness 0-7
uint8_t brightness = 0;

/// The current mode 0-24
uint8_t mode = 0;

/// The last button state.
bool lastButtonState = false;

/// The start time of the button press
unsigned long buttonPressStart = 0;

/// The last time for demo mode
unsigned long demoModeLastTime = 0;

/// The demo mode.
bool demoModeEnabled = false;


const uint16_t whiteColors[] PROGMEM {
  0xF000, // Warm white
  0xF008, // Warm white+
  0xF08F, // Warm white++
  0xF800, // Cold white
  0xFF00, // Cold white+
  0xFFFF, // Bright white
};

const uint16_t colorPatterns[] PROGMEM {
  0x000F, 0x000F, 0x000F, 0x000F, 0x00F0, 0x00F0, 0x00F0, 0x00F0,
  0x00AF, 0x008F, 0x006F, 0x004F, 0x002F, 0x004F, 0x006F, 0x008F,
  0x100F, 0x100F, 0x100F, 0x100F, 0xC400, 0xC400, 0xC400, 0xC400,
  0x00FF, 0x00F8, 0x00F0, 0x04F0, 0x08F0, 0x04F0, 0x00F0, 0x00F8,
  0x0F10, 0x0F10, 0x0200, 0x0200, 0x0F10, 0x0F10, 0x0200, 0x0200,
  0x002F, 0x002F, 0x0002, 0x0002, 0x002F, 0x002F, 0x0002, 0x0002,
};

/// The predefined brightness levels.
const uint8_t brightnessLevels[] PROGMEM = {0xff, 0xf0, 0xe0, 0xc0, 0xa0, 0x80, 0x60, 0x40}; 


/// Update the neopixels
void updateNeoPixels()
{
  if (randomSpeed>0) {
    for (int i = 0; i < numberOfPixels; ++i) {
      const Color result = baseColors[i].mix(blendColors[i], randomPhase).dim(pgm_read_byte_near(brightnessLevels+brightness));
      pixels.setPixelColor(i, result.getValue());
    }
  } else {
    for (int i = 0; i < numberOfPixels; ++i) {
      const uint8_t offset = (colorRotation >> 8);
      const uint8_t shift = (colorRotation & 0xff);
      const Color a = baseColors[(i+offset)%numberOfPixels];
      const Color b = baseColors[(i+1+offset)%numberOfPixels];
      const Color result = a.mix(b, shift).dim(pgm_read_byte_near(brightnessLevels+brightness));
      pixels.setPixelColor(i, result.getValue());
    }
  }
  pixels.show();
}


void fillWithColor(const Color &color)
{
  for (int i = 0; i < numberOfPixels; ++i) {
    baseColors[i] = color;
  }
}


uint8_t getSimpleRandom()
{
  static uint16_t seed = 70;
  seed = 181 * seed + 359;
  return (uint8_t)(seed >> 8);
}


void generateNewRandomBlend()
{
  for (int i = 0; i < numberOfPixels; ++i) {
    blendColors[i] = randomBegin.mix(randomEnd, getSimpleRandom());
  }
  randomSpeed = (getSimpleRandom() >> 3) + 8;
}


void enableRandom(Color a, Color b)
{
  randomBegin = a;
  randomEnd = b;
  randomPhase = 0;
  randomSpeed = 16;
  fillWithColor(Color());
  generateNewRandomBlend(); 
}


void initializeMode()
{
  if (mode >= 50) {
    mode = 0;
  }
  colorRotation = 0;
  colorRotationSpeed = 0;
  randomSpeed = 0;
  if (mode < 6) {
    fillWithColor(Color(pgm_read_word_near(whiteColors+mode)));
  } else if (mode < 18) {
    fillWithColor(Color::wheel((mode-6)*16, 0));
  } else if (mode < 30) {
    fillWithColor(Color::wheel((mode-18)*16, 32));
  } else if (mode < 36) {
    for (uint8_t j = 0; j < 8; ++j) {
      const Color color(pgm_read_word_near(colorPatterns+j+(8*(mode-30))));
      for (uint8_t i = 0; i < (numberOfPixels/8); ++i) {
        baseColors[j+8*i] = color;
      }
    }
  } else if (mode < 42) {
    for (uint8_t j = 0; j < 8; ++j) {
      const Color color(pgm_read_word_near(colorPatterns+j+(8*(mode-36))));
      for (uint8_t i = 0; i < (numberOfPixels/8); ++i) {
        baseColors[j+8*i] = color;
      }
    }
    colorRotationSpeed = 4;
  } else if (mode < 46) {
    for (uint8_t i = 0; i < numberOfPixels; ++i) {
      baseColors[i] = Color::wheel((uint8_t)( (uint16_t)i * 192 / numberOfPixels ), 0);
    }
    colorRotationSpeed = (mode - 42)*4;
  } else if (mode < 50) {
    switch (mode) {
      case 46:
        enableRandom(Color(0x001F), Color(0x00CF));
        break;
      case 47:
        enableRandom(Color(0x0006), Color(0x006F));
        break;
      case 48:
        enableRandom(Color(0x0F00), Color(0x0130));
        break;
      case 49:
        enableRandom(Color(0xF008), Color(0x4400));
        break;
    }
  }
}


/// Setup everything.
void setup()
{
  pixels.begin();
  pixels.clear();
  pixels.show();
  initializeMode();
  // Setup pin for the button
  pinMode(2, INPUT_PULLUP);
  // Check if the button is pressed initially.
  delay(200);
  const bool buttonPressed1 = (digitalRead(2) == LOW);
  delay(200);
  const bool buttonPressed2 = (digitalRead(2) == LOW);
  if (buttonPressed1 && buttonPressed2) {
    demoModeEnabled = true;
  }
}


/// The loop.
void loop()
{
  const unsigned int loopTimeMs = millis();
  const bool buttonPressed = (digitalRead(2) == LOW);
  if (buttonPressed != lastButtonState) {
    lastButtonState = buttonPressed;
    if (buttonPressed) {
      buttonPressStart = loopTimeMs;
    } else {
      const unsigned long duration = loopTimeMs - buttonPressStart;
      if (duration > 40 && duration < 2000) {
        ++mode;
        initializeMode();
      }
    }
  }
  if (buttonPressed) {
    const unsigned long duration = loopTimeMs - buttonPressStart;
    if (duration > 3000) {
      if (++brightness >= 8) {
        brightness = 0;
      }
      buttonPressStart += 1000;
    }
  }
  updateNeoPixels();
  delay(20);
  if (randomSpeed>0) {
    const uint8_t oldPhase = randomPhase;
    randomPhase += randomSpeed;
    if (oldPhase > randomPhase) {
      for (uint8_t i = 0; i < numberOfPixels; ++i) {
        baseColors[i] = blendColors[i];
      }
      generateNewRandomBlend();
    }
  } else {
    colorRotation += colorRotationSpeed;
    if (colorRotation >= (numberOfPixels*0x0100)) {
      colorRotation = 0;
    }
  }
  if (demoModeEnabled && ((loopTimeMs - demoModeLastTime) > 10000)) {
    demoModeLastTime = loopTimeMs;
    ++mode;
    initializeMode();
  }
}



