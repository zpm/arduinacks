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
    
    Serial.print("SETTING\t");
    Serial.print(mode);
    Serial.print("\t");
    Serial.print(red);
    Serial.print("\t");
    Serial.print(green);
    Serial.print("\t");
    Serial.print(blue);
    Serial.print("\n");
    
  } else if (strncmp(tempString, "fade", 4) == 0) {
    mode = mFADE;
    steps = temp1;
    timePerStep = temp2;
    
    Serial.print("SETTING\t");
    Serial.print(mode);
    Serial.print("\t");
    Serial.print(steps);
    Serial.print("\t");
    Serial.print(timePerStep);
    Serial.print("\n");
  }



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


long currentRGB[] = {255,0,0};
long initializeFade = 0;
long nextFadeEvent = 0;
long fadeState = 1;  //determines which stage of the fade we're currently in
//start with color set to 255,0,0
//1) increase green: 255,255,0
//2) decrease red: 0,255,0
//3) increase blue: 0,255,255
//4) decrease green: 0,0,255
//5) increase red: 255,0,255
//6) decrease blue: 255,0,0
//go to #1

void ledFader(long timePerStep){

  long currentTime = micros();
  
  if(initializeFade == 0){
  	nextFadeEvent = currentTime + timePerStep;
  	initializeFade = 1;
  }
  
  if(currentTime > nextFadeEvent){
  
    nextFadeEvent = currentTime + timePerStep;
    
    switch(fadeState){
      case 1:
        currentRGB[1]++;
        if(currentRGB[1] == 255){
          fadeState = 2;
        }
      break;
      case 2:
        currentRGB[0]--;
        if(currentRGB[0] == 0){
    	fadeState = 3;
        }
      break;
      case 3:
        currentRGB[2]++;
        if(currentRGB[2] == 255){
          fadeState = 4;
        }
      break;
      case 4:
        currentRGB[1]--;
        if(currentRGB[1] == 0){
          fadeState = 5;
        }
      break;
      case 5:
        currentRGB[0]++;
        if(currentRGB[0] == 255){
          fadeState = 6;
        }
      break;
      case 6:
        currentRGB[2]--;
        if(currentRGB[2] == 0){
          fadeState = 1;
        }
      break;
    }
    analogWrite(pinRED, currentRGB[0]);
    analogWrite(pinGREEN, currentRGB[1]);
    analogWrite(pinBLUE, currentRGB[2]);
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

  } else if (mode == mFADE) {
    ledFader(timePerStep);
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


