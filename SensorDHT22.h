#ifndef SENSOR_DHT22_H
#define SENSOR_DHT22_H

#include "DHT.h"

//#define DHTPIN 19  
#define DHTTYPE DHT22

class SensorDHT22 {
private:
  DHT dht;

public:
  SensorDHT22(int pin);
  void begin();
  float readHumidity();
  float readTemperature();
};

#endif