#include <Arduino.h>

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <LiquidCrystal_I2C.h>

#define CLK_PIN 5
#define DAT_PIN 4
#define RST_PIN 2


const char* ssid = "maxhotspot";
const char* password = "680856667m";

//Function declarations
void printDateTime(const RtcDateTime &dt);
void handleRoot(AsyncWebServerRequest *request);

LiquidCrystal_I2C lcd(0x27, 16, 2);


// Initializing communication with the RTC module
ThreeWire myWire(CLK_PIN, DAT_PIN, RST_PIN);
RtcDS1302<ThreeWire> rtc(myWire);


// --Async WebServer server(80);
AsyncWebServer server(80);


void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // get the local IP address of the ESP32
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  // Initialize the RTC
  Serial.print("Compiled time:");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);

  rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!rtc.IsDateTimeValid()) {
    Serial.println("RTC lost confidence in the DateTime!");
    rtc.SetDateTime(compiled);
  }
  if (rtc.GetIsWriteProtected()) {
    Serial.println("RTC was write protected, enabling writing now");
    rtc.SetIsWriteProtected(false);
  }
  if (!rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    rtc.SetIsRunning(true);
  }


  //Get the current date and time from the RTC
  RtcDateTime now = rtc.GetDateTime();

  if(now < compiled) {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    Serial.println("RTC is newer than compile time. (this is expected)");
  } else if (now == compiled) {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }


    server.on("/", HTTP_GET, handleRoot);
    server.begin();

}

void loop() {
  RtcDateTime now = rtc.GetDateTime();
 

  //Check if the date and time from rtc is not valid
  if(!now.IsValid()){
    Serial.println("Lost Confidence");
    //Check for the battery... check if it has enough power
    //remove the battery and reconnect it
  }

  printDateTime(now);
  Serial.println();

  delay(1000);
}

//Macro to calculate the size of an array
//countof(a)(sizeof(a)/sizeof(a[0]));

void printDateTime(const RtcDateTime &dt){
  char dateString[21];

  //format of the date ande time
  snprintf(dateString, sizeof(dateString), 
  "%02u/%02u/%04u %02u:%02u:%02u",
  dt.Day(),
  dt.Month(),
  dt.Year(),
  dt.Hour(),
  dt.Minute(),
  dt.Second() );

  Serial.println(dateString);


  //lcd
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(dateString);
}

void handleRoot(AsyncWebServerRequest *request){

  char dateString[21];

  RtcDateTime now = rtc.GetDateTime();

  //Format
  snprintf(dateString, 
    countof(dateString), 
    "%02u/%02u/%04u %02u:%02u:%02u",
    now.Day(),
    now.Month(),
    now.Year(),
    now.Hour(),
    now.Minute(),
    now.Second()

  );

  String response = "<html><body>";
  response += "<h1>Current time : </h1>";
  response += "<p>" + String(dateString) + "</p>";
  response += "</body></html>";

  request->send(200, "text/html", response);
}
