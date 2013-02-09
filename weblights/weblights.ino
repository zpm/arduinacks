#include <SPI.h>
#include <Ethernet.h>

// these must be PWM pins
#define pinRED 3
#define pinBLUE 5
#define pinGREEN 6
// create map so it's easy to iterate over these vals with a loop
const int pinMAP[3] = {pinRED, pinBLUE, pinGREEN};

// variables set by ethernet used to control internal modes
int mode = 0;
long red = 0;
long green = 0;
long blue = 0;
long transSteps = 0;
long transTimePerStepInMicros = 0;

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
  } else if (strncmp(tempString, "fade", 5) == 0) {
    mode = mFADE;
    transSteps = temp1;
    transTimePerStepInMicros = temp2;
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
long toRGB[] = {random(0, 255), random(0, 255), random(0, 255)};
long stepNo = 0;
long transNext = 0;

void ledFader(long transSteps, long transTimePerStepInMicros) {
  
  long currentMicros = micros();

  if (currentMicros > transNext) {
    // set next transition event
    transNext = currentMicros + transTimePerStepInMicros;
    // major transition event
    stepNo += 1;
    if (stepNo >= transSteps) {
      // go to next step
      for (int i = 0; i <= 2; i++) {
        atRGB[i] = toRGB[i];
      }
    
      for(int i=0; i<=2; i++) {
        toRGB[i] = random(0,255);
      }
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
      //Serial.print(newval);
      //Serial.print("\t");
    }
    //Serial.println();

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
    ledFader(transSteps, transTimePerStepInMicros);

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


