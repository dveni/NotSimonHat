#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "songs.h"  // Include the header file
#include <avr/pgmspace.h>


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
// byte colPins[ROWS] = {9, 8, 7, 6};   // Connected to row pins of keypad
byte rowPins[COLS] = {13, 11, 10, 12}; // Connected to column pins of keypad
 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ✅ LCD Setup (RS, E, D4, D5, D6, D7)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change 0x27 to 0x3F if needed

 
// ✅ Shift Register (74HC595 for LEDs) (No SPI)
#define LATCH_PIN 5
#define DATA_PIN 3
#define CLOCK_PIN 2
// #define BUTTON_PIN 4 // Connect button here
#define BUZZER A0 //buzzer to arduino pin


bool riddleOn = true;
const char correctPassword[] = "28022025#";
char enteredPassword[10] = ""; // Store input
byte index = 0;
bool accessGranted = false; // Flag to start LEDs

const char riddle_1[] PROGMEM = "Two plus eight";
const char riddle_2[] PROGMEM = "makes me whole.";
const char riddle_3[] PROGMEM = "The shortest";
const char riddle_4[] PROGMEM = "month plays role.";
const char riddle_5[] PROGMEM = "The year of";
const char riddle_6[] PROGMEM = "success,";
const char riddle_7[] PROGMEM = "Tell me now";
const char riddle_8[] PROGMEM = "who am I?";



const int numLeds = 8;
unsigned long previousMillis = 0;
int currentLed = 0;
const int interval = 100;
bool running = true;  // Track if sequence is running

unsigned long scrollPreviousMillis = 0;
const int scrollInterval = 300;  // Scrolling speed
const int startDelay = 300;     // Wait before scrolling starts
const int endDelay = 300;       // Wait after scrolling ends

int scrollIndex = 0;
String scrollMessage = "";
bool scrolling = false;
bool waitingStart = false;
bool waitingEnd = false;
unsigned long waitStartMillis = 0;
unsigned long waitEndMillis = 0;

void startScrolling(const char* message) {
    scrollMessage = " " + String(message) + " ";  // Add spaces for smooth scrolling
    scrollIndex = 0;
    scrolling = false;  // Wait before starting scroll
    waitingStart = true;
    waitStartMillis = millis();
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(scrollMessage.substring(0, 16));  // Show first part of the message
}

void updateScrolling() {
    if (!accessGranted){
      return;
    }
    Serial.println(F("Update scrolling called"));
    unsigned long currentMillis = millis();

    // Print current state
    Serial.print(F("Scroll state: waiting="));
    Serial.print(waitingStart);
    Serial.print(F(" scrolling="));
    Serial.print(scrolling);
    Serial.print(F(" waitEnd="));
    Serial.println(waitingEnd);

    // Wait before scrolling starts
    if (waitingStart) {
        if (currentMillis - waitStartMillis >= startDelay) {
            waitingStart = false;
            scrolling = true;  // Now start scrolling
            scrollPreviousMillis = millis();  // Reset scrolling timer
        }
        return; // Don't proceed further if waiting
    }

    // Scroll the message if scrolling is active
    if (scrolling) {
        if (currentMillis - scrollPreviousMillis >= scrollInterval) {
            scrollPreviousMillis = currentMillis;

            if (scrollIndex < scrollMessage.length() - 16) {
                scrollIndex++;
            } else {
                scrolling = false;  // Stop scrolling
                waitingEnd = true;   // Start waiting before clearing
                waitEndMillis = millis();
            }

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(scrollMessage.substring(scrollIndex, scrollIndex + 16));
        }
    }

    // Wait at the end before stopping scrolling
    if (waitingEnd && accessGranted) {
        if (currentMillis - waitEndMillis >= endDelay) {
            waitingEnd = false;
            lcd.clear();
            scrolling = false;
            startScrolling(scrollMessage.c_str());  // Restart scrolling
        }
    }
}

// ✅ Shift Out LED Control (No SPI)
void shiftOutLED(byte pattern) {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, pattern);
    digitalWrite(LATCH_PIN, HIGH);
}

// ✅ Store Pointers to PROGMEM Strings in an Array
const char* const riddle_lines[] PROGMEM = {
    riddle_1, riddle_2, riddle_3, riddle_4,
    riddle_5, riddle_6, riddle_7, riddle_8
};

// ✅ Function to Display a Line from PROGMEM
void displayRiddleLine(int index) {
    char buffer[17]; // LCD supports 16 chars per line + null terminator
    strcpy_P(buffer, (char*)pgm_read_word(&(riddle_lines[index])));

    // lcd.clear();
    // lcd.setCursor(0, 0);
    lcd.print(buffer);
}

// ✅ Show the Riddle with Delay
void showRiddle() {
    for (int i = 0; i < 8; i += 2) {
        displayRiddleLine(i);
        lcd.setCursor(0, 1);  // Move to the second line
        displayRiddleLine(i + 1);
        delay(500);  // Show for 3 seconds
        lcd.clear();
    }

    lcd.clear();
    lcd.print("Enter Password:");
}

 
void setup() {
    Serial.begin(9600);

    Wire.begin();
    Serial.println(F("Scanning...")); 
    
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            Serial.println(F("I2C Device found at 0x"));
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
    Serial.println(F("LCD Initialized"));
    // lcd.print("Enter Password:");
    // showRiddle();

    // pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER, OUTPUT);
    // Serial.println(F("Enter Password:"));
    
    // char buffer[100];  // Temporary buffer to store string
    // strcpy_P(buffer, riddle);
    // startScrolling(buffer);
    // startScrolling(riddle);

    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0b00000000); // Turn off all LEDs
}
 
void loop() {
    
    // if (riddleOn){
    //   showRiddle();
    //   riddleOn=false;
    // }
    char key = keypad.getKey();
    // Serial.println(key)

    // DEBUG
    // while(true){
    //   Serial.println(digitalRead(BUTTON_PIN));
    //   delay(1000);
    // }
 
    if (key) {
        Serial.println(F("Key Pressed: ")); Serial.println(key); // Debugging
 
        if (key == 'D') { 
            clearInput();
        } else if(key == 'C'){
          riddleOn = true;
        } else if (accessGranted && key == 'A'){
              running = false;  // Toggle LED sequence
              stopLEDs();
              delay(200);  // Simple debounce
        }
         else if (accessGranted && key == 'B'){
              running = false;  // Toggle LED sequence
              stopLEDs();
              thelionsleeptonight(BUZZER);
              delay(200);  // Simple debounce
        }
         else {
            enteredPassword[index] = key;
            index++;
 
            // Print input to Serial Monitor
            Serial.println(F("Current Input: "));
            Serial.println(enteredPassword);
            lcd.clear();
            lcd.print(enteredPassword);
 
            // Check if password is complete
            if (index == 9) {
                enteredPassword[9] = '\0'; // Null-terminate string
 
                if (strcmp(enteredPassword, correctPassword) == 0) {
                    lcd.clear();
                    lcd.print("Access Granted!");
                    Serial.println(F("✅ Access Granted! LEDs will turn on."));
                    accessGranted = true; // Start LEDs
                    delay(1000);
                    lcd.clear();
                    delay(200);
                    Serial.println(F("Starting welcome scroll"));
                    startScrolling("Welcome Dr. Dark Field!Welcome Dr. Dark Field!Welcome Dr. Dark Field!");
                    // Force initial display
                    lcd.setCursor(0, 0);
                    lcd.print(scrollMessage.substring(0, 16));
                    
                } else {
                    lcd.clear();
                    lcd.print("Wrong Password");
                    delay(1000);
                    clearInput();
                }
            }
        }
    }
    

    if (accessGranted) {    
      runLedSequence();
    }
    // updateScrolling(); 
        
}

// Function to stop LED sequence when button is pressed
void stopLEDs() {
    Serial.println(F("stopping leds"));
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0b00000000); // Turn off all LEDs
    digitalWrite(LATCH_PIN, HIGH);
    tone(BUZZER, 1000); // Send 1KHz sound signal...
    delay(5000);         // ...for 1 sec

    clearInput();
    accessGranted = false;
    tone(BUZZER, 3000); // Send 1KHz sound signal...
    delay(200); 
    // jigglypuff(BUZZER);
    noTone(BUZZER);     // Stop sound...
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
            // Serial.println(currentLed);
            // Turn on new LED
            shiftOutLED(1 << currentLed); 
        }
    }
    
}
 

 
// ✅ Clear Input Function
void clearInput() {
    memset(enteredPassword, 0, sizeof(enteredPassword));
    index = 0;
    lcd.clear();
    lcd.print("Enter Password:");
    Serial.println(F("Enter password:"));
} 


