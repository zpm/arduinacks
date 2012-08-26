unsigned long currentMicros = 0;

const int RED = 0;
const int GREEN = 1;
const int BLUE = 2;

int ledPin[] = { 2, 3, 4 };

// -------------------- zpm's pwm implementation --------------------- //

const unsigned long cycleMicros = 4000; // so 8000 us is 120 Hz
un
unsigned long cycleOffAt[] = { 0, 0, 0 };
unsigned long cycleAllOnAt = 0;

// value - between 0 and 100
void setBrightness(int color, int value) {
  cycleOnTime[color] = long(value) * 40;
}

void writeRGB() {
  if (currentMicros > cycleAllOnAt) {
    if (cycleOnTime[RED] != 0) digitalWrite(ledPin[RED], HIGH);
    if (cycleOnTime[GREEN] != 0) digitalWrite(ledPin[GREEN], HIGH);
    if (cycleOnTime[BLUE] != 0) digitalWrite(ledPin[BLUE], HIGH);
    cycleAllOnAt = currentMicros + cycleMicros;
    cycleOffAt[RED] = currentMicros +
  } else {
    if (currentMicros > cycleOffAt[RED])
    
    
  }
  for (int i=0; i<=2; i++) {
    if (currentMicros > cycleAllOnAt) {
      if (states[i] == LOW) {
        if (cycleOnTime[i] > 0) {
          states[i] = HIGH;
          digitalWrite(i+2, states[i]);
        }
        // time to turn back off
        cycleNext[i] = currentMicros + cycleOnTime[i];
      } else if (states[i] == HIGH) {
        if (cycleOnTime[i] != cycleMicros) {
          states[i] = LOW;
          digitalWrite(i+2, states[i]);
        }
        // time to turn back on
        cycleNext[i] = currentMicros + (cycleMicros - cycleOnTime[i]);
      }
    }
  }
}

// -------------------- zpm's pwm implementation --------------------- //

// -------------------- zpm's fader implementation --------------------- //

static int patternLen = 6;
static int pattern[6][3] = {
  {10, 0, 0},
  {10, 0, 10},
  {0, 0, 10},
  {0, 10, 10},
  {0, 10, 0},
  {10, 10, 0},
};

static int transSteps = 100;
static long transTimePerStep = 2000000 / transSteps;

int at = 0;
int next = 1;
int stepNo = 0;
long transNext = 0;

void transitionRGB() {
  if (currentMicros > transNext) {
    stepNo += 1;
    if (stepNo >= transSteps) {
      // go to next step
      at = (at + 1) % patternLen;
      next = (at + 1) % patternLen;
      stepNo = 0;
    }
    for (int i=0; i<=2; i++) {
      long delta = ((pattern[next][i] - pattern[at][i]) * stepNo) / transSteps;
      setBrightness(i, pattern[at][i] + delta);
      //Serial.print(pattern[at][i] + delta);
      //Serial.print("\t");
    }
    //Serial.println();
    transNext = currentMicros + transTimePerStep;
  }
}

// -------------------- zpm's fader implementation --------------------- //

void setup() {
  // set the digital pin as output:
  pinMode(ledPin[RED], OUTPUT);
  pinMode(ledPin[GREEN], OUTPUT);
  pinMode(ledPin[BLUE], OUTPUT);
  // initialize serial port:
  Serial.begin(9600);
  // initial set
  setBrightness(RED, 99);
  setBrightness(BLUE, 0);
  setBrightness(GREEN, 0);
}

void loop() {
  currentMicros = micros();
  //transitionRGB();
  writeRGB();
}
