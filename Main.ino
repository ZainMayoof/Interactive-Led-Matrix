
// libraries
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <FastLED.h>

// led setup
#define LED_PIN 6 // arduino pin 6 
#define NUM_LEDS 530
#define BRIGHTNESS 220 // max = 250
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// TCA9548A & MPR121 Setup
#define TCA_ADDR 0x70  // I2C address for TCA9548A 
#define TOUCH_THRESHOLD 5.6 // touch has to be larger than release
#define RELEASE_THRESHOLD 5

Adafruit_MPR121 mpr[3];  // Three MPR121 sensors (one per channel: c0, c1, c2)
bool currentState[3][12] = {false};  // Stores the current touch states
bool ledState[3][12] = {false};  // Keeps track of which LEDs should be on

// Electrode-LED Mapping
struct Electrode {
  int colorLeds1[6];  // first row of leds in the circle (max 6 individual leds)
  int colorLeds2[6];  // second row of leds in the circle
};

// Define electrode-LED mappings for all channels :{{led1}, {led2}}
// channel 0: rows 1 and 2
// channel 1: rows 2 and 4
// channel 2: rows 5 and 6

// about 10 leds per electrode / circle
Electrode electrodes[3][12] = {
  // Channel 0
  {
    {{2, 3, 4, 5, 6, -1}, {84, 85, 86, 87, 88, -1}},  //Electrode 1
    {{9, 10, 11, 12, 13, 14}, {76, 77, 78, 79, 80, 81}},  // Electrode 0
    {{16, 17, 18, 19, 20, 21}, {69, 70, 71, 72, 73, 74}}, // Electrode 2 
    {{23, 24, 25, 26, 27, 28}, {61, 62, 63, 64, 65, 66}},  // Electrode 3
    {{31, 32, 33, 34, 35, 36}, {54, 55, 56, 57, 58, 59}},  // Electrode 4
    {{38, 39, 40, 41, 42, 43}, {47, 48, 49, 50, 51, 52}},  // Electrode 5
    {{92, 93, 94, 95, 96, 97}, {173, 174, 175, 176, 177}},  // Electrode 6
    {{99,100, 101, 102, 103, 104}, {165, 166, 167, 168, 169, 170}}, // Electrode 7 
    {{ 107, 108, 109, 110, 111, -1}, {158, 159, 160, 161, 162, -1}},  // Electrode 8
    {{ 114, 115, 116, 117, 118, 119}, { 150, 151, 152, 153, 154, 155}},  // Electrode 9
    {{121, 122, 123, 124, 125, 126}, {143, 144, 145, 146, 147, 148}},  // Electrode 10
    {{ 129, 130, 131, 132, 133, -1}, {136, 137, 138, 139, 140, 141}}   // Electrode 11
  },

  // Channel 1
  {
    {{180, 181, 182, 183, 184, -1}, {261, 262, 263, 264, 265, -1}},  
    {{187, 188, 189, 190, 191, 192}, {253, 254, 255, 256, 257, 258}},  
    {{194, 195, 196, 197, 198, 199}, {251, 250, 249, 248, 247, 246}},  
    {{202, 203, 204, 205, 206, -1}, {239, 240, 241, 242, 243, -1}},  
    {{209, 210, 211, 212, 213, 214}, { 236, 235, 234, 233, 232, -1}},  
    {{216, 217, 218, 219, 220, 221}, {229, 228, 227, 226, 225, 224}}, 
    {{267, 268, 269, 270, 271, 272}, {347, 348, 349, 350, 351, 352}},
    {{275, 276, 277, 278, 279, -1}, { 344, 343, 342, 341, 340, -1}},  
    {{282, 283, 284, 285, 286, -1}, { 337, 336, 335, 334, 333, -1}},  
    {{288, 289, 290, 291, 292, 293}, { 330, 329, 328, 327, 326, 325}}, 
    {{297, 298, 299, 300, 301, 302}, {323, 322, 321, 320, 319, 318}},
    {{304, 305, 306, 307, 308, 309}, { 315, 314, 313, 312, 311, -1}}  
  },

  // Channel 2
  {
    {{354, 355, 356, 357, 358}, {432, 433, 434, 435, 436}},  
    {{360, 361, 362, 363, 364}, {424, 425, 426, 427, 428, 429 }},  
    {{368, 369, 370, 371, 372}, {417, 418, 419, 420, 421}},  
    {{375, 376, 377, 378, 379, 380}, {412, 413, 414, 415, 416, 417}},  
    {{382, 383,384, 385, 386, 387}, {402, 403, 404, 405, 406, 407}},  
    {{390, 391, 392, 393, 395 }, {396, 397, 398, 399, 400}},  
    {{439, 440, 441, 442, 443, 444}, {518, 519, 520, 521, 522, 523}},  
    {{446, 447, 448, 449, 450, 451}, {511, 512, 513, 514, 515, 516}}, 
    {{453, 454, 455, 456, 457, 458}, {504, 505, 506, 507, 508, -1}},  
    {{461, 462, 463, 464, 465}, {497, 498, 499, 500, 501}},  
    {{468, 469, 470, 471, 472, 473}, {489, 490, 491, 492, 493, 494}},  
    {{475, 476, 477, 478, 479, 480}, {482, 483, 484, 485, 486, 487}}  
  }
};

// Color Sequence for Touch LEDs
CRGB colorSequence[] = {CRGB::Red, CRGB::Orange, CRGB::Blue, CRGB::Purple};
int colorIndex[3][12] = {0};  // Tracks the current color index for each electrode

// Function to select a TCA9548A channel
void selectMuxChannel(uint8_t channel) {
  if (channel > 7) return;  
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);  
  Wire.endTransmission();
}

// Function to set LEDs for a specific electrode
void setElectrodeLeds(int channel, int electrode, CRGB color) {
  for (int i = 0; i < 6; i++) {
    int ledIndex1 = electrodes[channel][electrode].colorLeds1[i];
    int ledIndex2 = electrodes[channel][electrode].colorLeds2[i];
    if (ledIndex1 != -1) {
      leds[ledIndex1] = color;
    }
    if (ledIndex2 != -1) {
      leds[ledIndex2] = color;
    }
  }
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  // Initialize MPR121 sensors for each channel
  for (uint8_t i = 0; i < 3; i++) {
    selectMuxChannel(i);
    if (!mpr[i].begin(0x5A)) {  
      Serial.print("MPR121 not found on channel ");
      Serial.println(i);
      while (1);  
    }
    mpr[i].setThreshholds(TOUCH_THRESHOLD, RELEASE_THRESHOLD);  
  }
}

// loop to continuously check the state
void loop() {
  for (uint8_t i = 0; i < 3; i++) {
    selectMuxChannel(i);  
    uint16_t touchState = mpr[i].touched(); 

    for (uint8_t j = 0; j < 12; j++) {
      bool state = touchState & (1 << j);
      if (state && !ledState[i][j]) {
        colorIndex[i][j] = (colorIndex[i][j] + 1) % 4;
        setElectrodeLeds(i, j, colorSequence[colorIndex[i][j]]);
        ledState[i][j] = true;
      }
      else if (!state && ledState[i][j]) {
        for (int k = 0; k < 6; k++) {
          int ledIndex1 = electrodes[i][j].colorLeds1[k];
          int ledIndex2 = electrodes[i][j].colorLeds2[k];
          if (ledIndex1 != -1) {
            leds[ledIndex1].fadeToBlackBy(10);
          }
          if (ledIndex2 != -1) {
            leds[ledIndex2].fadeToBlackBy(10);
          }
        }
        ledState[i][j] = false;
      }
    }
  }
  FastLED.show();
}
