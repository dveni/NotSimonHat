#include <Keypad.h>
 
// ✅ 4x4 Keypad Setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
 
byte colPins[ROWS] = {9, 8, 7, 6};   // Connected to row pins of keypad
byte rowPins[COLS] = {13, 12, 11, 10}; // Connected to column pins of keypad
 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
 
// ✅ Shift Register (74HC595 for LEDs) (No SPI)
#define LATCH_PIN 5
#define DATA_PIN 3
#define CLOCK_PIN 2
 
const char correctPassword[] = "222222222"; //"28022025#";
char enteredPassword[10] = ""; // Store input
byte index = 0;
bool accessGranted = false; // Flag to start LEDs
 
void setup() {
    Serial.begin(9600);
 
    // Shift Register Setup
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);
 
    Serial.println("Enter Password:");
}
 
void loop() {
    char key = keypad.getKey();
 
    if (key) {
        Serial.print("Key Pressed: "); Serial.println(key); // Debugging
 
        if (key == 'D') { 
            clearInput();
        } else {
            enteredPassword[index] = key;
            index++;
 
            // Print input to Serial Monitor
            Serial.print("Current Input: ");
            Serial.println(enteredPassword);
 
            // Check if password is complete
            if (index == 9) {
                enteredPassword[9] = '\0'; // Null-terminate string
 
                if (strcmp(enteredPassword, correctPassword) == 0) {
                    Serial.println("✅ Access Granted! LEDs will turn on.");
                    accessGranted = true; // Start LEDs
                } else {
                    Serial.println("❌ Wrong Password. Try again.");
                    clearInput();
                }
            }
        }
    }
 
    if (accessGranted) {
        while (true){
        runLedSequence();
        } 
        // accessGranted = false; // Reset
        // clearInput();
    }
}
 
// ✅ LED Flow Function (Using shiftOut)
void runLedSequence() {
    for (int i = 0; i < 8; i++) {
        shiftOutLED(1 << i);  
        delay(200);
    }
}
 
// ✅ Shift Out LED Control (No SPI)
void shiftOutLED(byte pattern) {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, pattern);
    digitalWrite(LATCH_PIN, HIGH);
}
 
// ✅ Clear Input Function
void clearInput() {
    memset(enteredPassword, 0, sizeof(enteredPassword));
    index = 0;
    Serial.println("Enter Password:");
}