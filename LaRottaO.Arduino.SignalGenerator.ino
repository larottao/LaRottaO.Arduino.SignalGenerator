
//Based on some of the awesome code by Cezar Chirila
//But instead uses the AD9833 library by Rob Tillaart
//My version offers a friendlier UI to select the desired frequency 
//with 1 encoder and 2 buttons

#include "LiquidCrystal_I2C.h"
#include "AD9833.h"
#include "Rotary.h"

String valueString = "000001000";
int cursorPosition = 5;

const unsigned long CLOCK_FREQ = 25000000; 
const unsigned long MAX_FREQ = 12500000;

int currentWaveModeIndex = 0;
String currentWaveMode = "SIN";

#define AD9833_OFF            0
#define AD9833_SINE           1
#define AD9833_SQUARE1        2
#define AD9833_SQUARE2        3
#define AD9833_TRIANGLE       4

// AD9833 and LCD initialization
AD9833 AD(10);
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD initialization
Rotary encoder(3, 2); // Encoder on pins 2 and 3 (interrupt pins)

// Button pin
const int encButton = 4;
const int buttonLeft = 5;
const int buttonRight = 6;
const int buttonUp = 7;
const int buttonDown = 8;

volatile bool encoderFlag = false;

const uint8_t arrow_up[] = {
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00000,
  0b00000
};



void setup() {

  Serial.begin(9600);

  while(!Serial);

  // Initialize the LCD
  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, arrow_up);  


  // Set pins A and B from encoder as interrupts
  attachInterrupt(digitalPinToInterrupt(2), encChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(3), encChange, CHANGE);

  // Initialize button pin as input with pull-up enabled
  pinMode(encButton, INPUT_PULLUP);
  pinMode(buttonLeft, INPUT_PULLUP);
  pinMode(buttonRight, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP); 

  SPI.begin();
  AD.begin();
  AD.setWave(AD9833_SINE);
  setAD9833Frequency(1000.0);
  updateFreqShownOnScreen();
  
}

void loop() {
  // Handle encoder and button interactions
  checkButtons();

  // Check encoder flag
  if (encoderFlag) {
    encoderFlag = false;
    handleEncoderChange();
  }  
}

// Function to print text on the specified line of the LCD
void printTextOnScreen(String text, int line) {
  lcd.setCursor(0, line);
  lcd.print(text);
}

void printArrowOnScreen(int pos, int line) {
  lcd.setCursor(pos, line);
  lcd.write((uint8_t)0);
}


void setAD9833Frequency(unsigned long frequencyInHz) {

    // Ensure frequency is within the allowed range
    if (frequencyInHz > MAX_FREQ) {
        frequencyInHz = MAX_FREQ;
        valueString = MAX_FREQ;
    } 
        AD.setFrequency(frequencyInHz, 0);    
}



// Interrupt service routine for encoder change
void encChange() {
  encoderFlag = true;
}

void handleEncoderChange() {
  unsigned char state = encoder.process();
  if (state == DIR_CW) {
    encoderValueIncreased();
  } else if (state == DIR_CCW) {
    encoderValueDecreased();
  }
}

void checkButtons() {
  static unsigned long lastButtonPress = 0;
  static unsigned char lastButtonStates[] = {HIGH, HIGH, HIGH, HIGH, HIGH};
  const int buttonPins[] = {encButton, buttonLeft, buttonRight, buttonUp, buttonDown};

  if ((millis() - lastButtonPress) > 100) { // Simple debounce
    for (int i = 0; i < 5; i++) {
      unsigned char buttonState = digitalRead(buttonPins[i]);
      if (buttonState != lastButtonStates[i]) {
        if (buttonState == LOW) {
          switch (i) {
            case 0:
              encoderButtonPressed();
              break;
            case 1:
              buttonLeftPressed();
              break;
            case 2:
              buttonRightPressed();
              break;
            case 3:
              buttonUpPressed();
              break;
            case 4:
              buttonDownPressed();
              break;
          }
        }
        lastButtonStates[i] = buttonState;
        lastButtonPress = millis();
      }
    }
  }
}

void encoderValueIncreased() {
  increaseCursorValue();
  updateFreqShownOnScreen();
  setAD9833Frequency(getValueAsULong());
}

void encoderValueDecreased() {
  decreaseCursorValue();
  updateFreqShownOnScreen();
  setAD9833Frequency(getValueAsULong());
}

void encoderButtonPressed() {
  Serial.println("encoderButtonPressed");
  changeWaveMode();
}

void buttonLeftPressed() {
  // Add your code to handle left button press
  Serial.println("buttonLeftPressed");
  moveCursorLeft();
  updateFreqShownOnScreen();
}

void buttonRightPressed() {
  // Add your code to handle right button press
  Serial.println("buttonRightPressed");
  moveCursorRight();
   updateFreqShownOnScreen();
 
}

void buttonUpPressed() {
  // Add your code to handle up button press
  Serial.println("buttonUpPressed");
}

void buttonDownPressed() {
  // Add your code to handle down button press
  Serial.println("buttonDownPressed");
}

void moveCursorRight() {
  if (cursorPosition < valueString.length() - 1) {
    cursorPosition++;
  }
}

void moveCursorLeft() {
  if (cursorPosition > 0) {
    cursorPosition--;
  }
}

void increaseCursorValue() {
  if (valueString[cursorPosition] < '9') {
    valueString[cursorPosition]++;
  }
  else{
    valueString[cursorPosition]='0';
    cursorPosition--;
    increaseCursorValue();
  }
}

void decreaseCursorValue() {
  if (valueString[cursorPosition] > '0') {
    valueString[cursorPosition]--;
  }
  else{
    if(cursorPosition==8){
      return;
    }
    valueString[cursorPosition]='0';
    cursorPosition++;
    valueString[cursorPosition]='9';
  }
}

unsigned long getValueAsULong() {
  char valueCharArray[10];
  valueString.toCharArray(valueCharArray, 10);
  return strtol(valueCharArray, NULL, 10);
}

String getValueAsString() {
  return valueString;
}

void updateFreqShownOnScreen(){
 lcd.clear();
 printTextOnScreen(getValueAsString() + " Hz " + currentWaveMode, 0);  
 printArrowOnScreen(cursorPosition, 1);
}

void changeWaveMode(){

currentWaveModeIndex++;

if(currentWaveModeIndex == 5){
  currentWaveModeIndex = 0;
}

switch(currentWaveModeIndex){

    case 0:
      AD.setWave(AD9833_OFF);
      Serial.print("WAVE CHANGED TO AD9833_OFF");
      currentWaveMode = "OFF";
      updateFreqShownOnScreen();
    break;
    case 1:
      AD.setWave(AD9833_SINE);
      Serial.print("WAVE CHANGED TO AD9833_SINE");
      currentWaveMode = "SIN";
      updateFreqShownOnScreen();
    break;
    case 2:
      AD.setWave(AD9833_SQUARE1);
      Serial.print("WAVE CHANGED TO AD9833_SQUARE1");
      currentWaveMode = "SQ1";
      updateFreqShownOnScreen();
    break;
    case 3:
      AD.setWave(AD9833_SQUARE2);
      Serial.print("WAVE CHANGED TO AD9833_SQUARE2");
      currentWaveMode = "SQ2";
      updateFreqShownOnScreen();    
    break;
    case 4:
      AD.setWave(AD9833_TRIANGLE);
      Serial.print("WAVE CHANGED TO AD9833_TRIANGLE");
      currentWaveMode = "TRI";
      updateFreqShownOnScreen();    
    break;  

  }

}
