#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "SensorDHT22.h"
#include "SoilMoistureSensor.h"
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>

const String WIFI_SSID = "Wokwi-GUEST";
const String WIFI_PASSWORD = "";
const char* urlDHT22 = "https://notesweb.azurewebsites.net/sensor/api/v1/air"; 
const char* urlSoilMoistureBase = "https://notesweb.azurewebsites.net/sensor/api/v1/soil"; 
const char* urlAspersor = "https://notesweb.azurewebsites.net/sensor/api/v1/irrigate";

const int pinSensorHumedadSuelo1 = 32;
const int pinSensorHumedadSuelo2 = 33;
const int pinSensorHumedadSuelo3 = 35;
const int pinSensorHumedadSuelo4 = 34;
const int pinSensorHumedadSuelo5 = 39;
const int pinSensorHumedadSuelo6 = 36;

const int pinDHT = 19;

int pinAspersor = 4;

LiquidCrystal_I2C lcd(0x27, 16, 2);

SoilMoistureSensor sm1(pinSensorHumedadSuelo1);
SoilMoistureSensor sm2(pinSensorHumedadSuelo2);
SoilMoistureSensor sm3(pinSensorHumedadSuelo3);
SoilMoistureSensor sm4(pinSensorHumedadSuelo4);
SoilMoistureSensor sm5(pinSensorHumedadSuelo5);
SoilMoistureSensor sm6(pinSensorHumedadSuelo6);

SensorDHT22 sensorDHT22(pinDHT);

float previousHumidity = 0.0;
float previousTemperature = 0.0;
float previousSoilMoistureValues[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  sensorDHT22.begin(); 
  pinMode(pinAspersor, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi");
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.setCursor(0, 0);
  lcd.print("Connected!         ");
  lcd.setCursor(0, 1);
  lcd.print("        ");
  Serial.println(" Connected!");
  delay(3000);
  lcd.clear();
  lcd.noBacklight();
}

void loop() {
  delay(2000);  

  float humedad = sensorDHT22.readHumidity();
  float temperatura = sensorDHT22.readTemperature();

  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error al leer del sensor DHT22!");
    return;
  }

  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" *C");

  if (humedad != previousHumidity || temperatura != previousTemperature) {
    sendDatatToAPIDHT22(temperatura, humedad);
    previousHumidity = humedad;
    previousTemperature = temperatura;
  }

  float valueSHS1 = sm1.leerValor();
  float valueSHS2 = sm2.leerValor();
  float valueSHS3 = sm3.leerValor();
  float valueSHS4 = sm4.leerValor();
  float valueSHS5 = sm5.leerValor();
  float valueSHS6 = sm6.leerValor();

  Serial.print("Humedad Maceta 1: ");
  Serial.print(valueSHS1);
  Serial.println("%");

  Serial.print("Humedad Maceta 2: ");
  Serial.print(valueSHS2);
  Serial.println("%");

  Serial.print("Humedad Maceta 3: ");
  Serial.print(valueSHS3);
  Serial.println("%");

  Serial.print("Humedad Maceta 4: ");
  Serial.print(valueSHS4);
  Serial.println("%");

  Serial.print("Humedad Maceta 5: ");
  Serial.print(valueSHS5);
  Serial.println("%");

  Serial.print("Humedad Maceta 6: ");
  Serial.print(valueSHS6);
  Serial.println("%");

  float soilMoistureValues[] = {valueSHS1, valueSHS2, valueSHS3, valueSHS4, valueSHS5, valueSHS6};
  int changedIndexes[6];
  int changedCount = 0;

  for (int i = 0; i < 6; i++) {
    if (soilMoistureValues[i] != previousSoilMoistureValues[i]) {
      changedIndexes[changedCount++] = i;
      previousSoilMoistureValues[i] = soilMoistureValues[i];
    }
  }

  if (changedCount > 0) {
    sendDatatToAPISoilMoisture(soilMoistureValues, changedIndexes, changedCount);
  }

  checkAspersor();
}

void sendDatatToAPIDHT22(float t, float h) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(urlDHT22);
    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument jsonDoc(200); 

    jsonDoc["temperatureValue"] = round(t * 100) / 100.0;
    jsonDoc["humidityValue"] = round(h * 100) / 100.0;
    jsonDoc["sensorName"] = "DHT22";
    
    String jsonString;
    serializeJson(jsonDoc, jsonString);

    int httpResponseCode = http.sendRequest("PUT", jsonString);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending PUT: ");
      Serial.println(httpResponseCode);
    }

    Serial.println(jsonString);

    http.end();
  }
}

void sendDatatToAPISoilMoisture(float soilMoistureValues[], int changedIndexes[], int changedCount) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(urlSoilMoistureBase);
    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument jsonDoc(500); 
    JsonArray sensorsArray = jsonDoc.to<JsonArray>();

    for (int i = 0; i < changedCount; i++) {
      int index = changedIndexes[i];
      JsonObject sensorData = sensorsArray.createNestedObject();
      sensorData["sensorName"] = "soil_moisture" + String(index + 1);
      sensorData["sensorValue"] = round(soilMoistureValues[index] * 100) / 100.0;
    }
    
    String jsonString;
    serializeJson(jsonDoc, jsonString);

    int httpResponseCode = http.sendRequest("PUT", jsonString);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending PUT: ");
      Serial.println(httpResponseCode);
    }

    Serial.println(jsonString);

    http.end();
  }
}

void checkAspersor(){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(urlAspersor);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);

      DynamicJsonDocument jsonDoc(1024);
      deserializeJson(jsonDoc, response);

      bool activate = jsonDoc["active"];

      if(activate){
        digitalWrite(pinAspersor, HIGH);
        lcd.backlight();
        lcd.setCursor(0, 0);
        lcd.print("Regando...");
      }else{
        digitalWrite(pinAspersor, LOW);
        lcd.clear();
        lcd.noBacklight();
      }

    } else {
      Serial.print("Error on sending GET: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
}
