#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>

File myFile;
#define file "mac.csv"

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define btn 6
String res = "", mac = "";

void press_button(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Press Button:");
  }

void file_error(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("File Error!");
  }

void show_count(int count){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MAC count:");
  lcd.setCursor(0,1);
  lcd.print(String(count));
  }

void waitforres(){
  long int startmillis = millis();
  while(!Serial1.available()){
    if((millis() - startmillis) >= 2000)
      break;
    }
  }
  
int findMAC(String recvMAC){
  String dbMAC = "";
  myFile = SD.open(file);
  if(myFile){
    while(myFile.available()){
      char buff = myFile.read();
      dbMAC += buff;
      if(buff == '\r'){
        dbMAC.trim();
//        Serial.println(dbMAC);
        if(dbMAC == recvMAC){
          myFile.close();
          return 1;
          }
        dbMAC = "";
        }
      }
    myFile.close();
    return -1;
    }
  else
    return 0;
  }
  
bool writeMAC(String recvMAC){
  myFile = SD.open(file, FILE_WRITE);
  if(myFile){
    myFile.println(recvMAC);
    myFile.close();
    return true;
    }
  else
    return false;  
  }
int countMAC(){
  myFile = SD.open(file);
  if(myFile){
    int i = 0;
    while(myFile.available()){
      if(myFile.read() == '\r')
        i++;
      }
    myFile.close();
    return i;
    }
  else
    return -1;
  }

void setup() {
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("ELEMENTZ");
  lcd.setCursor(0,1);
  lcd.print("MACCER");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0,0);

  //CS pin 53 is given as parameter
  if (!SD.begin(53)) {
    Serial.println("SD initialization failed!");
    lcd.print("SD failed!");
    while (1);
  }
  if(!SD.exists(file)){
    myFile = SD.open(file, FILE_WRITE);
    myFile.close();
    }
  Serial.println("SD initialization done.");
  lcd.print("SD Initialized...");

  Serial.begin(9600);
  Serial1.begin(38400);

  pinMode(btn, INPUT_PULLUP);
  
  delay(1000);
  show_count(countMAC());
  delay(2000);
  press_button();
}

void loop() {
  while(true){
    if(digitalRead(btn) == LOW){
      lcd.clear();
      lcd.print("Checking board...");
      delay(1000);
      //cleaning the bus
      Serial1.readString();
      delay(500);
      Serial1.write("AT\n\r");
      waitforres();
      if(Serial1.available()){
        res = Serial1.readString();
        res.trim();
        Serial.println(res);
        if(res != "OK"){
          Serial.println("Couldn't Communicate with board!");
          lcd.clear();
          lcd.print("Fix board!");
          delay(2000);
          press_button();
          continue;
          }
        else{
          lcd.clear();
          lcd.print("Board found.");
          delay(1000);
          }
        }
      else{
        Serial.println("Couldn't find the board!");
          lcd.clear();
          lcd.print("Board not found!");
          delay(2000);
          press_button();
          continue;
        }
      lcd.clear();
      lcd.print("Getting MAC...");
      delay(1000);
      Serial1.readString();
      Serial1.write("AT+ADDR\n\r");
      waitforres();
      if(Serial1.available()){
        res = Serial1.readString();
        Serial.println(res);
        int startIndex = 0, stopIndex = 0;
        for(int i = 0; i < res.length(); i++){
          if((res[i] == ':') && (startIndex == 0)){
            startIndex = i+1;
            }
          else if(res[i] == '\n'){
            stopIndex = i-1;
            break;
            }
          }
        mac = res.substring(startIndex, stopIndex);
        Serial.println(mac);
        }
      else{
        Serial.println("Connection to board lost!");
        lcd.clear();
        lcd.print("Board lost!");
        delay(2000);
        press_button();
        continue;
        }
      lcd.clear();
      lcd.print(mac);
      lcd.setCursor(0,1);
      lcd.print("Searching MAC...");
      delay(1000);
      int resFind = findMAC(mac);
      switch(resFind){
        case 0: Serial.println("Unable to open file!");
                file_error();
                break;
        case 1: Serial.println("MAC ID already exists!");
                lcd.setCursor(0,1);
                lcd.print("                ");
                lcd.setCursor(0,1);
                lcd.print("MAC ID exists!");
                break;
        case -1: Serial.println("MAC ID does not exist...");
                 lcd.setCursor(0,1);
                 lcd.print("                ");
                 lcd.setCursor(0,1);
                 lcd.print("MAC not found...");
                 break;
        }
      if((resFind == 0) || (resFind == 1)){
        delay(2000);
        press_button();
        continue;
        }
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Adding MAC...");
      delay(1000);
      if(writeMAC(mac) == false){
        Serial.println("Unable to add MAC!");
        file_error();
        delay(2000);
        press_button();
        continue;
        }
      else{
        if(findMAC == -1){
          Serial.println("Issue adding MAC!");
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Add failed!");
          delay(2000);
          press_button();
          continue;
          }
        Serial.println("MAC added successfully!");
        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("MAC added!");
        while(!digitalRead(btn) == LOW);
        show_count(countMAC());
        delay(2000);
        press_button();
        continue;
        }
      }
    }
  }
