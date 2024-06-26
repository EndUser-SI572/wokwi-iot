#ifndef SOIL_MOISTURE_SENSOR_H
#define SOIL_MOISTURE_SENSOR_H

class SoilMoistureSensor {
private:
  int pin;

public:
  SoilMoistureSensor(int pin);
  float leerValor();
};

#endif