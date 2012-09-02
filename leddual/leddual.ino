// debug enables serial and spams it with debug information. serial takes a really
// long time, so enabling debug will significantly impact fade/flash times at low settings
#define DEBUG true

// create map of pwm pins so it's easy to set these with a loop
// syntax to call: pinMap[channelNumber][r=0, g=1, b=2]
#define RGBCHANNELS 2
const int pinMap[RGBCHANNELS][3] = {
  {3, 5, 6},
  {9, 10, 11}
};

long controlArg0 = 0;
long controlArg1 = 0;
long controlArg2 = 0;
long controlArg3 = 0;


// ==================== begin led controller ==================== //


#define ledMax 255

int ledAtRGB[] = {0, 0, 0};
int ledToRGB[] = {0, 0, 0};


// ledNextMode controls how the next colors are picked:
//   NEXT_STATIC: uses ledStaticColors to set the next color the same every cycle
//   NEXT_BASIS_ORDERED: uses basisBase, a set of colors to transition through that ensure
//                       at least one color is always on and at least one is always off
//   NEXT_BASIS_RANDOM: uses basisBase, but selects a phase randomly instead of cycling through
//   NEXT_RANDOM: purely random next color, but contains a lot of white-ish values eg. (123, 113, 135)
#define NEXT_STATIC 0
#define NEXT_BASIS_ORDERED 1
#define NEXT_BASIS_RANDOM 2
#define NEXT_RANDOM 3
int ledNextMode = NEXT_BASIS_ORDERED;

// ledStaticColors controls the colors that will be set if NEXT_STATIC is selected as the mode.
// these colors have no effect on any of the other modes
int ledStaticColors[3] = {255, 0, 255};

// ledBasisRandomization controls how random the colors will be in either of the BASIS modes.
// a random value betwene 0 and this number is subtracted from ledMax.
int basisRandomization = 0;

// ledFaderSteps controls the smoothness of the fade. for smooth fading, this should be set to
// ledMax (so when fading from 0-255, each value in the scale will have a step). setting larger
// than ledMax is unnecessary, use ledFaderTimePerStepInMicros instead. if this number is less than
// ledMax, the fade will be sharper. setting to 1 means there is no fade.
long ledFaderSteps = ledMax;

// setting this to zero will fade as fast as the arduino can handle it, which turns out to
// be pretty damn fast if there's no other processing going on. 1 second / 255 steps = 3921 micros
long ledFaderTimePerStepInMicros = 4000 * 3;

// ledFaderDelayWhileAtTarget pauses briefly after reaching one of the target steps, which
// makes the transition appear much smoother. this pauses briefly a the point that the colors
// reverse directions (reversing instaneously makes the leds appear to "pulse" intermittently)
long ledFaderDelayWhileAtTarget = ledFaderTimePerStepInMicros * 50;

// ledFaderStyle determines how the lights fade:
//   STYLE_PURE: use set values, limited to ledVolume
//   STYLE_PULSE: use set values, limited to ledVolume, returning to (0,0,0) between each one
//   STYLE_CONSTANT: normalize all channels to the value set by ledVolume; (1,1,0) becomes (128,128,0)
#define STYLE_PURE 0
#define STYLE_PULSE 1
#define STYLE_CONSTANT 2
int ledFaderStyle = STYLE_CONSTANT;

// ledVolume sets the maximum power per channel, or if STYLE_CONSTANT is selected, the maximum power
// summed across all three channels. the default is 255 for brightest possible.
int ledVolume = ledMax;


// sets a channel given rgb values
void ledWrite(int channel, int r, int g, int b) {
  analogWrite(pinMap[channel][0], r);
  analogWrite(pinMap[channel][1], g);
  analogWrite(pinMap[channel][2], b);
}


// ledSetNextColor is called after ledFaderLoop runs through all of its transition steps;
// values here always assume a 0-255 scale

#define BASISPHASES 6
int basisNextPhase = 0;
static int basisBase[BASISPHASES][3] = {
  { 0, 0, 1 },
  { 0, 1, 1 },
  { 0, 1, 0 },
  { 1, 1, 0 },
  { 1, 0, 0 },
  { 1, 0, 1 }
};
int pulsarPhase = 0;

void ledSetNextColor() {

  if (DEBUG) {
    Serial.print("ledSetNextColor with ledNextMode = ");
    Serial.println(ledNextMode);
  }

  if (ledNextMode == NEXT_STATIC) {
    
    ledToRGB[0] = ledStaticColors[0];
    ledToRGB[1] = ledStaticColors[1];
    ledToRGB[2] = ledStaticColors[2];

  } else if ((ledNextMode == NEXT_BASIS_ORDERED) || (ledNextMode == NEXT_BASIS_RANDOM)) {

    // basis rotates through turning channels on and off as defined in basisBase.
    // channels that are on (==1) are multiplied by ledMax less a randomization factor.
    // basis always guarantees at least one channel is on and at least one is off.
    // basis rotates through in a predicatable pattern, crazybasis picks phases at random,
    // but ensuring that no phase is picked twice in a row

    // set next target based off of basisNextPhase
    for (int i=0; i<=2; i++) {
      int target = ledMax;
      if (basisRandomization > 0) {
        target -= random(0, basisRandomization);
        target = max(1, target);
      }
      ledToRGB[i] = basisBase[basisNextPhase][i] * target;
    }

    // pick basisNextPhase
    if (ledNextMode == NEXT_BASIS_RANDOM) {
      int newBasis = random(0, BASISPHASES);
      // if the randomly selected one is the same as the current one,
      // just pick the one after the current one to ensure we change
      if (newBasis == basisNextPhase) {
        newBasis = (basisNextPhase+1) % BASISPHASES;
      }
      basisNextPhase = newBasis;
    } else {
      basisNextPhase = (basisNextPhase+1) % BASISPHASES;
    }

  } else if (ledNextMode == NEXT_RANDOM) {
    
    // purerandom provides three completely random channels. because these channels are
    // completely random, unfortunately, a lot of these colors turn out to be white
    // because values like (120, 114, 127) can often occur. to get random pure colors,
    // use CRAZYBASIS, which avoides white by ensuring one channel will always be off.
    for(int i=0; i<=2; i++) {
      ledToRGB[i] = random(0,255);
    }

  }
  
  if (DEBUG) {
    Serial.print("\tFrom:\t");
    for (int i=0; i<=2; i++) {
      Serial.print(ledAtRGB[i]);
      Serial.print("\t");
    }
    Serial.println();
    Serial.print("\tTo:\t");
    for (int i=0; i<=2; i++) {
      Serial.print(ledToRGB[i]);
      Serial.print("\t");
    }
    Serial.println();
  }
  
}


long ledFaderStep = 1;
long ledFaderNextEvent = 0;
boolean ledFaderPulseState = true;

void ledFaderLoop() {
  
  long currentMicros = micros();
  if (currentMicros > ledFaderNextEvent) {
    
    // set next transition event
    ledFaderNextEvent = currentMicros + ledFaderTimePerStepInMicros;

    // if done transitioning, change target
    if (ledFaderStep == ledFaderSteps) {
      // add delay at last step
      ledFaderNextEvent += ledFaderDelayWhileAtTarget;
    } else if (ledFaderStep > ledFaderSteps) {
      // copy ledToRGB into ledAtRGB
      for (int i = 0; i <= 2; i++) {
        ledAtRGB[i] = ledToRGB[i];
      }
      // pick next color
      if ((ledFaderStyle == STYLE_PULSE) && ledFaderPulseState) {
        if (DEBUG) Serial.println("Pulsing to zero.");
        for (int i = 0; i <= 2; i++) {
          ledToRGB[i] = 0;
        }
      } else {
        ledSetNextColor();
      }
      ledFaderPulseState = !ledFaderPulseState;
      // reset step number
      ledFaderStep = 1;
      // add extra delay at target
    }

    if (DEBUG) {
      Serial.print(ledFaderStep);
      Serial.print("/");
      Serial.print(ledFaderSteps);
      Serial.print("\t");
    }

    // set lights
    long newVal[3];
    for (int i = 0; i <= 2; i++) {
      newVal[i] = ledAtRGB[i] + ((ledToRGB[i] - ledAtRGB[i]) * ledFaderStep) / ledFaderSteps;
    } 

    // do constant power calculations
    if (ledFaderStyle == STYLE_CONSTANT) {
      long totalVal = newVal[0] + newVal[1] + newVal[2];
      if (totalVal > 0) { // avoid dividing by zero
        for (int i = 0; i <= 2; i++) {
          newVal[i] = newVal[i] * ledVolume / totalVal;
        }
      }
    }

    if (DEBUG) {
      for (int i=0; i<=2; i++) {
        Serial.print(newVal[i]);
        Serial.print("\t");
      }
      Serial.println();
    }

    // write
    ledWrite(0, newVal[0], newVal[1], newVal[2]);
    ledWrite(1, newVal[0], newVal[1], newVal[2]);

    ledFaderStep += 1;

  }
}

// force starts the system by picking the first to values given the default settings
void ledFaderSetup() {
  ledSetNextColor();
  for (int i = 0; i <= 2; i++) {
    ledAtRGB[i] = ledToRGB[i];
  }
  ledSetNextColor();
}


// ==================== end led controller ==================== //


void setup() {

  if (DEBUG) {
    Serial.begin(9600);
    Serial.println("\n\n=========\n= RESET =\n=========\n\n");
  }

  // setup output pins on pwm
  for (int i=0; i<RGBCHANNELS; i++) {
    for (int j=0; j<=2; j++) {
      pinMode(pinMap[i][j], OUTPUT);
      analogWrite(pinMap[i][j], 0);
    }
  }

  ledFaderSetup();
  
}

void loop() {
  ledFaderLoop();
}


