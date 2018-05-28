#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
const char* ssid       = "**";
const char* password   = "**";

const int IndicatorPin = 2;

const int AttemptThreshold = 50;

const char* JsonContentType="application/json";

const char* RooomUrl = ""; // URL for Room
const char* RooomStatusUrl = ""; // URL for Room Status


const int sensorId=1;

void setup() {
  Serial.begin(115200);
  pinModeSetups();
  connectToWifi();
  roomSetUp();
}

void loop() {
  delay(5000);
  startIndicator();
  ensureWifiConnection();
  processPIRSensor();
  processThermalSensor();
  publishData();
  stopIndicator();
}

void pinModeSetups(){
  pinMode(IndicatorPin, OUTPUT);
}

void publishData(){
  preparePostRequest();
}

void roomSetUp(){
  String body = "{\"id\":1,\"name\":\"1\"}";
  sendHttpPost(JsonContentType, RooomUrl, body);
}

/*Http request utils*/
void sendHttpPost(const char* contentType, const char* url,  String body){
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", contentType);
  int httpResponseCode = http.POST(body);
  httpResponseHandler(httpResponseCode, "Room setup completed", "Room setup failed");
  if( httpResponseCode > 0){
    String response = http.getString();
    Serial.println(response);
  }
  http.end();
}

void preparePostRequest(){
  HTTPClient http;
  http.begin(RooomUrl);
  ////http.addHeader("Content-Type", "application/json");
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
