#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "wifi ssid/name";
const char* password = "wifi password";
const char* Gemini_Token = "Gemini api key";
const char* Gemini_Max_Tokens = "500";
String res = "";

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  while (!Serial)
    ;

  // Wait for WiFi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  Serial.println("");
  Serial.println("Ask your Question : ");
  while (!Serial.available())
    ;
  while (Serial.available()) {
    char add = Serial.read();
    res = res + add;
    delay(1);
  }
  int len = res.length();
  res = res.substring(0, (len - 1));
  res = "\"" + res + "\"";
  Serial.println("");
  Serial.print("Asking Your Question : ");
  Serial.println(res);

  WiFiClientSecure client;
  client.setInsecure();  
  HTTPClient http;

  String url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + String(Gemini_Token);

  if (http.begin(client, url)) {  // HTTPS connection
    http.addHeader("Content-Type", "application/json");


    String payload = String("{\"contents\": [{\"parts\":[{\"text\":" + res + "}]}],\"generationConfig\": {\"maxOutputTokens\": " + String(Gemini_Max_Tokens) + "}}");

    int httpCode = http.POST(payload);

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String responsePayload = http.getString();

      DynamicJsonDocument doc(2048);
      deserializeJson(doc, responsePayload);
      String Answer = doc["candidates"][0]["content"]["parts"][0]["text"];

      // Filter out special characters
      Answer.trim();
      String filteredAnswer = "";
      for (size_t i = 0; i < Answer.length(); i++) {
        char c = Answer[i];
        if (isalnum(c) || isspace(c)) {
          filteredAnswer += c;
        } else {
          filteredAnswer += ' ';
        }
      }
      Answer = filteredAnswer;

      Serial.println("");
      Serial.println("Here is your Answer: ");
      Serial.println("");
      Serial.println(Answer);
    } else {
      Serial.printf("[HTTPS] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  res = "";
}
