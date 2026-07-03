#include "HX711.h"

#define DOUT 16
#define CLK 4

HX711 scale;

void setup() {
  Serial.begin(115200);

  scale.begin(DOUT, CLK);

  Serial.println("HX711 Ready");
}

void loop() {
  if (scale.is_ready()) {
    Serial.println(scale.read());
  }

  delay(2);
}