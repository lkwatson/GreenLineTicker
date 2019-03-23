#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Fill in network info below, and put in the desired stop ID on line 93.
const char* ssid     = "REPLACE";
const char* password = "ME";

const char* SHA1_NIST = "E7 75 AB E4 D6 9B 38 44 83 21 61 76 84 0E 27 83 0A AF 20 62";
const char* SHA1_MBTA = "01 F4 85 E0 3B 54 17 27 F5 FF 94 D9 64 D8 10 FA C8 24 10 9D";

Adafruit_IS31FL3731_Wing matrix = Adafruit_IS31FL3731_Wing();

void setup() {
  
  Serial.begin(115200);
  delay(100);

  Serial.begin(9600);
  Serial.println("ISSI manual animation test");
  if (! matrix.begin()) {
    Serial.println("IS31 not found");
    while (1);
  }
  Serial.println("IS31 Found!");

  // We start by connecting to a WiFi network
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

unsigned long getCurrentTime() {
  HTTPClient http;
  http.begin("https://nist.time.gov/actualtime.cgi", SHA1_NIST);
  int httpCode = http.GET();

  if (httpCode > 0) {
    // A hacky way to parse the XML and convert ms to s at the same time!
    unsigned long currentTime = atol(http.getString().substring(17, 27).c_str());

    // Subtract timezone offset
    // This doesn't account for DST, yet :)
    return (currentTime - 14400);
  }else{
    Serial.println("HTTP call to NIST failed");
    return 0;
  }
  
  http.end();
}

unsigned long parseDateString(char* timeStr) {
  struct tm tm;
  time_t epochTime;
  if (strptime(timeStr, "%Y-%m-%dT%H:%M:%S-04:00", &tm) != NULL ) {
    epochTime = mktime(&tm);
    return (unsigned long)epochTime;
  } else {
    Serial.println("Couldn't parse time string");
    return 0;
  }
}

void loop() {

  unsigned long currentTime = getCurrentTime();
  long firstTrain = -1;
  long secondTrain = -1; 

  DynamicJsonDocument jsonDoc;
  HTTPClient http;
  // REPLACE XXXXX with the stop id!
  http.begin("https://api-v3.mbta.com/predictions?filter[stop]=XXXXX", SHA1_MBTA);
  int httpCode = http.GET();

  if (httpCode > 0) {
    DeserializationError error = deserializeJson(jsonDoc, http.getString());
    
    if (error) {
      Serial.println("Error with JSON deserializing");
    }else{
      // Parse JSON
      JsonObject jsonObj = jsonDoc.as<JsonObject>();
      JsonArray predictions = jsonObj["data"];

      if (predictions.size() > 0) {
        char timeString[26];
        String arrivalTime1 = predictions[0]["attributes"]["arrival_time"].as<char*>();
        arrivalTime1.toCharArray(timeString, 26);
        firstTrain = parseDateString(timeString) - currentTime;
        if (firstTrain < 0) {
          firstTrain = 0;
        }
      }

      if (predictions.size() > 1) {
        char timeString[26];
        String arrivalTime2 = predictions[1]["attributes"]["arrival_time"].as<char*>();
        arrivalTime2.toCharArray(timeString, 26);
        secondTrain = parseDateString(timeString) - currentTime;
        if (secondTrain < 0) {
          secondTrain = 0;
        }
      }
      
      Serial.println(firstTrain);
      Serial.println(secondTrain);
    }
  }else{
    Serial.println("HTTP call to MBTA API failed");
  }
  http.end(); //Close connection
  
  if (firstTrain != -1 && secondTrain != -1) {
    // Repeat for 18 seconds
    for (int j=0; j < 3; j++) {
      for (int i=0; i < 3; i++) {
        showTime(firstTrain);
        firstTrain--;
        secondTrain--;
        delay(1000);
      }
      for (int i=0; i < 3; i++) {
        showTime(secondTrain);
        firstTrain--;
        secondTrain--;
        delay(1000);
      }
    }
  }else if (firstTrain != -1) {
    // Repeat for 15 seconds
    for (int i=0; i < 15; i++) {
      showTime(firstTrain);
      firstTrain--;
      delay(1000);
    }
  }else{
    matrix.print("FU");
  }
  
  Serial.println();
  Serial.println("--- End Loop ---");
}

void showTime(int rawSecs) {
  int minutes = rawSecs / 60;
  int secs = rawSecs % 60;
  
  matrix.setTextSize(1);
  matrix.setTextColor(32);
  matrix.setTextWrap(false);
  matrix.clear();

  if (minutes > 9) {
    matrix.setCursor(0,0);
    matrix.print(minutes/10);
    matrix.setCursor(5,0);
    matrix.print(minutes%10);
    matrix.setCursor(10,0);
    matrix.print("m");
  }else{
    matrix.setCursor(0,0);
    matrix.print(minutes);
    matrix.setCursor(5,0);
    matrix.print(secs/10);
    matrix.setCursor(10,0);
    matrix.print(secs%10);
  }
}
