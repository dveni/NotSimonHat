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
byte rowPins[COLS] = {13, 11, 10, 12}; // Connected to column pins of keypad
 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ✅ LCD Setup (RS, E, D4, D5, D6, D7)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change 0x27 to 0x3F if needed

 
// ✅ Shift Register (74HC595 for LEDs) (No SPI)
#define LATCH_PIN 5
#define DATA_PIN 3
#define CLOCK_PIN 2
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


// Show the Riddle with Delay
void showRiddle() {
    char buffer[17];  // Buffer to hold each line (16 chars + null terminator)
    
    // Display the riddle two lines at a time with delay
    for (int i = 0; i < 8; i += 2) {
        lcd.clear();
        
        // First line
        strcpy_P(buffer, (char*)pgm_read_word(&(riddle_lines[i])));
        lcd.setCursor(0, 0);
        lcd.print(buffer);
        
        // Second line (if available)
        if (i + 1 < 8) {
            strcpy_P(buffer, (char*)pgm_read_word(&(riddle_lines[i + 1])));
            lcd.setCursor(0, 1);
            lcd.print(buffer);
        }
        
        delay(3000);  // Show each pair of lines for 3 seconds
    }
    
    // After showing the riddle, return to password entry screen
    lcd.clear();
    lcd.print("Enter password:");
    Serial.println(F("Enter password:"));
}

// Scrolling text variables
const char welcomeMessageLine1[] PROGMEM = "Welcome Welcome Welcome Welcome ";
const char welcomeMessageLine2[] PROGMEM = "Dr. Dark Field! Dr. Dark Field! ";
unsigned long scrollPreviousMillis = 0;
const int scrollInterval = 300;  // Scroll speed in milliseconds
int scrollPosition = 0;
bool scrollingActive = false;

// Initialize the scrolling text
void startScrollingText() {
  scrollingActive = true;
  scrollPosition = 0;
  scrollPreviousMillis = millis();
  lcd.clear();
}

// Update the scrolling text - call this from loop()
void updateScrollingText() {
  if (!scrollingActive) return;
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - scrollPreviousMillis >= scrollInterval) {
    scrollPreviousMillis = currentMillis;
    
    // Get the length of both welcome messages
    int messageLength1 = strlen_P(welcomeMessageLine1);
    int messageLength2 = strlen_P(welcomeMessageLine2);
    
    // Clear the LCD
    lcd.clear();
    
    // Update first line
    for (int i = 0; i < 16; i++) {
      // Calculate the position in the message string
      int charPosition = (scrollPosition + i) % messageLength1;
      
      // Read the character from PROGMEM
      char c = pgm_read_byte(&welcomeMessageLine1[charPosition]);
      
      // Print the character at the appropriate LCD position
      lcd.setCursor(i, 0);
      lcd.print(c);
    }
    
    // Update second line
    for (int i = 0; i < 16; i++) {
      // Calculate the position in the message string
      int charPosition = (scrollPosition + i) % messageLength2;
      
      // Read the character from PROGMEM
      char c = pgm_read_byte(&welcomeMessageLine2[charPosition]);
      
      // Print the character at the appropriate LCD position
      lcd.setCursor(i, 1);
      lcd.print(c);
    }
    
    // Update scroll position for next time
    scrollPosition++;
    
    // Reset position if it exceeds both message lengths
    // This ensures smooth looping for both lines
    if (scrollPosition >= max(messageLength1, messageLength2)) {
      scrollPosition = 0;
    }
  }
}

// Stop the scrolling text
void stopScrollingText() {
  scrollingActive = false;
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
    Serial.println(F("LCD Initialized"));
    // Call the showRiddle function instead of directly printing "Enter password"
    if (riddleOn) {
        showRiddle();
    } else {
        lcd.print("Enter password:");
    }

    pinMode(BUZZER, OUTPUT);
    

    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0b00000000); // Turn off all LEDs
}
 
void loop() {
    
    
    char key = keypad.getKey();
    
 
    if (key) {
        Serial.println(F("Key Pressed: ")); Serial.println(key); // Debugging
 
        if (key == 'D') { 
            clearInput();
        } else if(key == 'C'){
          showRiddle();
        } else if (accessGranted && key == 'A'){
              running = false;  // Toggle LED sequence
              stopLEDs();
              stopScrollingText();  // Stop the scrolling text
              delay(200);  // Simple debounce
        }
         else if (accessGranted && key == 'B'){
              running = false;  // Toggle LED sequence
              stopLEDs();
              stopScrollingText();  // Stop the scrolling text
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
                    startScrollingText();
                    
                    
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
    // Update the scrolling text (does nothing if scrolling is not active)
    updateScrollingText();
        
}

// Function to stop LED sequence when button is pressed
void stopLEDs() {
    Serial.println(F("stopping leds"));
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0b00000000); // Turn off all LEDs
    digitalWrite(LATCH_PIN, HIGH);
    tone(BUZZER, 1000); // Send 1KHz sound signal...
    delay(5000);         // ...for 1 sec

    stopScrollingText();  // Stop the scrolling text
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


