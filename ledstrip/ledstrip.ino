#include <SPI.h>
#include <Ethernet.h>

unsigned long currentMicros = 0;

const int RED = 0;
const int GREEN = 1;
const int BLUE = 2;

int newcolors[] = {0, 0, 0};

// -------------------- zpm's pwm implementation --------------------- //

// minimum value to do something without flicker seems to be 200 us

const unsigned long cycleMicros = 4000; // 8000 us is 120 Hz
unsigned long cycleOnFor[] = { 0, 0, 0 };
unsigned long cycleOffAt[] = { 0, 0, 0 };
unsigned long cycleAllOnAt = 0;

void setBrightness(int color, int value) {
  cycleOnFor[color] = long(value) * 4;
}

void writeRGB() {
  if (currentMicros >= cycleAllOnAt) {
    if (cycleOnFor[RED] != 0) digitalWrite(2, HIGH);
    if (cycleOnFor[GREEN] != 0) digitalWrite(3, HIGH);
    if (cycleOnFor[BLUE] != 0) digitalWrite(4, HIGH);
    // set times to turn off colors
    cycleOffAt[RED] = currentMicros + cycleOnFor[RED];
    cycleOffAt[GREEN] = currentMicros + cycleOnFor[GREEN];
    cycleOffAt[BLUE] = currentMicros + cycleOnFor[BLUE];
    // set times to turn them all back on
    cycleAllOnAt = currentMicros + cycleMicros;
  } else {
    if (currentMicros >= cycleOffAt[RED]) digitalWrite(2, LOW);
    if (currentMicros >= cycleOffAt[GREEN]) digitalWrite(3, LOW);
    if (currentMicros >= cycleOffAt[BLUE]) digitalWrite(4, LOW);
  }
}

// -------------------- zpm's pwm implementation --------------------- //

// -------------------- zpm's fader implementation --------------------- //

static long transTime = 5000000;
static long transSteps = 100;
static long transTimePerStep = transTime / transSteps;

long atRGB[] = {0, 0, 0};
long toRGB[] = {random(0, 1000), random(0, 1000), random(0, 1000)};
long stepNo = 0;
long transNext = 0;

void transitionRGB() {
  if (currentMicros > transNext) {
    // set next transition event
    transNext = currentMicros + transTimePerStep;
    // major transition event
    stepNo += 1;
    if (stepNo >= transSteps) {
      // go to next step
      for (int i = 0; i <= 2; i++) {
        atRGB[i] = toRGB[i];
        toRGB[i] = newcolors[i]; //random(0, 1000);
        // Serial.print("\n");
      }
      stepNo = 0;
    }
    // Serial.print(stepNo);
    // Serial.print("\t");
    // set lights
    for (int i = 0; i <= 2; i++) {
      long newval = atRGB[i] + ((toRGB[i] - atRGB[i]) * stepNo) / transSteps;
      setBrightness(i, newval);
      // Serial.print(newval);
      // Serial.print("\t");
    }
    // Serial.println();

  }
}

// -------------------- zpm's fader implementation --------------------- //

EthernetServer server(80);
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x4B, 0x82 };

void setup() {

  Serial.begin(9600);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  // startup setup
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  setBrightness(RED, 0);
  setBrightness(BLUE, 0);
  setBrightness(GREEN, 0);
}

char ir[15];
char ig[15];
char ib[15];

#define bufferMax 128
int bufferSize;
char buffer[bufferMax];

void loop() {
  currentMicros = micros();
  // transitionRGB();

  EthernetClient client = server.available();
  if (client) {
    waitForRequest(client);
    parseRequest();
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connnection: close");
    client.println();
    
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    
    newcolors[RED] = atoi(ir);
    newcolors[GREEN] = atoi(ig);    
    newcolors[BLUE] = atoi(ib);
    
    setBrightness(0, atoi(ir));
    setBrightness(1, atoi(ig));
    setBrightness(2, atoi(ib));
    
  }

  writeRGB();

}

void waitForRequest(EthernetClient client) {
  bufferSize = 0;
  
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n')
        break;
      else
        if (bufferSize < bufferMax)
          buffer[bufferSize++] = c;
        else
          break;
    }
  }
  
  Serial.print("BufferSize - ");
  Serial.println(bufferSize);
  Serial.println(buffer);
}

void parseRequest() {
  //Received buffer contains "GET /cmd/param1/param2 HTTP/1.1".  Break it up.
  char* slash1;
  char* slash2;
  char* slash3;
  char* space2;
  
  slash1 = strstr(buffer, "/") + 1; // Look for first slash
  slash2 = strstr(slash1, "/") + 1; // second slash
  slash3 = strstr(slash2, "/") + 1; // third slash
  space2 = strstr(slash2, " ") + 1; // space after second slash (in case there is no third slash)
  if (slash3 > space2) slash3=slash2;
  
  // strncpy does not automatically add terminating zero, but strncat does! So start with blank string and concatenate.
  ir[0] = 0;
  ig[0] = 0;
  ib[0] = 0;
  strncat(ir, slash1, slash2-slash1-1);
  strncat(ig, slash2, slash3-slash2-1);
  strncat(ib, slash3, space2-slash3-1);
  
  Serial.print("Red value - ");
  Serial.println(ir);
  Serial.print("Green value - ");
  Serial.println(ig);
  Serial.print("Blue value - ");
  Serial.println(ib);
}
