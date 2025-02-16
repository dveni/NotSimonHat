//**************************************************************//
//  Name    : shiftOutCode, Hello World
//  Author  : Carlyn Maw,Tom Igoe, David A. Mellis
//  Date    : 25 Oct, 2006
//  Modified: 23 Mar 2010
//  Version : 2.0
//  Notes   : Code for using a 74HC595 Shift Register           //
//          : to count from 0 to 255
//****************************************************************
//Pin connected to ST_CP of 74HC595
int latchPin = 5;
//Pin connected to SH_CP of 74HC595
int clockPin = 2;
////Pin connected to DS of 74HC595
int dataPin = 3;

void setup() {
pinMode(latchPin, OUTPUT);
pinMode(clockPin, OUTPUT);
pinMode(dataPin, OUTPUT);
}
// void loop() {
// int first = 1;
// if (first == 1){
//   shiftOutPattern(0b00000000);  // Shift out a single bit for each LED  
//   first = 0;
// } 

// shiftOutPattern(0b00001100);  // Shift out a single bit for each LED
// while(true);
// }
void loop() {
for (int i = 0; i < 255; i++) {
shiftOutPattern(i);  // Shift out a single bit for each LED
delay(1000);
    }
}
// void loop() {
// for (int i = 0; i < 8; i++) {
// shiftOutPattern(1 << i);  // Shift out a single bit for each LED
// delay(200);
//     }
// }
 
// Function to send data to shift register
void shiftOutPattern(byte pattern) {
digitalWrite(latchPin, LOW);
shiftOut(dataPin, clockPin, MSBFIRST, 0b00000000);
digitalWrite(latchPin, HIGH);
delay(200);
digitalWrite(latchPin, LOW);
shiftOut(dataPin, clockPin, MSBFIRST, pattern);
digitalWrite(latchPin, HIGH);
}


