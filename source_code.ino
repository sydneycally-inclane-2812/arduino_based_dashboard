/*
Objective: get time, information, time since vietnam, etc: done
print them to screen, screen address is 0x3C kinda done dont know why it doesnt work now

3 screens:
time: call showTime
sensors:
- DHT11: temp C/hum% H yeah this piece of shit died
- MQ2: vall Thresh: Thresh
- pir: stat/IRz: stat
- light: R/G/B
should create a separate function
sentry mode:
- toggle on/off by holding it for 5 seconds
- record time of first movement, how many times movement was detected : TODO
- blink red/blue when movement is recorded : done-ish, could use more polishing

The PIR sensor has a tendency to hallucinate, so sentry mode is not too reliable, but can be used to keep you feel safer, etc.
*/

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
// Importing all libraries
#include "RTClib.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Defining pins, 
#define ROW_SPACING 9 // Spacing of the rows displayed
#define MQ2_PIN A1 // Gas sensor
#define sw1 8 // Toggle light
#define sw2 7 // Toggle screen dimming
#define sw3 6 // Toggle status screen
#define sw4 5 // Toggle sentry mode
#define sw5 4

#define PIR_PIN 3
#define IR_PIN 2

#define R_PIN 11
#define G_PIN 10
#define B_PIN 9

// #define sw5 4 // Toggle quotes page, needs more work on that 

// Initial value of the RGB LED
int RValue = 0;
int GValue = 30;
int BValue = 150;

// Storing the number of the page it is on
int pageNumber = 1;
bool pageChangerLatch = false; // Latch for changing the page number

// Used this for setting the cursor for indentation, STILL IN USE
int indent = 35;

// Creating light counter and the latch for that toggle
int lightToggle = 0;
bool lightToggleLatch = false;

// Creating dim toggle and the latch for that toggle
bool dimToggle = false;
bool dimToggleLatch = false;

// Threshold for the gas sensor
int threshold = 500;


bool sentryLatch = false; // Latch for sentry mode (or page 3)
bool sentrySecondLatch = false; // Latch for getting the second when sentry mode is turned on
// bool sentryCountingLatch =  false; // Depreciated


bool airAlert = false; // Storing the state of air sensor to trigger a routine, which has not been coded
bool IRAlert = false; // Storing the state of the IR sensor to trigger some routine, also unfinished
bool motionAlert = false; //Storing the state of PIR sensor

bool sentryMode = false; // Storing the state of Sentry Mode
long int startingSecond = 0; // Storing the unix second when sentry mode is activated
long int SecondNow = 0; // Storing the current second

bool quoteToggle = false;
bool quoteToggleLatch = false;
bool randomQuoteLatch = false; // Stops it from refreshing before the button is pressed again
int RandNum = 0;

RTC_DS3231 rtc; // Setting up RTC

//const char quotesFromEveryone[3][50] PROGMEM = {"get home safe - Mian", "shh bbg - ntd", "take a lil break and watch an anime ep, you'll feel better - Do"};
const char s0[] PROGMEM = "get home safe\n- Mian";
const char s1[] PROGMEM = "shh bbg\n- ntd";
const char s2[] PROGMEM = "take a lil break and\nwatch an anime ep,\nyou'll feel better\n- Do";
const char s3[] PROGMEM = "are numbers even real\n- mnlt";
const char s4[] PROGMEM = "why do fuck and nail\nmean the same when\nyour mom, but when\nexams\n- mnlt";
const char s5[] PROGMEM = "it is not you who is\na simp, the simp is\nborn within you\n- ntd";
const char s6[] PROGMEM = "i'm not gay but 5$ is\n5$\n- yiank";
const char s7[] PROGMEM = "do not go gentle into\nthat good night,";
const char s8[] PROGMEM = "old age should burn\nand rave at close of\nday;";
const char s9[] PROGMEM = "rage, rage against\nthe dying of the\nlight.\n- Dylan Thomas";
const char s10[] PROGMEM = "why do you need a\ngirlfriend when you\ncan become girlfriend\n- khoachad";
const char s11[] PROGMEM = "tim femboi trong ban\nkinh 5km gan nha\n- ntd";
const char s12[] PROGMEM = "the first and best\nvictory is to conquer\nself\n- kiets1tg";
const char s13[] PROGMEM = "life is a d.\nit goes up sometimes and down other times\nbut it won't be hard\nforever\n- kiets1tg";
const char s14[] PROGMEM = "start where you are,\nknow what you have,\ndo what you can\n- Arthur Ashe/Mrs.\nBinh";
const char s15[] PROGMEM = "tell your heart that\nthe fear of suffering\nis worse than\nsuffering itself\n- Paul Coelho";


const char* const string_table[] PROGMEM = {s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15};
//const char* const string_table[] PROGMEM = {s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11};
char buffer[100];


//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"}; //Lookup table for week day
char daysOfTheWeek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"}; // Does not work, no idea why. Probably something to do with C++ that I'm to dumb to know

// Setting up SSD1306
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
int ledState = 255;
const long interval = 300;

void setup () {  
  Serial.begin(9600); // For debugging

  // Setting up SSD1306
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay(); // Setting up the display for the first time, clearing the display buffer to avoid the splash screen
  display.display(); // Pushing the update to the screen

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.cp437(true);


  // Setting up RTC
  // rtc.setClockMode(true);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  // Code for setting the time if the onboard battery fails, isn't that necessary or accurate even, and we could run another code to set the time if need be
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  /*
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  */

}

void loop() {
  // put your main code here, to run repeatedly:
  // Get the state of the switches
  bool sw1_state = digitalRead(sw1);
  bool sw2_state = digitalRead(sw2);
  bool sw3_state = digitalRead(sw3);
  bool sw4_state = digitalRead(sw4);
  bool sw5_state = digitalRead(sw5);

  // Get the state all sensors
  IRAlert = !digitalRead(IR_PIN);
  motionAlert = digitalRead(PIR_PIN);
  if (analogRead(MQ2_PIN) > threshold) {
    airAlert = true;
  } else {airAlert = false;}

  if (sw1_state == 1 && lightToggleLatch == false) {
    lightToggle = !lightToggle;
    lightToggleLatch = true;
  }
  if (sw1_state == 0 && lightToggleLatch == true) {lightToggleLatch = false;}

  // Defining the function of sw2 or dim toggle switch
  if (sw2_state == 1 && dimToggleLatch == false) {
    dimToggle = !dimToggle;
    dimToggleLatch = true;
  }
  if (sw2_state == 0 && dimToggleLatch == true) {dimToggleLatch = false;}

  // Defining the function of sw3 or page changer switch
  if (sw3_state == 1 && pageChangerLatch == false) {
    pageNumber++;
    pageChangerLatch = true;
    if (pageNumber > 4) {pageNumber = 1;}
  }
  if (sw3_state == 0 && pageChangerLatch == true) {pageChangerLatch = 0;}
  
  // switch 4 functionality - switching to the sentry page if clicked, if already on the page then activate/deactivate sentry mode
  if (sw4_state == 1 && sentryLatch == false) {
    if (pageNumber != 3) {
      pageNumber = 3;
      sentryLatch = true;
    } else {
      sentryMode = !sentryMode;
      sentryLatch = true;
    }
  }
  if (sw4_state == 0 && sentryLatch ==  true) {sentryLatch = false;} 

  // switch 5
  if (sw5_state == 1 && quoteToggleLatch == false) {
    if (pageNumber != 4) {
      pageNumber = 4;
    }
      quoteToggleLatch = true;
      randomQuoteLatch = true;
  }
  if (sw5_state == 0 && quoteToggleLatch == true) {quoteToggleLatch = false;}

  //showTime();
  switch (pageNumber) {
    case 1: // time page
      showTime();
      break;
    case 2:// sensor page
      showSensor();
      break;
    case 3:
      Sentry();
      break;
    case 4:
      showQuote();
  }

  setLight();
  SetDimOLED();
  /*
  Serial.print(lightToggle);
  Serial.print(' ');
  Serial.print(dimToggle);
  Serial.print(' ');
  Serial.print(pageNumber);
  Serial.print(' ');
  Serial.print(sentryMode);
  Serial.print(' ');
  Serial.print(quoteToggle);
  Serial.print(' ');
  Serial.println(motionAlert);
  Serial.print(' ');
  */
}

void setLight() {
  if (sentryMode == true && motionAlert == true) {
    analogWrite(R_PIN, 255);
    analogWrite(G_PIN, 0);
    analogWrite(B_PIN, 0);
  } else if (sentryMode == true && motionAlert == false) {
    analogWrite(R_PIN, 0);
    analogWrite(G_PIN, 0);
    analogWrite(B_PIN, 0);
  } else if (sentryMode == false && lightToggle == false) {
    analogWrite(R_PIN, 0);
    analogWrite(G_PIN, 0);
    analogWrite(B_PIN, 0);
  } else if (sentryMode == false && lightToggle == true) {
    analogWrite(R_PIN, RValue);
    analogWrite(G_PIN, GValue);
    analogWrite(B_PIN, BValue);
  }
}

void SetDimOLED() {
  if(dimToggle == true) {display.dim(true);} else {display.dim(false);}
}

void showTime() {
  // Display the first page - time and all

  // get the current time into now
  DateTime now = rtc.now();

  //Get all the values
  /*
  String year = String(now.year());
  String month = String(now.month());
  String day = String(now.day());
  String hour = String(now.twelveHour());
  String minute = String(now.minute());
  String second = String(now.second());
  */
  /*
  String AMPM;
  if (now.isPM() == true) {AMPM = "PM";}
  else {AMPM = "AM";}
  */
  // Serial.println(year + month + day + hour + minute + second + AMPM);
  // Serial.println(now.toString("DDD, DD MMM YYYY hh:mm:ss"));
  display.clearDisplay();
  // First line: Time: hh mm ss AP
  display.setCursor(0, ROW_SPACING*0);
  display.print("Time: ");
  display.setCursor(indent + 0, ROW_SPACING*0);
  display.print(now.twelveHour(), DEC);
  display.setCursor(indent + 16, ROW_SPACING*0);
  display.print(now.minute(), DEC);
  display.setCursor(indent + 32, ROW_SPACING*0);
  display.print(now.second(), DEC); 
  display.setCursor(indent + 48, ROW_SPACING*0);
  if (now.isPM() == true) {display.print("PM");}
  else {display.print("AM");}

  // Second line: Day: DayOfTheWeek, dd mm yyyy
  // Print the day of the week from the lookup table

  display.setCursor(0, ROW_SPACING*1);
  //display.print("Today is ");
  display.print(daysOfTheWeek[now.dayOfTheWeek()]);
  display.print(", ");
  
  // Print dd mm yyyy in the same row
  //display.setCursor(0, ROW_SPACING*1);
  display.print(now.day(), DEC);
  display.print(' ');
  //display.print(monthsOfTheYear[now.month()]);
  display.print(now.month(), DEC);
  display.print(' ');
  display.print(now.year(), DEC);

  //String timenow = now.toString("hh:mm:ss");

  // Third line: Print temperature
  display.setCursor(0, ROW_SPACING*2);
  display.print("Ambient: ");
  display.print(rtc.getTemperature());
  display.print(" C");

  // Skip fourth line
  // Fifth line: Last time since Vietnam
  display.setCursor(0, ROW_SPACING*4);
  display.print("*VN 1930+7 20231228");
  
  // Sixth line: Seconds since Vietnam
  display.setCursor(0, ROW_SPACING*5);
  display.print("Elapsed: ");
  display.print(now.unixtime() - 1703766975);
  display.print("s");
  display.setCursor(0, ROW_SPACING*6);
  display.print("or ");
  display.print((now.unixtime() - 1703766975) / 86400L);
  display.print("d");

  display.display();
}


void showSensor() {
  // Show stats of RGB, Gas, IR

  display.clearDisplay(); // Clear buffer
  // Print RGB value
  display.setCursor(0, ROW_SPACING*0);
  display.print("LED ");
  if (lightToggle > 0) {display.print("ON");} else {display.print("OFF");}
  display.setCursor(50, ROW_SPACING*0);
  display.print(RValue);
  display.print(" ");
  display.print(GValue);
  display.print(" ");
  display.print(BValue);
  display.print(" ");

  // Print MQ-2 Sensor value
  display.setCursor(0, ROW_SPACING*1);
  display.print("MH-2: ");
  display.print(analogRead(MQ2_PIN));
  display.print(" THRES ");
  display.print(threshold);

  // Print Danger or Safe
  display.setCursor(0, ROW_SPACING*2);
  if (airAlert == true) {
    display.print("DANGER");
  } else {
    display.print("SAFE");
  }

  display.setCursor(0, ROW_SPACING*3);
  display.print("IR: ");
  if (IRAlert == true) {
    display.print("ON");
  } else {
    display.print("OFF");
  }
  display.display();
}
void Sentry() {
  DateTime sentry_now = rtc.now();
  display.clearDisplay();
  display.setCursor(0, ROW_SPACING*0);
  display.print("Sentry Mode: ");
  if (sentryMode == true) {display.print("ON");} else {display.print("OFF");} // Print the state of SentryMode
  
  if (sentryMode == true && sentrySecondLatch == false) {
    startingSecond = sentry_now.unixtime(); // If on: get time
    sentrySecondLatch = true;
  }
  if (sentryMode == false && sentrySecondLatch == true) {sentrySecondLatch = false;} //release latch when off
  
  SecondNow = sentry_now.unixtime();
  // display.print(SecondNow - startingSecond);

    if (sentryMode == true) {
      display.setCursor(0, ROW_SPACING*1);
      display.print(F("Runtime: "));
      display.print(SecondNow - startingSecond);
      display.print('s');
      if (SecondNow - startingSecond <= 20) {
        display.setCursor(0, ROW_SPACING*2);
        display.print("You have ");
        display.print(20 - (SecondNow - startingSecond));
        display.print("s left");
      } else {
        display.setCursor(0, ROW_SPACING*2);
        // display.print("Motion detected: "); idk, but makes the screen black
        if (digitalRead(PIR_PIN) == 1) {
          display.print("YES");
        } else {
          display.print("NO");
        }
      }
    }
  display.display();
}


void showQuote() {
  display.clearDisplay();
  if (randomQuoteLatch == true) {
    RandNum = random(0, 16);
    strcpy_P(buffer, (char*)pgm_read_dword(&(string_table[RandNum]))); // Using 
    randomQuoteLatch = false;
  }
  display.setCursor(0, ROW_SPACING*0);
  display.print(buffer);
  display.display();
}

/* Weird to implement, somehow doesn't run (maybe memory issues) and not really needed so I'll leave it here
void blinkRed() {
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == 0) {
      ledState = 255;
    } else {
      ledState = 0;
    }
  analogWrite(R_PIN, ledState);
  Serial.println("Blinking Red");
  }
}
*/
