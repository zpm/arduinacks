#include <SPI.h>
#include <Ethernet.h>

#define pinRED 3
#define pinBLUE 5
#define pinGREEN 6
const int pinMAP[3] = {pinRED, pinBLUE, pinGREEN};

// variables set by ethernet used to control internal modes
int ic = 0;
long ir = 0;
long ig = 0;
long ib = 0;

#define mOFF 0
#define mRGB 1
#define mBASIS 2
#define mSCHIZM 3
#define mBLINDER 4
#define mPULSAR 5


// ==================== start ethernet sheild ==================== //

EthernetServer server(80);
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x4B, 0x82 };

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

  // set all the values to zero
  ic = 0;
  ir = 0;
  ig = 0;
  ib = 0;

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
        ib = atol(tempString);
        tempString[0] = 0;
        strncat(tempString, slashAt2, slashAt3-slashAt2-1);
      } else {
        tempString[0] = 0;
        strncat(tempString, slashAt2, spaceAt-slashAt2-1);
      }
      ig = atol(tempString);
      tempString[0] = 0;
      strncat(tempString, slashAt1, slashAt2-slashAt1-1);
    } else {
      tempString[0] = 0;
      strncat(tempString, slashAt1, spaceAt-slashAt1-1);
    }
    ir = atol(tempString);
    tempString[0] = 0;
    strncat(tempString, &ethernetBuffer[6], slashAt1-&ethernetBuffer[6]-1);
  } else {
    tempString[0] = 0;
    strncat(tempString, &ethernetBuffer[6], spaceAt-&ethernetBuffer[6]-1);
  }

  // command set for ic bit
  if (!strncmp(tempString, "off", 3) != 0) {
    ic = mOFF;
  } else if (strncmp(tempString, "rgb", 3) == 0) {
    ic = mRGB;  
  } else if (strncmp(tempString, "basis", 5) == 0) {
    ic = mBASIS;
  } else if (strncmp(tempString, "schizm", 6) == 0) {
    ic = mSCHIZM;
  } else if (strncmp(tempString, "blinder", 7) == 0) {
    ic = mBLINDER;
  } else if (strncmp(tempString, "pulsar", 6) == 0) {
    ic = mPULSAR;
  }

  Serial.print("SETTING\t");
  Serial.print(ic);
  Serial.print("\t");
  Serial.print(ir);
  Serial.print("\t");
  Serial.print(ig);
  Serial.print("\t");
  Serial.print(ib);
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

void ledSetNextTarget() {

  // (ic == mOFF)
  // (ic == mRGB)
  // (ic == mBLINDER)

  if (ic == mBASIS) {

    for(int i=0; i<=2; i++) {
      toRGB[i] = random(0,255);
    }

  } else if (ic == mSCHIZM) {

    for(int i=0; i<=2; i++) {
      toRGB[i] = random(0,255);
    }

  } else if (ic == mBLINDER) {

    for(int i=0; i<=2; i++) {
      toRGB[i] = random(0,255);
    }

  } else if (ic == mPULSAR) {

    for(int i=0; i<=2; i++) {
      toRGB[i] = random(0,255);
    }

  }
  
}

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
      //Serial.print(newval);
      //Serial.print("\t");
    }
    //Serial.println();

  }
}

void ledcontrollerLoop() {

  // this function assumes that ic/ir/ig/ib have been written successfully by the ethernet
  // sheild and parses functionality out of them accordingly
  
  if (ic == mOFF) {
 
    analogWrite(pinRED, 0);
    analogWrite(pinBLUE, 0);
    analogWrite(pinGREEN, 0);
    
  } else if (ic == mRGB) {

    analogWrite(pinRED, ir);
    analogWrite(pinBLUE, ib);
    analogWrite(pinGREEN, ig);

  } else if (ic == mBASIS) {
 
    // ir is number of steps
    // ig is number of microseconds per step
    ledFader(ir, ig);

  } else if (ic == mSCHIZM) {

    analogWrite(pinRED, 0);
    analogWrite(pinBLUE, 0);
    analogWrite(pinGREEN, 0);

  } else if (ic == mBLINDER) {

    analogWrite(pinRED, ir);
    analogWrite(pinBLUE, ir);
    analogWrite(pinGREEN, ir);

  } else if (ic == mPULSAR) {

    analogWrite(pinRED, 0);
    analogWrite(pinBLUE, 0);
    analogWrite(pinGREEN, 0);

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


