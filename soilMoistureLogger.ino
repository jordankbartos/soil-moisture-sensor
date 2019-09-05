/********************************************************************************************
 *                 First "permanent-build" soil-moisture sensor setup
 *                             Jordan Bartos
 *                               08/19/2018
 *                               
 * This soil moisture sensor reads soil moisture from four capacitive soil moisture sensors                              
 * at a pre-set interval and records the data to a micro-SD.
 * 
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 7
 * ((indicator LED set at D_6))
 * 
 * LCD attached via pins:
 * RS: 10
 * EN: 9
 * D_4: 5
 * D_5: 4
 * D_6: 3
 * D_7: 2
 *
 *Soil moisture sensors:
 *sensor1: A0
 *sensor2: A1
 *sensor3: A2
 *sensor4: A3
 **********************************************************************************************/

//include necessary files
#include <SPI.h>
#include "SdFat.h"
#include <LiquidCrystal.h>

//these variables control the timing of the program
const int TOTAL_DELAY_BETWEEN_READINGS = 30; //minutes
const int WARNING_TIME = 10; //seconds

//declare file object and LiquidCrystal object
const int RS = 10, EN = 9, D_4 = 5, D_5 = 4, D_6 = 3, D_7 = 2, led = 6;
LiquidCrystal lcd(RS, EN, D_4, D_5, D_6, D_7);
SdFat SD;

//soil moisture sensor pins
const int MOISTURE_SENSOR_ONE = A0;
const int MOISTURE_SENSOR_TWO = A1;
const int MOISTURE_SENSOR_THREE = A2;
const int MOISTURE_SENSOR_FOUR = A3;

//function definitions
void blinkWarning();
void delayMinutes(unsigned int);
void getReadings(int&,int&,int&,int&);
void displayValues(int&,int&,int&,int&);
void printToFile(File&,int&,int&,int&);

//set the file name of the data file where soil moisture data will be stored
String fileName = "soilDat.txt";

// set DEBUGGING to true to have debugging info printed to the Serial monitor
const bool DEBUGGING = false;
int count = 0;

/******************************************************************************************
 *                                void setup()
 * the setup for the soil moisture sensor and components
 *  - initializes the SD card reader and ensures it works before proceeding
 *  - begins the Serial connection if in debugging mode
 ******************************************************************************************/
void setup() 
{
  
  File dataLog;
  //initialize the LCD display
  lcd.begin(16,2);
  pinMode(led,OUTPUT);
  
  lcd.print("Setup is running");
  lcd.setCursor(0,1);
  lcd.print(F("..."));
  delay(2000);
  
  //if SD cannot initialize, blink an error pattern
  if (!SD.begin(7)) 
  {
    lcd.clear();
    lcd.print(F("SD Card error"));
    //blink an error pattern indefinitely and do not continue with the program
    //the SD card must initialize properly to proceed
    while(true)
    {
      digitalWrite(led,HIGH);
      delay(1000);
      digitalWrite(led,LOW);
      delay(500);
    }
  }
  
  //else display success message, turn on led and continue
  else
  {
    //print table headers to SD card text file
    dataLog = SD.open(fileName,FILE_WRITE);
    dataLog.print(F("***********************\n"));
    dataLog.print(F("Interval: "));
    dataLog.print(TOTAL_DELAY_BETWEEN_READINGS);
    dataLog.print(F(" minutes apart."));
    dataLog.print(F("num\tS1\tS2\tS3\tS4\n"));
    
    //if debugging mode is on, the light will turn on during as an indicator
    if(DEBUGGING) 
    {
      digitalWrite(led,HIGH);
    }

    //display initialization success msg to the LCD display
    lcd.clear();
    lcd.print(F("SD card "));
    lcd.setCursor(0,1);
    lcd.print(F("initialized..."));
    dataLog.close();
    delay(2000);
  }

  //for debugging purposes, activate the serial connection for
  //data read-out when connected to computer
  Serial.begin(9600);  
  if(DEBUGGING)
  {

    Serial.print("debugging mode on\n");
  }
}

/***********************************************************************************
 *                             void loop()
 *   the main loop of the program
***********************************************************************************/
void loop() 
{
  Serial.print("loop\n");
  //count number of recordings the logger has made
  count++;
  File dataLog;

  //turn on the LED during each reading to indicate the dubugging mode is on
  if(DEBUGGING)
  {
    digitalWrite(led,HIGH);
  }

  //blink the warning light to prevent removal of SD card during reading
  blinkWarning(); 
  
  dataLog = SD.open(fileName,FILE_WRITE);

  int moistureOne = 0,
      moistureTwo = 0,
      moistureThree = 0,
      moistureFour = 0;

  //open the file. If the file opened successfully, collect readings, write the results to the screen
  //and the log file
  dataLog = SD.open(fileName,FILE_WRITE);
  if(dataLog)
  {
    //get values from sensors, display the values to the LCD screen, and print the values
    //in the log file
    getReadings(moistureOne,moistureTwo,moistureThree,moistureFour);
    displayValues(moistureOne,moistureTwo,moistureThree,moistureFour);
    printToFile(dataLog,moistureOne,moistureTwo,moistureThree,moistureFour);
  } 
  else 
  {
    // if the file didn't open, print an error:
    lcd.clear();
    lcd.print(F("SD ERROR"));
  }
  
  //close the file and turn off the LED
  dataLog.close();
  digitalWrite(led,LOW);

  //delay to take one reading every TOTAL_DELAY_BETWEEN_READINGS minutes
  //timeAdjustment is the amount needed to delay to round remaining total delay
  //to the next length in minutes
  unsigned int timeAdjustmentSeconds = ((TOTAL_DELAY_BETWEEN_READINGS * 60) - WARNING_TIME) % 60;
  
  //if debugging mode is on, display this data to the Serial connection before the long delay
  if(DEBUGGING){
    Serial.print("Time delay calculations:\n");
    Serial.print("timeAdjustmentSeconds = ");
    Serial.print(timeAdjustmentSeconds);
    Serial.print("\ndelayMinutes = ");
    Serial.print(TOTAL_DELAY_BETWEEN_READINGS - (timeAdjustmentSeconds + WARNING_TIME) / 60);
    Serial.print("\n");
  }
  
  delay(timeAdjustmentSeconds * 1000);
  //delay for remaining time
  delayMinutes(TOTAL_DELAY_BETWEEN_READINGS - (timeAdjustmentSeconds + WARNING_TIME) / 60);
}


/***********************************************************************************
 *                       void blinkWarning()
 * blinks the led for 3 seconds to warn the SD card is about to be written to. This
 * will help prevent accidentally removing the card in the middle of a read/write
 * cycle. It is safe to remove the card at any other time as the file is closed and
 * the card is not being used.
 * Delays six seconds
 **********************************************************************************/
void blinkWarning()
{
  //number of loops necessary depends upon WARNING_TIME desired for the LED
  //there are 1000 ms per second and each loop takes 500ms, so the number of loops 
  //needed is twice the length of the desired warning time
  for(int i = 0; i < (WARNING_TIME * 2); ++i)
  {
    digitalWrite(led,HIGH);
    delay(250);
    digitalWrite(led,LOW);
    delay(250);
  }
}

/**************************************************************************************
 *                        void delayMinutes(int)
 * delays the program for a number of minutes (approximately)
***************************************************************************************/
void delayMinutes(unsigned int minutes)
{
  for(int i = 0; i < minutes; i++)
  {
    delay(60000); //1000 milliseconds per second * 60 seconds
  }
}

/******************************************************************************************
 *                        void getReadings(int&,int&,int&,int&)
 * takes references to four ints as input, updates them with the readings from the four 
 * sensors.
 *****************************************************************************************/
void getReadings(int& s1, int& s2, int& s3, int& s4)
{
  //read the sensors
  int sen1 = analogRead(MOISTURE_SENSOR_ONE);
  int sen2 = analogRead(MOISTURE_SENSOR_TWO);
  int sen3 = analogRead(MOISTURE_SENSOR_THREE);
  int sen4 = analogRead(MOISTURE_SENSOR_FOUR);

  //map the values and update the variables
  //the calibration numbers must be gathered manually from each sensor
  s1 = map(sen1,642,325,0,100);    // dry = 640 wet = 325
  s2 = map(sen2,648,332,0,100);    // dry = 649 wet = 331
  s3 = map(sen3,638 ,324,0,100);    // dry = 637 wet = 324
  s4 = map(sen4,643,324,0,100);    // dry = 643 wet = 324
}

/*******************************************************************************************
 *                               void displayValues(int&,int&,int&,int&)
 * takes references to four ints as input, displays their values with headers to the LCD
 ******************************************************************************************/
void displayValues(int& s1, int&s2, int& s3, int& s4)
{
  lcd.clear();
  lcd.print(F("S1:")); lcd.print(s1); lcd.print("%");
  
  lcd.setCursor(9,0);
  lcd.print(F("S2:")); lcd.print(s2); lcd.print("%");
  
  lcd.setCursor(0,1);
  lcd.print(F("S3:")); lcd.print(s3); lcd.print("%");
  
  lcd.setCursor(9,1);
  lcd.print(F("S4:")); lcd.print(s4); lcd.print("%");
}


/**********************************************************************************************
 *                          void printToFile(File&, int&, int&, int&, int&)
 * Takes a reference to a file object and references to four integers as parameters.
 * Formats the integers appropriately and writes them to the data file
 *********************************************************************************************/
void printToFile(File& f, int& s1, int& s2, int& s3, int& s4)
{
  f.print(count);
  f.print(F("\t"));
  f.print(s1);
  f.print(F("\t"));
  f.print(s2);
  f.print(F("\t"));
  f.print(s3);
  f.print(F("\t"));
  f.print(s4);
  f.print(F("\n"));
}
