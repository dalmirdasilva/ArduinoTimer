#include <TimerOne.h>

void timerIsr() {
    digitalWrite(13, digitalRead(13) ^ 1);
} 

void setup() {
    pinMode(13, OUTPUT);
    Timer1.initialize(100000);
    Timer1.attachInterrupt(timerIsr);
}
 
void loop() {
}
