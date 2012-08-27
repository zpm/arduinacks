#include <SPI.h>
#include <Ethernet.h>


char im[15];
char ir[15];
char ig[15];
char ib[15];


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

  // Received ethernetBuffer contains "POST /cmd/param1/param2 HTTP/1.1".  Break it up.
  char* slash1;
  char* slash2;
  char* slash3;
  char* space2;
  
  slash1 = strstr(ethernetBuffer, "/") + 1; // Look for first slash
  slash2 = strstr(slash1, "/") + 1; // second slash
  slash3 = strstr(slash2, "/") + 1; // third slash
  space2 = strstr(slash3, "/") + 1; // Look for first slash
  if (slash3 > space2) slash3=slash2;
  
  // strncpy does not automatically add terminating zero,
  // but strncat does! So start with blank string and concatenate.
  ir[0] = 0;
  ig[0] = 0;
  ib[0] = 0;

  strncat(ir, slash1, slash2-slash1-1);
  strncat(ig, slash2, slash3-slash2-1);
  strncat(ib, slash3, space2-slash3-1);

  return true;
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

  // this function assumes that im/ir/ig/ib have been written successfully by the ethernet
  // sheild and parses functionality out of them accordingly
  
  analogWrite(3, atoi(ir));
  analogWrite(5, atoi(ig));
  analogWrite(6, atoi(ib));

  Serial.print(atoi(ir));
  Serial.print("\t");
  Serial.print(atoi(ig));
  Serial.print("\t");
  Serial.print(atoi(ib));
  
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


