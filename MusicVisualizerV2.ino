#include <FastLED.h>

//----------LED LIGHTING SETUP-----------------
#define LED_PIN 7
#define NUM_LEDS 95
#define BRIGHTNESS 64
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define LED_UPDATES_PER_SECOND 100
#define MAX_LED_ARM 7
CRGB leds[NUM_LEDS];

int sound_sensor = A0;     // assign to pin A2
int k = 255;               // COLOR WHEEL POSITION
unsigned long decay = 55; // HOW MANY MS BEFORE ONE LIGHT DECAY

//------------MICROPHONE SETUP-----------------
const int sampleWindow = 5;
unsigned int sample;
float soundValue;
unsigned long silenceValue = 4; //Consider 0 - 3 to be silence.
float maxSoundValue = 36;

//--------- LED RUNTIME------------------------
unsigned long lastLEDUpdate;
unsigned long lastReactUpdate;
unsigned int currentLevel = 0;     // current reaction. (no. LED lit)
unsigned int thresholdLevel = 1;    //Min level of LEDs to turn on

//Level can move past number of LEDs for delay effect when sound is high. 
unsigned int max_level = 13;    

//---------RAINBOW WAVE SETTINGS----------------
int wheel_speed = 1;

//------------LED GROUPS------------------------
unsigned int groups[8]= {8, 5, 9,6,10,4,7,5};

void setup()
{
    delay(3000); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    //Clear LEDs.
    for (int i = 0; i < NUM_LEDS; i++)
        leds[i] = CRGB(0, 0, 0);
    FastLED.show();

    //Begin serial
    Serial.begin(9600);
}

//Update LED by groups.
void UpdateGroups()
{
    int ledIndex = 0;

    for (int group : groups)
    {
        UpdateLEDGroup(ledIndex, group);
        ledIndex += group;
    }


    CRGB newColor = Scroll((ledIndex * 256 / 50 + k) % 256);
    int innerCicle = 17;
  

   
    
    if(currentLevel >= thresholdLevel){
        Fill(ledIndex, innerCicle, newColor);
        Fill(92, 3, newColor);
    }else{
      Fill(ledIndex, innerCicle, CRGB(0, 0, 0));
       Fill(92, 3, CRGB(0, 0, 0));
    }
    
    newColor = Scroll(((ledIndex+1) * 256 / 50 + k) % 256);
    ledIndex += innerCicle;
    
    int remaining = NUM_LEDS - ledIndex - 3;
    
    if(currentLevel >= 2){
        Fill(ledIndex, remaining, newColor);
    }else{
      Fill(ledIndex, remaining, CRGB(0, 0, 0));
    }
    
}

void Fill(int _startIndex, int _amt, CRGB _color ){
    for(int i = 0; i < _amt; i++){
          leds[_startIndex + i] = _color;
    }
}


void UpdateLEDGroup(int _startIndex, int _amt)
{
    for (int i = _amt - 1; i >= 0; i--)
    {
    
        if (i < currentLevel)
            leds[_startIndex + i] = Scroll((i * 256 / 50 + k) % 256);
        else
            leds[_startIndex + i] = CRGB(0, 0, 0);
    }
}


void UpdateLEDs()
{
    unsigned long currentTime = millis();

    if ((currentTime - lastLEDUpdate) > (1000 / LED_UPDATES_PER_SECOND))
    {
        UpdateGroups();
        lastLEDUpdate = currentTime;
        FastLED.show();
        k = k - wheel_speed; // SPEED OF COLOR WHEEL
        if (k < 0)           // RESET COLOR WHEEL
            k = 255;
    }
}

void UpdateSoundValue()
{
    unsigned long startMillis = millis();
    unsigned long signalMax = 0;
    unsigned long signalMin = analogRead(sound_sensor);
    unsigned long peakToPeak = 0;

    while (millis() - startMillis < sampleWindow)
    {
        sample = analogRead(sound_sensor);
        signalMin = min(signalMin, sample);
        signalMax = max(signalMax, sample);
    }
    peakToPeak = signalMax - signalMin;
    soundValue -= silenceValue; 
    
    soundValue = constrain(peakToPeak, 0, maxSoundValue);
    //
}

//Update music reaction level.
void UpdateLevel()
{
   unsigned long currentTime = millis();
   if ((currentTime - lastReactUpdate) > decay)
   {
       lastReactUpdate = currentTime;
       if (currentLevel > 0)
           currentLevel--;
   }
   
   //New level
  unsigned int level = max_level * (float)(soundValue / maxSoundValue);
  //Serial.println((float)(soundValue / maxSoundValue));
  //Only update if the  level is at least 2 LEDs.
  //And if currentLevel has decreased back to 1, OR 
  //The new level is significantly higher than current level. (at least 2).
  if (level >= thresholdLevel &&  (currentLevel < 1 || level >= currentLevel + 3))
  {
    currentLevel = level;
  }

 
}
void loop()
{
    UpdateSoundValue();
    UpdateLevel();
    UpdateLEDs();
}


// FUNCTION TO GENERATE COLOR BASED ON VIRTUAL WHEEL
// https://github.com/NeverPlayLegit/Rainbow-Fader-FastLED/blob/master/rainbow.ino
CRGB Scroll(int pos)
{
    CRGB color(0, 0, 0);
    if (pos < 85)
    {
        color.g = 0;
        color.r = ((float)pos / 85.0f) * 255.0f;
        color.b = 255 - color.r;
    }
    else if (pos < 170)
    {
        color.g = ((float)(pos - 85) / 85.0f) * 255.0f;
        color.r = 255 - color.g;
        color.b = 0;
    }
    else if (pos < 256)
    {
        color.b = ((float)(pos - 170) / 85.0f) * 255.0f;
        color.g = 255 - color.b;
        color.r = 1;
    }
    return color;
}
