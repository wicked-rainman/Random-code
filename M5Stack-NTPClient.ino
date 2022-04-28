//Simple NTP client to use as a bedside clock
//Green display during the day
//Dull grey at night

#include <M5Stack.h>
#include "Free_Fonts.h"

#include <WiFi.h>
#include "time.h"
#include "Free_Fonts.h"
#define TRUE 1
#define FALSE 0
#define BRIGHTNESS 10
int status = WL_IDLE_STATUS;
const char* ssid = "Network";
const char* pass = "password";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;

//--------------------------------------------------------
void setup() {
  M5.begin(true, true, true);
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  M5.Lcd.setBrightness(BRIGHTNESS);
  M5.Lcd.setTextSize(4);
  while (WiFi.status() != WL_CONNECTED) {
    M5.Lcd.print(".");
    delay(1000);
  }
  M5.Lcd.println("\n");
  M5.Lcd.println(WiFi.localIP());
  delay(5000);
  M5.Lcd.clear();
  M5.Lcd.setFreeFont(FS12);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

}
//--------------------
void loop() {
  //--------------------
  int k = 70;
  int hrsInt, minsInt,storedMinsInt;
  char hrs[3];
  char mins[3];
  storedMinsInt=0;
  while (1) {
    getLocalTime(&timeinfo);
    strftime(hrs,3, "%H", &timeinfo);
    hrsInt = atoi(hrs);
    strftime(mins,3,"%M", &timeinfo);
    minsInt = atoi(mins);
    if ((hrsInt < 5) || (hrsInt < 21))  {
    M5.Lcd.setTextColor(GREEN);
      M5.Lcd.setBrightness(50);
    }
    else {
      M5.Lcd.setTextColor(0x7bef);
      M5.Lcd.setBrightness(10);
    }
    if(storedMinsInt != minsInt) {
      storedMinsInt = minsInt;
    M5.Lcd.clear();
    if (k == 220) k = 70;
    else k += 10;
    M5.Lcd.setCursor(40, k);
    M5.Lcd.printf("%s:%s", hrs,mins);
    }
    delay(30000);
  }
}
