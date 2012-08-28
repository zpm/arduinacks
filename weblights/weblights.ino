#include <SPI.h>
#include <Ethernet.h>

// variables set by ethernet used to control internal modes
int ic = 0;
int ir = 0;
int ig = 0;
int ib = 0;

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
  char tempString[8];

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
          strncpy(tempString, slashAt3, slashAt4-slashAt3-1);
        } else {
          tempString[0] = 0;
          strncpy(tempString, slashAt3, spaceAt-slashAt3-1);
        }
        ib = atoi(tempString);
        tempString[0] = 0;
        strncpy(tempString, slashAt2, slashAt3-slashAt2-1);
      } else {
        tempString[0] = 0;
        strncpy(tempString, slashAt2, spaceAt-slashAt2-1);
      }
      ig = atoi(tempString);
      tempString[0] = 0;
      strncpy(tempString, slashAt1, slashAt2-slashAt1-1);
    } else {
      tempString[0] = 0;
      strncpy(tempString, slashAt1, spaceAt-slashAt1-1);
    }
    ir = atoi(tempString);
    tempString[0] = 0;
    strncpy(tempString, &ethernetBuffer[6], slashAt1-&ethernetBuffer[6]-1);
  } else {
    tempString[0] = 0;
    strncpy(tempString, &ethernetBuffer[6], spaceAt-&ethernetBuffer[6]-1);
  }

  // command set for ic bit
  if (!strncmp(tempString, "off", 6) != 0) {
    ic = 0;
  } else if (strncmp(tempString, "basis", 6) == 0) {
    ic = 1;
  } else if (strncmp(tempString, "schizm", 6) == 0) {
    ic = 2;
  } else if (strncmp(tempString, "blinder", 6) == 0) {
    ic = 3;
  } else if (strncmp(tempString, "pulsar", 6) == 0) {
    ic = 4;
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

void ledcontrollerLoop() {

  // this function assumes that ic/ir/ig/ib have been written successfully by the ethernet
  // sheild and parses functionality out of them accordingly
  
  analogWrite(3, ir);
  analogWrite(5, ig);
  analogWrite(6, ib);

  Serial.print(ic);
  Serial.print("\t");
  Serial.print(ir);
  Serial.print("\t");
  Serial.print(ig);
  Serial.print("\t");
  Serial.print(ib);
  Serial.print("\n");
  
}

// ==================== end led controller ==================== //



void setup() {

  Serial.begin(9600);

  ethernetSetup();

  // setup output pins on pwm
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  // used to reset to 0 here, but prefer to not to keep last setting

}

void loop() {

  ethernetLoop();
  ledcontrollerLoop();

}


