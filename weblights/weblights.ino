#include <SPI.h>
#include <Ethernet.h>

// these must be PWM pins
#define pinRED 3
#define pinGREEN 5
#define pinBLUE 6
// create map so it's easy to iterate over these vals with a loop
const int pinMAP[3] = {pinRED, pinBLUE, pinGREEN};

// variables set by ethernet used to control internal modes
int mode = 0;
long red = 0;
long green = 0;
long blue = 0;
long steps = 0;
long timePerStep = 12000;
long brightness = 255;

#define mOFF 0
#define mRGB 1
#define mFADE 2


// ==================== start ethernet sheild ==================== //

EthernetServer server(80);
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x30, 0xB7 };

#define ETHERNETBUFFERMAX 128
int ethernetBufferSize;
char ethernetBuffer[ETHERNETBUFFERMAX];

void ethernetSetup() {

  // start the Ethernet connection and the server:
  Ethernet.begin(mac);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

}

void ethernetWaitForRequest(EthernetClient client) {
  ethernetBufferSize = 0;
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n')
        break;
      else
        if (ethernetBufferSize < ETHERNETBUFFERMAX)
          ethernetBuffer[ethernetBufferSize++] = c;
        else
          break;
    }
  }
}

// tries to parse arguments out of the url. returns false if anything fails
boolean ethernetParseRequest() {

  if (strncmp(ethernetBuffer, "POST /", 6) != 0) {
    // if this isn't a post, return false
    return 0;
  }

  // use these temp variables until we know which mode is specified
  long temp1 = 0;
  long temp2 = 0;
  long temp3 = 0;

  // find terminating space
  char* spaceAt = strstr(&ethernetBuffer[6], " ") + 1;

  // setup temp variable
  char tempString[255];

  // find all the slashes
  char* slashAt1 = strstr(&ethernetBuffer[6], "/") + 1;
  if (slashAt1 < spaceAt) {
    char* slashAt2 = strstr(slashAt1, "/") + 1;
    if (slashAt2 < spaceAt) {
      char* slashAt3 = strstr(slashAt2, "/") + 1;
      if (slashAt3 < spaceAt) {
        char* slashAt4 = strstr(slashAt3, "/") + 1;
        if (slashAt4 < spaceAt) {
          tempString[0] = 0;
          strncat(tempString, slashAt3, slashAt4-slashAt3-1);
        } else {
          tempString[0] = 0;
          strncat(tempString, slashAt3, spaceAt-slashAt3-1);
        }
        temp3 = atol(tempString);
        tempString[0] = 0;
        strncat(tempString, slashAt2, slashAt3-slashAt2-1);
      } else {
        tempString[0] = 0;
        strncat(tempString, slashAt2, spaceAt-slashAt2-1);
      }
      temp2 = atol(tempString);
      tempString[0] = 0;
      strncat(tempString, slashAt1, slashAt2-slashAt1-1);
    } else {
      tempString[0] = 0;
      strncat(tempString, slashAt1, spaceAt-slashAt1-1);
    }
    temp1 = atol(tempString);
    tempString[0] = 0;
    strncat(tempString, &ethernetBuffer[6], slashAt1-&ethernetBuffer[6]-1);
  } else {
    tempString[0] = 0;
    strncat(tempString, &ethernetBuffer[6], spaceAt-&ethernetBuffer[6]-1);
  }

  // set all the variables now
  if (!strncmp(tempString, "off", 3) != 0) {
    mode = mOFF;
  } else if (strncmp(tempString, "rgb", 3) == 0) {
    mode = mRGB;  
    red = temp1;
    green = temp2;
    blue = temp3;
  } else if (strncmp(tempString, "fade", 4) == 0) {
    mode = mFADE;
    steps = temp1;
    timePerStep = temp2;
  }

  Serial.print("SETTING\t");
  Serial.print(mode);
  Serial.print("\t");
  Serial.print(red);
  Serial.print("\t");
  Serial.print(green);
  Serial.print("\t");
  Serial.print(blue);
  Serial.print("\n");

  // parsing was successful
  return 1;

}

void ethernetLoop() {

  EthernetClient client = server.available();
  if (client) {
    ethernetWaitForRequest(client);

    // respond so client can report properly
    if (ethernetParseRequest()) {
      client.println("HTTP/1.1 200 OK");
    } else {
      client.println("HTTP/1.1 400 BAD REQUEST");
    }
    client.println("Content-Type: text/html");
    client.println("Connnection: close");
    client.println();
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();

  }

}

// ==================== end ethernet sheild ==================== //


// ==================== begin led controller ==================== //


long atRGB[] = {0, 0, 0};
long toRGB[] = {255, 0, 255};
long ledFaderStep = 1;
long ledFaderNextEvent = 0;
boolean ledFaderPulseState = true;

// ledFaderStyle determines how the lights fade:
//   STYLE_PURE: use set values, limited to ledVolume
//   STYLE_PULSE: use set values, limited to ledVolume, returning to (0,0,0) between each one
//   STYLE_CONSTANT: normalize all channels to the value set by ledVolume; (1,1,0) becomes (128,128,0)
#define STYLE_PURE 0
#define STYLE_PULSE 1
#define STYLE_CONSTANT 2
int ledFaderStyle = STYLE_CONSTANT;

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

void ledFader(long ledFaderSteps, long transTimePerStepInMicros) {
  
  long ledFaderDelayWhileAtTarget = transTimePerStepInMicros * 100;
  long currentMicros = micros();
  if (currentMicros > ledFaderNextEvent) {
    
    // set next transition event
    ledFaderNextEvent = currentMicros + transTimePerStepInMicros;

    // if done transitioning, change target
    if (ledFaderStep == ledFaderSteps) {
      // add delay at last step
      ledFaderNextEvent += ledFaderDelayWhileAtTarget;
    } else if (ledFaderStep > ledFaderSteps) {
      // copy ledToRGB into ledAtRGB
      for (int i = 0; i <= 2; i++) {
        atRGB[i] = toRGB[i];
      }
      // pick next color
      if ((ledFaderStyle == STYLE_PULSE) && ledFaderPulseState) {
        for (int i = 0; i <= 2; i++) {
          toRGB[i] = 0;
        }
      } else {
        ledSetNextColor();
      }
      ledFaderPulseState = !ledFaderPulseState;
      // reset step number
      ledFaderStep = 1;
      // add extra delay at target
    }

    // set lights
    long newVal[3];
    for (int i = 0; i <= 2; i++) {
      newVal[i] = atRGB[i] + ((toRGB[i] - atRGB[i]) * ledFaderStep) / ledFaderSteps;
      if (ledFaderStyle != STYLE_CONSTANT) {
        newVal[i] = newVal[i] * brightness / 255;
      }
    }

    // limit to constant power
    if (ledFaderStyle == STYLE_CONSTANT) {
      long totalVal = newVal[0] + newVal[1] + newVal[2];
      if (totalVal > 0) { // avoid dividing by zero
        for (int i = 0; i <= 2; i++) {
          newVal[i] = newVal[i] * brightness / totalVal;
        }
      }
    }

    // write
    analogWrite(pinRED, newVal[0]);
    analogWrite(pinBLUE, newVal[1]);
    analogWrite(pinGREEN, newVal[2]);

    ledFaderStep += 1;

  }
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

void ledSetNextColor() {

  if (ledNextMode == NEXT_STATIC) {
    
    toRGB[0] = ledStaticColors[0];
    toRGB[1] = ledStaticColors[1];
    toRGB[2] = ledStaticColors[2];

  } else if ((ledNextMode == NEXT_BASIS_ORDERED) || (ledNextMode == NEXT_BASIS_RANDOM)) {

    // basis rotates through turning channels on and off as defined in basisBase.
    // channels that are on (==1) are multiplied by ledMax less a randomization factor.
    // basis always guarantees at least one channel is on and at least one is off.
    // basis rotates through in a predicatable pattern, crazybasis picks phases at random,
    // but ensuring that no phase is picked twice in a row

    // set next target based off of basisNextPhase
    for (int i=0; i<=2; i++) {
      int target = brightness;
      if (basisRandomization > 0) {
        target -= random(0, basisRandomization);
        target = max(1, target);
      }
      toRGB[i] = basisBase[basisNextPhase][i] * target;
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
      toRGB[i] = random(0,255);
    }

  }
  
}

void ledcontrollerLoop() {
  
  // this function assumes that the variables have been written successfully by the ethernet
  // sheild and parses functionality out of them accordingly
  
  // static functions
  
  if (mode == mOFF) {
    analogWrite(pinRED, 0);
    analogWrite(pinBLUE, 0);
    analogWrite(pinGREEN, 0);
   
  } else if (mode == mRGB) {
    analogWrite(pinRED, red);
    analogWrite(pinBLUE, blue);
    analogWrite(pinGREEN, green);

  }

  // fading functions that utilize ledSetNextTarget()
  // transSteps, "smoothness" - number of steps in the transition. 0 is flash instantly
  // transTimePerStepInMicros, 1000000 = 1 sec. effective minimum ~400 us
  // effectively, this means cycle time is transSteps*transTimePerStepInMicros

  else if (mode == mFADE ) {
 
    // transSteps is number of steps
    // transTimePerStepInMicros is number of microseconds per step
    ledFader(steps, timePerStep);

  }

}

// ==================== end led controller ==================== //

void setup() {

  Serial.begin(9600);

  ethernetSetup();

  // setup output pins on pwm
  pinMode(pinRED, OUTPUT);
  pinMode(pinBLUE, OUTPUT);
  pinMode(pinGREEN, OUTPUT);
  
  // used to reset to 0 here, but prefer to not to keep last setting

}

void loop() {

  ethernetLoop();
  ledcontrollerLoop();

}


