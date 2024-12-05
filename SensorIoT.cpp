#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>  
 
#define DHT11PIN 4
#define DHTTYPE DHT11
 
DHT dhtSensor(DHT11PIN, DHTTYPE);

#define WIFI_SSID "myRouter"
#define WIFI_PASSWORD "09112005" //senha do wifi do rubens
#define API_KEY "AIzaSyDhZ0jNSrvphkJlCwOQXorBcvK-ZzPVkrY"
#define FIREBASE_PROJECT_ID "finalproject-c4d5d"
#define USER_EMAIL "p4dojefinho@gmail.com"
#define USER_PASSWORD "P4dojefinho123"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
 
WiFiUDP ntpUDP;
NTPClient ntp(ntpUDP);
 
void setup() {
  Serial.begin(115200);
 
  dhtSensor.begin();
 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to wifi...");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.println("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
 
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  ntp.begin();
  ntp.setTimeOffset(-10800);
}
 
void loop() {
 
  String collectionPath = "/SensorData";
  String docId = String(millis());;
 
  FirebaseJson content;
  FirebaseJson fields;
 
  float temperature = dhtSensor.readTemperature();
  float humidity = dhtSensor.readHumidity();
 
  if (!isnan(temperature) && !isnan(humidity)) {
    FirebaseJson tempField;
    tempField.set("stringValue", String(temperature, 2));
    fields.set("temp", tempField);
 
    FirebaseJson hmdField;
    hmdField.set("stringValue", String(humidity, 2));
    fields.set("hmd", hmdField);
 
    String time = "";
    String date = "";
 
    if (ntp.update()){
      time = ntp.getFormattedTime();
      String formattedDateTime = ntp.getFormattedDate();
      date = formattedDateTime.substring(0, 10);
    } else {
      Serial.println("!Erro ao atualizar NTP!\n");
    }

    FirebaseJson dateField;
    dateField.set("stringValue", date);
    fields.set("date", dateField);
    FirebaseJson timeField;
    timeField.set("stringValue", time);
    fields.set("time", timeField);
    content.set("fields", fields);
 
    Serial.print("Update/Add DHT Data...");
 
    if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", collectionPath.c_str(), docId.c_str(), content.raw(), "")){
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    } else {
      Serial.println(fbdo.errorReason());
    }
  } else {
    Serial.println("Failed to read DHT data.");
  }
 
  delay(3600000);
}