#include <WiFi.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
const char* ssid       = "**";
const char* password   = "**";

const int IndicatorPin = 2;

const int AttemptThreshold = 50;

void setup() {
  Serial.begin(115200);
  pinModeSetups();
  connectToWifi();
}

void loop() {
  delay(5000);
  startIndicator();
  ensureWifiConnection();
  processPIRSensor();
  processThermalSensor();
  stopIndicator();
}

void pinModeSetups(){
  pinMode(IndicatorPin, OUTPUT);
}
/*Thermal Utils*/
void processThermalSensor(){
  delay(50);
}

/*PIR utils*/
void processPIRSensor(){
  delay(50);
}

/*Utill Fiunctions for Wifi */
void ensureWifiConnection(){
  if (WiFi.status() != WL_CONNECTED) { //Check for the connection
    connectToWifi();
  }
  Serial.println("WIFI - OK ...");
}

void connectToWifi(){
  //connect to WiFi
  int attemptCount = 0;
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      Serial.print(".");
      toggleIndicator();
      attemptCount++;
      if( attemptCount > AttemptThreshold){
        ESP.restart();
      }
  }
  stopIndicator();
  Serial.println("WIFI CONNECTED");
}

void disconnectWifi(){
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("WIFI DISCONNECTED");
}


/*Indicator Light*/
void startIndicator(){
  digitalWrite(IndicatorPin, HIGH);
}

void stopIndicator(){
  digitalWrite(IndicatorPin, LOW);
}

void toggleIndicator(){
  if(digitalRead(IndicatorPin) == HIGH){
    stopIndicator();
  }
  else{
    startIndicator();
  }
}

/*Process utills*/

void hard_restart() {
  esp_task_wdt_init(1,true);
  esp_task_wdt_add(NULL);
  while(true);
}
