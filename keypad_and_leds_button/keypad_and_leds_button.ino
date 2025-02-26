#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change 0x27 to 0x3F if needed

 
// ✅ Shift Register (74HC595 for LEDs) (No SPI)
#define LATCH_PIN 5
#define DATA_PIN 3
#define CLOCK_PIN 2
#define BUTTON_PIN 4 // Connect button here

 
const char correctPassword[] = "222222222"; //"28022025#";
char enteredPassword[10] = ""; // Store input
byte index = 0;
bool accessGranted = false; // Flag to start LEDs

const int numLeds = 8;
unsigned long previousMillis = 0;
int currentLed = 0;
const int interval = 200;
bool running = true;  // Track if sequence is running

void scrollText(const char* message, int delayTime = 300) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
    delay(3*delayTime);

    int textLength = strlen(message);
    if (textLength > 16) {  // Only scroll if text is longer than 16 characters
        for (int i = 0; i < textLength - 15; i++) {
            delay(delayTime);  // Adjust speed of scrolling
            lcd.scrollDisplayLeft();
        }
    }
    delay(2*delayTime);
}
 
void setup() {
    Serial.begin(9600);

    Wire.begin();
    Serial.println("Scanning...");
    
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            Serial.print("I2C Device found at 0x");
            Serial.println(address, HEX);
            break;
        }
    }
 
    // Shift Register Setup
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);

    // Initialize LCD
    lcd.init();         // Initialize the LCD
    lcd.backlight();    // Turn on backlight

    lcd.setCursor(0, 0);
    // lcd.print("Text");
    // while(true){
    //   scrollText("Alex ist eine fregi Sau");
    // }
    Serial.println("LCD Initialized");
    // lcd.print("Enter Password:");

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.println("Enter Password:");
    lcd.print("Enter Password:");
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
            lcd.clear();
            lcd.print(enteredPassword);
 
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
              running = false;  // Toggle LED sequence
              stopLEDs();
              delay(200);  // Simple debounce
          }else{
            Serial.println("Running sequence");
            runLedSequence();
          }
          }

        } 
        // accessGranted = false; // Reset
        // clearInput();
    
}

// Function to stop LED sequence when button is pressed
void stopLEDs() {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0b00000000); // Turn off all LEDs
    digitalWrite(LATCH_PIN, HIGH);

    accessGranted = false;
    clearInput();
    running=true;
}


// ✅ LED Flow Function (Using shiftOut)
void runLedSequence() {
    if (running) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            
            // Move to next LED
            currentLed = (currentLed + 1) % numLeds;

            // Turn on new LED
            shiftOutLED(1 << currentLed); 
        }
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


