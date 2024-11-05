#include <Arduino.h>

#include "switchKaKu.h"
#define TRANSMITTERID1 046241
#define rfPin 25
void setup() {
  
}

void loop() {
  switchKaku(rfPin, TRANSMITTERID1, 1, 1, true, 3);
  delay(3000);
  switchKaku(rfPin, TRANSMITTERID1, 1, 1, false, 3);
  delay(3000);
}