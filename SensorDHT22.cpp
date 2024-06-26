#include "SensorDHT22.h"

SensorDHT22::SensorDHT22(int pin) : dht(pin, DHTTYPE) {}

void SensorDHT22::begin() {
  dht.begin();
}

float SensorDHT22::readHumidity() {
  return dht.readHumidity();
}

float SensorDHT22::readTemperature() {
  return dht.readTemperature();
}