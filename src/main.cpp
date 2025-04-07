// SPI - Version: Latest 
#include <Arduino.h>
#include <Bounce2.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "pitches.h"

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// notes in the melody:
int melody[] = {

  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {

  4, 8, 8, 4, 4, 4, 4, 4
};

/*
  A reaction game
*/

const int LED[4] = { 52,50,48,46 };
const int BUTTON[4] = { 44,42,40,38 };
const int NUMBER_BUTTONS = 4;

int piezoPin = A8;

int ledTimeDur = 1000; //ms

Bounce * buttons = new Bounce[NUMBER_BUTTONS];

const bool DEBUG = false;
int score = 0;

bool isLEDon = false;
unsigned long blinkTime;


void Debug (String text, unsigned long value)
{
  if (DEBUG)
  {
    Serial.print(text);
    Serial.println(value);
  }
}

void DebugStr (String text, String value)
{
  if (DEBUG)
  {
    Serial.print(text);
    Serial.println(value);
  }
}

void IncreaseScore()
{
    ledTimeDur = (score <   10) ? 1000
               : (score <=  20) ? 900
               : (score <=  30) ? 800
               : (score <=  40) ? 700
               : (score <=  50) ? 600
                                : 600;
}

/*
  Set a button
*/
void SetLED (int button, int led)
{
  if (button != HIGH)
    digitalWrite(led, LOW);
  else
    digitalWrite(led, HIGH);
}

void SetLEDs (int pattern)
{
  Debug("SetLEDs: ", pattern);
  int bitPattern = 0x01;
  
  for(int i=0;i<NUMBER_BUTTONS;i++)
  {
    if ((pattern & bitPattern) != 0)
      SetLED(HIGH, LED[i]);
    else
      SetLED(LOW, LED[i]);

    bitPattern = bitPattern << 1;
  }
}


/*
  Check to see if any button is pressed
*/
bool CheckButtons()
{
  for (int i=0; i< NUMBER_BUTTONS; i++)
  {
    if(digitalRead(BUTTON[i]) > 0)
    {
      Debug("CheckButtons() - pressed: ", i);
      return true;
    }
  }

  Debug("CheckButtons() - NOT PRESSED - ", -1);
  return false;
}

bool CheckRightButton(int realButton, int pressedButton)
{
  if(realButton == pressedButton)
  {
    return true;
  }
  else
  {
    return false;
  }

}

void showScore(){
  lcd.clear();
  lcd.print("Score");
  lcd.setCursor(0,1);
  lcd.print(score);
}

void defaultScreen() {
  lcd.clear();
  lcd.print("Select game");
  lcd.setCursor(0,1);
  lcd.print("Red or Blue");
}

/*
  Play the game
*/
void PlayGame()
{
  unsigned long endTime = millis() + 31000;
  unsigned long ledTime = millis() + ledTimeDur;

  
  Debug("endTime: ", endTime);
  Debug("millis(): ", millis());

  long randButton = random(0,NUMBER_BUTTONS);
  Debug("Random number: ", randButton);

  // Set the button to press as lit
  SetLED(HIGH, LED[randButton]);
  Debug("SetLED: ", LED[randButton]);
  
  while (millis() < endTime)
  {
    //If timer runs out we switch the button
    if (millis() > ledTime)
    {
      Debug("Missed timer on button: ", LED[randButton]);
      SetLED(LOW, LED[randButton]);
      // Get the new button and check it has changed
      int newButton = random(0,NUMBER_BUTTONS);
      while(newButton == randButton)
      {
        newButton = random(0,NUMBER_BUTTONS);
      }
      
      // Values differ, so continue
      randButton = newButton;
      SetLED(HIGH, LED[randButton]);
      Debug("SetLED: ", LED[randButton]);
      ledTime = millis() + ledTimeDur;
    }

    // Set up the pins
    for (int i = 0; i < NUMBER_BUTTONS; i++)
    {
      buttons[i].update();
      if(buttons[i].fell())
      {
        if(CheckRightButton(randButton, i))
        {
          score ++;
          showScore();
          // Turn off the button
          SetLED(LOW, LED[randButton]);
          
          // Get the new button and check it has changed
          int newButton = random(0,NUMBER_BUTTONS);
          while(newButton == randButton)
          {
            newButton = random(0,NUMBER_BUTTONS);
          }
          
          // Values differ, so continue
          randButton = newButton;
          SetLED(HIGH, LED[randButton]);
          Debug("SetLED: ", LED[randButton]);
          ledTime = millis() + ledTimeDur;
          IncreaseScore();
        }
        else
        {
          Debug("Wrong button pressed: ", i);
          tone(piezoPin, 500, 500);
        }
        
      }
    }
  }  
  // Play endgame melody
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(piezoPin, melody[thisNote], noteDuration);
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(piezoPin);
  }

  SetLEDs(0);
  showScore();
  delay(4000);
  defaultScreen();
  Debug("Score--------------------------------------------: ", score);
}


void PlayGameSpeed()
{
  
  bool wrongButtonPressed = false;

  unsigned long ledTime = millis() + ledTimeDur;
  
  Debug("millis(): ", millis());

  long randButton = random(0,NUMBER_BUTTONS);
  Debug("Random number: ", randButton);

  // Set the button to press as lit
  SetLED(HIGH, LED[randButton]);
  Debug("SetLED: ", LED[randButton]);
  
  while (!wrongButtonPressed)
  {
    //If timer runs out we switch the button
    if (millis() > ledTime)
    {
      Debug("Missed timer on button: ", LED[randButton]);
      SetLED(LOW, LED[randButton]);
      // Get the new button and check it has changed
      int newButton = random(0,NUMBER_BUTTONS);
      while(newButton == randButton)
      {
        newButton = random(0,NUMBER_BUTTONS);
      }
      
      // Values differ, so continue
      randButton = newButton;
      SetLED(HIGH, LED[randButton]);
      Debug("SetLED: ", LED[randButton]);
      ledTime = millis() + ledTimeDur;
    }

    for (int i = 0; i < NUMBER_BUTTONS; i++)
    {
      buttons[i].update();
      if(buttons[i].fell())
      {
        if(CheckRightButton(randButton, i))
        {
          score ++;
          showScore();
          // Turn off the button
          SetLED(LOW, LED[randButton]);
          
          // Get the new button and check it has changed
          int newButton = random(0,NUMBER_BUTTONS);
          while(newButton == randButton)
          {
            newButton = random(0,NUMBER_BUTTONS);
          }
          
          // Values differ, so continue
          randButton = newButton;
          SetLED(HIGH, LED[randButton]);
          Debug("SetLED: ", LED[randButton]);
          ledTime = millis() + ledTimeDur;
          ledTimeDur = ledTimeDur - 5;
          if (ledTimeDur < 500)
          {
            ledTimeDur = 500;
          }
          Debug("ledTimeDur: ", ledTimeDur);
        }
        else
        {
          Debug("Wrong button pressed: ", i);
          tone(piezoPin, 500, 500);
          wrongButtonPressed = true;
        }
        
      }
    }
  }  
  // Play endgame melody
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(piezoPin, melody[thisNote], noteDuration);
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(piezoPin);
  }

  SetLEDs(0);
  showScore();
  delay(4000);
  defaultScreen();
  Debug("Score--------------------------------------------: ", score);
}

/*
  Countdown timer, flashes on and off
*/
void CountDown()
{
  for (int i=1;i<10;i++)
  {
    SetLEDs(255);
    delay(1000/i);
    SetLEDs(0);
    delay(1000/i);
  }
  delay(500);
}

/*
  Set up the game
*/
void setup() {
  // Set up the seed
  randomSeed(analogRead(0));

  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  Serial.begin(9600);
  
  // Set up the pins
  for (int i = 0; i < NUMBER_BUTTONS; i++)
  {
    pinMode(LED[i], OUTPUT);
    buttons[i].attach( BUTTON[i], INPUT_PULLUP );      //setup the bounce instance for the current button
    buttons[i].interval(25);              // interval in ms
  }
  blinkTime = millis() + 1000;
  defaultScreen();
}

/*
  The main loop
*/
void loop() 
{
  Debug("DEBUG ON - ", 0);

  if(digitalRead(BUTTON[0]) == LOW)
  {
    lcd.clear();
    lcd.print("Time game");
    lcd.setCursor(0,1);
    lcd.print("Hit as many");
    ledTimeDur = 1000;
    score = 0;
    tone(piezoPin, 2000, 1000);
    Serial.println("Time Game starting:");
    // Countdown to start
    CountDown();

    lcd.clear();
    lcd.print("GO GO GO");
    // Play the game
    PlayGame();
  }

  if(digitalRead(BUTTON[1]) == LOW)
  {
    lcd.clear();
    lcd.print("Error game");
    lcd.setCursor(0,1);
    lcd.print("Hit until wrong");
    ledTimeDur = 1000;
    score = 0;
    tone(piezoPin, 2000, 1000);
    Serial.println("Until wrong Game starting:");
    // Countdown to start
    CountDown();

    lcd.clear();
    lcd.print("GO GO GO");
    // Play the game
    PlayGameSpeed();
  }


  if(millis() > blinkTime)
  {
    blinkTime = millis() + 1000;
    isLEDon = !isLEDon;
    if(isLEDon)
    {
      SetLEDs(255);
    }
    else
    {
      SetLEDs(0);
    }
  }

  
}
