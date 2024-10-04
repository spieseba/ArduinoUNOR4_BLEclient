#include <Arduino.h>
#include "ble.h"


void wrappedFunc(uint8_t data)
{
  Serial.print("Do something with value (");
  Serial.print(data);
  Serial.println(") in wrappedFunc!");
}


void setup() {
  Serial.begin(115200); 
  startBLEStack(); 
}

void loop() {
  wrapperBLE(wrappedFunc);
  Serial.println("No connection! Try again in 3 seconds ...");
  delay(3000);
}
