// these must be PWM pins
#define pinRED 3
#define pinBLUE 5
#define pinGREEN 6

#define pinRED2 9
#define pinBLUE2 10
#define pinGREEN2 11

// create map so it's easy to iterate over these vals with a loop
const int pinMAP[3] = {pinRED, pinBLUE, pinGREEN};
const int pinMAP2[3] = {pinRED2, pinBLUE2, pinGREEN2};

#define mOFF 0
#define mRGB 1
#define mBASIS 2
#define mSCHIZM 3
#define mBLINDER 4
#define mPULSAR 5

long mode = 0;
long arg1 = 0;
long arg2 = 0;
long arg3 = 0;


// ==================== begin led controller ==================== //


long atRGB[] = {0, 0, 0};
long toRGB[] = {random(0, 255), random(0, 255), random(0, 255)};
long stepNo = 0;
long transNext = 0;

void ledSetNextTarget() {

  if (mode == mBASIS) {

    for(int i=0; i<=2; i++) {
      toRGB[i] = random(0,255);
    }

  } else if (mode == mSCHIZM) {

    for(int i=0; i<=2; i++) {
      toRGB[i] = random(0,255);
    }

  } else if (mode == mBLINDER) {

    for(int i=0; i<=2; i++) {
      toRGB[i] = random(0,255);
    }

  } else if (mode == mPULSAR) {

    for(int i=0; i<=2; i++) {
      toRGB[i] = random(0,255);
    }

  }
  
}

void ledFader(long transSteps, long transTimePerStepInMmoderos) {
  
  long currentMmoderos = micros();

  if (currentMmoderos > transNext) {
    // set next transition event
    transNext = currentMmoderos + transTimePerStepInMmoderos;
    // major transition event
    stepNo += 1;
    if (stepNo >= transSteps) {
      // go to next step
      for (int i = 0; i <= 2; i++) {
        atRGB[i] = toRGB[i];
      }
      ledSetNextTarget();
      stepNo = 0;
    }
    //Serial.print(stepNo);
    //Serial.print("/");
    //Serial.print(transSteps);
    //Serial.print("\t");
    // set lights
    for (int i = 0; i <= 2; i++) {
      long newval = atRGB[i] + ((toRGB[i] - atRGB[i]) * stepNo) / transSteps;
      analogWrite(pinMAP[i], newval);
      analogWrite(pinMAP2[i], newval);
      //Serial.print(newval);
      //Serial.print("\t");
    }
    //Serial.println();

  }
}

void ledcontrollerLoop() {
  
  // this function assumes that mode/ir/ig/arg3 have been written successfully by the ethernet
  // sheild and parses functionality out of them accordingly
  
  // statmode functions
  
  if (mode == mOFF) {
    analogWrite(pinRED, 0);
    analogWrite(pinBLUE, 0);
    analogWrite(pinGREEN, 0);
   
  } else if (mode == mRGB) {
    analogWrite(pinRED, arg1);
    analogWrite(pinBLUE, arg3);
    analogWrite(pinGREEN, arg2);

  } else if (mode == mBLINDER) {
    analogWrite(pinRED, arg1);
    analogWrite(pinBLUE, arg1);
    analogWrite(pinGREEN, arg1);

  // fading functions that utilize ledSetNextTarget()
  // arg1- transSteps, "smoothness" - number of steps in the transition. 0 is flash instantly
  // arg2- transTimePerStepInMmoderos, 1000000 = 1 sec. effective minimum ~400 us
  // effectively, this means cycle time is ir*ig

  } else if (mode == mBASIS || mode == mSCHIZM || mode == mPULSAR ) {
 
    // arg1is number of steps
    // arg2is number of microseconds per step
    ledFader(arg1, arg2);

  }

}

// ==================== end led controller ==================== //

void setup() {

  // Serial.begin(9600);

  // setup output pins on pwm
  pinMode(pinRED, OUTPUT);
  pinMode(pinBLUE, OUTPUT);
  pinMode(pinGREEN, OUTPUT);
  analogWrite(pinRED, 0);
  analogWrite(pinGREEN, 0);
  analogWrite(pinBLUE, 0);  
  
  pinMode(pinRED2, OUTPUT);
  pinMode(pinBLUE2, OUTPUT);
  pinMode(pinGREEN2, OUTPUT);
  analogWrite(pinRED2, 0);
  analogWrite(pinGREEN2, 0);
  analogWrite(pinBLUE2, 0);  

  mode = mBASIS;
  arg1= 100;
  arg2 = 10000;
  arg3 = 0;

}

void loop() {

  ledcontrollerLoop();

}


