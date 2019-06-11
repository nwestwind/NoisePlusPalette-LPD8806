#include <FastLED.h>
#include "palettes.h"

#define LED_PIN     6
#define CLOCK_PIN   7
#define BRIGHTNESS  60
#define LED_TYPE    LPD8806
#define COLOR_ORDER BRG

const uint8_t kMatrixWidth  = 1;
const uint8_t kMatrixHeight = 8;
const bool    kMatrixSerpentineLayout = false;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define MAX_DIMENSION ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)

CRGB leds[kMatrixWidth * kMatrixHeight];

// The 16 bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;

uint16_t speed = 20; // speed is set dynamically once we've started up
uint16_t scale = 100; // scale is set dynamically once we've started up

uint8_t noise[MAX_DIMENSION][MAX_DIMENSION];

CRGBPalette16 currentPalette( Colorfull_gp );
uint8_t colorLoop = 1;

void setup() {
  delay(3000);
  LEDS.addLeds<LED_TYPE,LED_PIN,CLOCK_PIN,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);

  // Initialize our coordinates to some random values
  x = random16();
  y = random16();
  z = random16();
}

void fillnoise8() {

  uint8_t dataSmoothing = 0;
  if( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }
  
  for(int i = 0; i < MAX_DIMENSION; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < MAX_DIMENSION; j++) {
      int joffset = scale * j;
      
      uint8_t data = inoise8(x + ioffset,y + joffset,z);

      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      data = qsub8(data,16);
      data = qadd8(data,scale8(data,39));

      if( dataSmoothing ) {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
        data = newdata;
      }
      
      noise[i][j] = data;
    }
  }
  
  z += speed;
  
  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;
}

void mapNoiseToLEDsUsingPalette() {
  static uint8_t ihue=0;
  
  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's index into the color palette.

      uint8_t index = noise[j][i];
      uint8_t bri =   noise[i][j];

      // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) { 
        index += ihue;
      }

      // brighten up, as the color palette itself often contains the 
      // light/dark dynamic range desired
      if( bri > 127 ) {
        bri = 255;
      } else {
        bri = dim8_raw( bri * 2);
      }

      CRGB color = ColorFromPalette( currentPalette, index, bri);
      leds[XY(i,j)] = color;
    }
  }
  
  ihue+=1;
}

void loop() {
  // Periodically choose a new palette, speed, and scale
  ChangePaletteAndSettingsPeriodically();

  // generate noise data
  fillnoise8();
  
  // convert the noise data to colors in the LED array
  // using the current palette
  mapNoiseToLEDsUsingPalette();

  LEDS.show();
  delay(60);
}

#define HOLD_PALETTES_X_TIMES_AS_LONG 3

void ChangePaletteAndSettingsPeriodically() {
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand ==  0)  { selectRandomPalette();         speed = 5; scale = 100; colorLoop = 1; }
    if( secondHand == 15)  { selectRandomPalette();             speed = 10; scale = 50; colorLoop = 1; }
    if( secondHand == 30)  { selectRandomPalette();       speed = 30; scale = 10; colorLoop = 1; }
    if( secondHand == 45)  { selectRandomPalette();         speed =  8; scale = 3; colorLoop = 1; }
    if( secondHand == 60)  { selectRandomPalette();          speed =  74; scale = 100; colorLoop = 1; }
    if( secondHand == 75)  { selectRandomPalette();           speed =  48; scale = 300; colorLoop = 1; }
    if( secondHand == 90)  { SetupRandomPalette();           speed = 20; scale = 37; colorLoop = 1; }
    if( secondHand == 105)  { selectRandomPalette();          speed = 20; scale = 75; colorLoop = 1; }
    if( secondHand == 120)  { SetupRandomPalette();                     speed = 5; scale = 20; colorLoop = 1; }
    if( secondHand == 135)  { SetupRandomPalette();                     speed = 50; scale = 50; colorLoop = 1; }
    if( secondHand == 150)  { SetupRandomPalette();                     speed = 90; scale = 90; colorLoop = 1; }
    if( secondHand == 165)  { SetupRandomPalette();  speed = 30; scale = 20; colorLoop = 1; }
  }
}
void SetupRandomPalette() {
  currentPalette = CRGBPalette16( 
    CHSV( random8(), 255, 32), 
    CHSV( random8(), 255, 255), 
    CHSV( random8(), 128, 255), 
    CHSV( random8(), 255, 255)); 
}

void selectRandomPalette() {

  switch(random8(41)) {
    case 0:
    currentPalette = CloudColors_p;
    break;
    
    case 1:
    currentPalette = LavaColors_p;
    break;
    
    case 2:
    currentPalette = OceanColors_p;
    break;
    
    case 4:
    currentPalette = ForestColors_p;
    break;
    
    case 5:
    currentPalette = RainbowColors_p;
    break;
    
    case 6:
    currentPalette = PartyColors_p;
    break;
    
    case 7:
    currentPalette = HeatColors_p;
    break;
    
    case 8:
    currentPalette = Sunset_Real_gp;
    break;
    
    case 9:
    currentPalette = es_rivendell_15_gp;
    break;
    
    case 10:
    currentPalette = es_ocean_breeze_036_gp;
    break;
    
    case 11:
    currentPalette = rgi_15_gp;
    break;
    
    case 12:
    currentPalette = retro2_16_gp;
    break;
    
    case 13:
    currentPalette = Analogous_1_gp;
    break;
    
    case 14:
    currentPalette = es_pinksplash_08_gp;
    break;
    
    case 15:
    currentPalette = Coral_reef_gp;
    break;
    
    case 16:
    currentPalette = es_ocean_breeze_068_gp;
    break;
    
    case 17:
    currentPalette = es_pinksplash_07_gp;
    break;
    
    case 18:
    currentPalette = es_vintage_01_gp;
    break;
    
    case 19:
    currentPalette = departure_gp;
    break;
    
    case 20:
    currentPalette = es_landscape_64_gp;
    break;
    
    case 21:
    currentPalette = es_landscape_33_gp;
    break;
    
    case 22:
    currentPalette = rainbowsherbet_gp;
    break;
    
    case 23:
    currentPalette = gr65_hult_gp;
    break;
    
    case 24:
    currentPalette = gr64_hult_gp;
    break;
    
    case 25:
    currentPalette = GMT_drywet_gp;
    break;
    
    case 26:
    currentPalette = ib_jul01_gp;
    break;
    
    case 27:
    currentPalette = es_vintage_57_gp;
    break;
    
    case 28:
    currentPalette = ib15_gp;
    break;
    
    case 29:
    currentPalette = Fuschia_7_gp;
    break;
    
    case 30:
    currentPalette = es_emerald_dragon_08_gp;
    break;
    
    case 31:
    currentPalette = lava_gp;
    break;
    
    case 32:
    currentPalette = fire_gp;
    break;
    
    case 33:
    currentPalette = Colorfull_gp;
    break;
    
    case 34:
    currentPalette = Magenta_Evening_gp;
    break;
    
    case 35:
    currentPalette = Pink_Purple_gp;
    break;
    
    case 36:
    currentPalette = es_autumn_19_gp;
    break;
    
    case 37:
    currentPalette = BlacK_Blue_Magenta_White_gp;
    break;
    
    case 38:
    currentPalette = BlacK_Magenta_Red_gp;
    break;
    
    case 39:
    currentPalette = BlacK_Red_Magenta_Yellow_gp;
    break;
    
    case 40:
    currentPalette = Blue_Cyan_Yellow_gp;
    break;
 
  }
}

//
// Mark's xy coordinate mapping code.  See the XYMatrix for more information on it.
//
uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }
  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  return i;
}
