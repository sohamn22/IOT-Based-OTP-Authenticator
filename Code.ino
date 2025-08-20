  #include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "TimeLib.h"
#include "NTPClient.h"
#include "WiFiUdp.h"
#include "TOTP.h"  

//OLED 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

//Web Server
WebServer server(80);

//NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // IST offset 19800 sec

// TOTP Secret
uint8_t hmacKey[] = { '1','2','3','4','5','6','7','8','9','0' }; // Example secret
TOTP totp = TOTP(hmacKey, 10);

String currentOTP;

void handleRoot() {
  String html = "<html><body><h2>ESP32 OTP Verification</h2>";
  html += "<p>Enter OTP:</p>";
  html += "<form action='/verify' method='POST'>";
  html += "<input type='text' name='otp'><input type='submit' value='Check'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleVerify() {
  if (server.hasArg("otp")) {
    String entered = server.arg("otp");
    if (entered == currentOTP) {
      server.send(200, "text/html", "<h3>✅ Correct OTP!</h3>");
    } else {
      server.send(200, "text/html", "<h3>❌ Wrong OTP!</h3>");
    }
  } else {
    server.send(200, "text/html", "<h3>No OTP entered</h3>");
  }
}

void setup() {
  Serial.begin(115200);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
  display.display();

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected. IP: " + WiFi.localIP().toString());

  // NTP
  timeClient.begin();
  timeClient.update();

  // Web server 
  server.on("/", handleRoot);
  server.on("/verify", HTTP_POST, handleVerify);
  server.begin();
}

void loop() {
  server.handleClient();
  timeClient.update();

  unsigned long epochTime = timeClient.getEpochTime();
  currentOTP = String(totp.getCode(epochTime));

  // Display OTP on OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("OTP:");
  display.setCursor(0, 40);
  display.print(currentOTP);
  display.display();

  delay(1000);
}
