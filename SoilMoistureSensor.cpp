#include "SoilMoistureSensor.h"
#include <Arduino.h>

SoilMoistureSensor::SoilMoistureSensor(int pin) : pin(pin) {
  pinMode(pin, INPUT);
}

float SoilMoistureSensor::leerValor() { 
  int valor = analogRead(pin);
  return map(valor, 0, 4095, 0, 100);
}