#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
const char* ssid       = "**";
const char* password   = "**";
#include "time.h"

const int IndicatorPin = 2;

const int AttemptThreshold = 50;

const char* JsonContentType="application/json";

const char* RooomUrl = ""; // URL for Room
const char* RooomStatusUrl = ""; // URL for Room Status


const int sensorId=1;

//PIR Values
#define PIRInputPin 4
#define PIRRetryCount 100

//Time Setup
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

void setup() {
  Serial.begin(115200);
  pinModeSetups();
  connectToWifi();
  roomSetUp();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  delay(5000);
  startIndicator();
  ensureWifiConnection();
  boolean pirResult = processPIRSensor();
  //processThermalSensor();
  publishData(pirResult);
  stopIndicator();
}

void pinModeSetups(){
  pinMode(IndicatorPin, OUTPUT);
  pinMode(PIRInputPin, INPUT);
}

void publishData(boolean pirResult){
  String body ;
  String hours = getHours();
  String date = getDate();
  if(pirResult == true)
    body = "{\"id\":1,\"status\":\"true\", \"businessDate\":\""+date+"\",\"hours\":\""+hours+"\"},";
  else
    body = "{\"id\":1,\"status\":\"false\", \"businessDate\":\""+date+"\",\"hours\":\""+hours+"\"},";
  sendHttpPost(JsonContentType, RooomStatusUrl, body);
}

void roomSetUp(){
  String body = "{\"id\":1,\"name\":\"1\"}";
  sendHttpPost(JsonContentType, RooomUrl, body);
}

/**
 * TimeUtils
 */

tm getTimeInfo()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return timeinfo;
  }
  return timeinfo;
 //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  //getHours(&timeinfo);//Serial.println(&timeinfo, "%H%M");
  //getDate(&timeinfo);//Serial.println(&timeinfo, "%d-%m-%y");
}

String getHours(){
  struct tm timeinfo = getTimeInfo();
  Serial.println(&timeinfo, "%H%M");
  char buf[100];
  strptime(buf, "%H%M",&timeinfo);
  Serial.println(String(buf));
  return String(buf);
}

String getDate(){
  struct tm timeinfo = getTimeInfo();
  char buf[100];
  Serial.println(&timeinfo, "%d-%m-%y");
  strptime(buf, "%d-%m-%y",&timeinfo);
  Serial.println(String(buf));
  return String(buf);
}

/*Http request utils*/
void sendHttpPost(const char* contentType, const char* url,  String body){
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", contentType);
  int httpResponseCode = http.POST(body);
  Serial.print("POST HTTP Request: ");Serial.print(url); Serial.print(" "); Serial.println(body);
  httpResponseHandler(httpResponseCode, "Room setup completed", "Room setup failed");
  if( httpResponseCode > 0){
    String response = http.getString();
    Serial.println(response);
  }
  http.end();
}

void getRoomDetails(){
  HTTPClient http;
  http.begin(RooomUrl);
  http.addHeader("Content-Type", "text/plain");  
  int httpResponseCode = http.GET();
  httpResponseHandler(httpResponseCode, "Got Rooms", "Rooms Get error");
  if( httpResponseCode > 0){
    String response = http.getString();
    Serial.println(response);
  }
  http.end();
}

void httpResponseHandler(int httpResponseCode, String successMessage, String failureMessage){
  if(httpResponseCode>0){
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(successMessage);
  }else{
    Serial.print("Error on sending GET: ");
    Serial.println(httpResponseCode);
    Serial.println(failureMessage);
  }
}

/*Thermal Utils*/
void processThermalSensor(){
  delay(50);
}

/*PIR utils*/
/**
 * In 10 sec interval if PIR detects one motion the result will be true
 */
boolean processPIRSensor(){
  boolean motionDetected = false;
  for(int i = 0; i < PIRRetryCount; i++){
    if(digitalRead(PIRInputPin) == HIGH) {
      Serial.println("Motion detected. - ");
      motionDetected = true;
    }else{
      Serial.println("Motion Not detected. - ");
    }
    delay(100);
  }
  return motionDetected;
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
  Serial.println(WiFi.localIP());
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
