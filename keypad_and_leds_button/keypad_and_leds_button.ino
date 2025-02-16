#include <Keypad.h>
 #include <LiquidCrystal.h>

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

// ✅ LCD Setup (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(A5, A4, A3, A2, A1, A0);
 
// ✅ Shift Register (74HC595 for LEDs) (No SPI)
#define LATCH_PIN 5
#define DATA_PIN 3
#define CLOCK_PIN 2
#define BUTTON_PIN 4 // Connect button here

 
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

    // LCD Setup
    lcd.begin(16, 2);

    delay(500);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Text");
    Serial.println("LCD Initialized");

    // lcd.print("Enter Password:");

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.println("Enter Password:");
}
 
void loop() {
    char key = keypad.getKey();

    // DEBUG
    // while(true){
    //   Serial.println(digitalRead(BUTTON_PIN));
    //   delay(1000);
    // }
 
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
                    lcd.clear();
                    lcd.print("Access Granted!");
                    Serial.println("✅ Access Granted! LEDs will turn on.");
                    accessGranted = true; // Start LEDs
                    delay(1000);
                    lcd.clear();
                } else {
                    lcd.clear();
                    lcd.print("Wrong Password");
                    Serial.println("❌ Wrong Password. Try again.");
                    delay(1000);
                    clearInput();
                }
            }
        }
    }
 
    if (accessGranted) {
        while (accessGranted){
          if (digitalRead(BUTTON_PIN) == LOW) { 
            Serial.println("Stopping leds");
            stopLEDs();
          } else {
          Serial.println("Running sequence");
          runLedSequence();
          }
        } 
        // accessGranted = false; // Reset
        // clearInput();
    }
}

// Function to stop LED sequence when button is pressed
void stopLEDs() {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0b00000000); // Turn off all LEDs
    digitalWrite(LATCH_PIN, HIGH);

    accessGranted = false;
    clearInput();
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
    lcd.clear();
    lcd.print("Enter Password:");
}