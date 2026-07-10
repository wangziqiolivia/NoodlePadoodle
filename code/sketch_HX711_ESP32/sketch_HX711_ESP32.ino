#include "HX711.h"

#define DOUT1 16
//#define DOUT2 17
#define CLK   4

HX711 scale1;
//HX711 scale2;

void setup() {
  Serial.begin(115200);

  scale1.begin(DOUT1, CLK);
  //scale2.begin(DOUT2, CLK);

  Serial.println("HX711 Ready");
}

void loop() {

  if (scale1.is_ready() ) {
    //&& scale2.is_ready()

    long value1 = scale1.read();
    //long value2 = scale2.read();

    Serial.println(value1);
    //Serial.print(",");
    //Serial.println(value2);
  }

}