#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "YOUR_SSID";
const char* password = "PASSWORD";
WiFiUDP udp;
unsigned int localPort = 4210;  

const char* secret = "secret42";

#define MATCH_DELAY 500

#define DEBUG true

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP8266 Timing Side-Channel Victim ===");
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  udp.begin(localPort);
  Serial.print("Listening on UDP port ");
  Serial.println(localPort);
  
  Serial.print("Secret password is: ");
  Serial.println(secret);
}

void loop() {
  int packetSize = udp.parsePacket();
  
  if (packetSize) {
    char packetBuffer[64];
    int len = udp.read(packetBuffer, 63);
    if (len > 0) {
      packetBuffer[len] = 0; 
    }
    
    IPAddress remoteIP = udp.remoteIP();
    unsigned int remotePort = udp.remotePort();
    
    if (DEBUG) {
      Serial.print("Received '");
      Serial.print(packetBuffer);
      Serial.print("' from ");
      Serial.print(remoteIP);
      Serial.print(":");
      Serial.println(remotePort);
    }
    
    int matchCount = 0;
    
    int i = 0;
    for (i = 0; i < strlen(packetBuffer) && i < strlen(secret); i++) {
      if (packetBuffer[i] == secret[i]) {
        delayMicroseconds(MATCH_DELAY);
        matchCount++;
      } else {
        break;
      }
    }

    bool completeMatch = (matchCount == strlen(secret)) && (strlen(packetBuffer) == strlen(secret));
    
    if (DEBUG) {
      Serial.print("Matched ");
      Serial.print(matchCount);
      Serial.print("/");
      Serial.print(strlen(secret));
      Serial.print(" characters. Response: ");
      Serial.println(completeMatch ? "Correct" : "Wrong");
    }
    
    udp.beginPacket(remoteIP, remotePort);
    if (completeMatch) {
      udp.write("Correct");
    } else {
      udp.write("Wrong");
    }
    udp.endPacket();
  }
  
  delay(1);
}
