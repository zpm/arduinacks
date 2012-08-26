#include <SPI.h>
#include <Ethernet.h>

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
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
//  pinMode(9, OUTPUT);
//  pinMode(10, OUTPUT);
//  pinMode(11, OUTPUT);

  analogWrite(3, 0);
  analogWrite(5, 0);
  analogWrite(6, 0);
//  analogWrite(9, 0);
//  analogWrite(10, 0);
//  analogWrite(11, 0);

}

char ir[15];
char ig[15];
char ib[15];
//char ir2[15];
//char ig2[15];
//char ib2[15];

#define bufferMax 128
int bufferSize;
char buffer[bufferMax];

void loop() {
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
    
    analogWrite(3, atoi(ir));
    analogWrite(5, atoi(ig));
    analogWrite(6, atoi(ib));
//    analogWrite(9, atoi(ir2));
//    analogWrite(10, atoi(ig2));
//    analogWrite(11, atoi(ib2));

    Serial.print(atoi(ir));
    Serial.print("\t");
    Serial.print(atoi(ig));
    Serial.print("\t");
    Serial.print(atoi(ib));
//    Serial.print("\t");
//    Serial.print(atoi(ir2));
//    Serial.print("\t");
//    Serial.print(atoi(ig2));
//    Serial.print("\t");
//    Serial.print(atoi(ib2));  
//    Serial.print("\n");
  }
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
}

void parseRequest() {
  //Received buffer contains "GET /cmd/param1/param2 HTTP/1.1".  Break it up.
  char* slash1;
  char* slash2;
  char* slash3;
//  char* slash4;
//  char* slash5;
//  char* slash6;
  char* space2;
  
  slash1 = strstr(buffer, "/") + 1; // Look for first slash
  slash2 = strstr(slash1, "/") + 1; // second slash
  slash3 = strstr(slash2, "/") + 1; // third slash
  space2 = strstr(slash3, "/") + 1; // Look for first slash
//  slash4 = strstr(slash3, "/") + 1; // Look for first slash
//  slash5 = strstr(slash4, "/") + 1; // second slash
//  slash6 = strstr(slash5, "/") + 1; // third slash
//  space2 = strstr(slash6, " ") + 1; // space after second slash (in case there is no third slash)
//  if (slash6 > space2) slash6=slash2;
  if (slash3 > space2) slash3=slash2;
  
  // strncpy does not automatically add terminating zero, but strncat does! So start with blank string and concatenate.
  ir[0] = 0;
  ig[0] = 0;
  ib[0] = 0;
//  ir2[0] = 0;
//  ig2[0] = 0;
//  ib2[0] = 0;
  strncat(ir, slash1, slash2-slash1-1);
  strncat(ig, slash2, slash3-slash2-1);
  strncat(ib, slash3, space2-slash3-1);
//  strncat(ib, slash3, slash4-slash3-1);
//  strncat(ir2, slash4, slash5-slash4-1);
//  strncat(ig2, slash5, slash6-slash5-1);
//  strncat(ib2, slash6, space2-slash6-1);

}
