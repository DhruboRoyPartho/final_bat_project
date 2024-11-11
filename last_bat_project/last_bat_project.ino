// Project: BAT project
// Code Author: Dhrubo Roy Partho
// Date: 18/06/2024
// Version: 3.0v
// Upgrade: touch display + log data bug fix + 3 Screen + more customizability

#include <FS.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "Free_Fonts.h"
#include <Keypad.h>

TFT_eSPI tft = TFT_eSPI(); // Create TFT instance

// Touch calibration
#define CALIBRATION_FILE "/TouchCalData1"
// Calibration Repeat
#define REPEAT_CAL false

uint16_t x = 0, y = 0;
int textWidth = 60;  // Estimated text width (or calculate dynamically)
int textHeight = 28;
#define offset_row 20
#define offset_col 0.5
#define LINE tft.setCursor(offset_col, offset_row + tft.getCursorY())

char homeKeyLabel[4][7] = {"MANUAL", "AUTO", "PHASE", "RESET"};

#define CUSTOM_PURPLE   tft.color565(255-244, 255-92, 255-250)   // Purple
#define CUSTOM_LIGHT_PINK  tft.color565(255-92, 255-250, 255-122)  // Light Pink
#define CUSTOM_LIGHT_RED   tft.color565(255-79, 255-87, 255-249)  // Light Red
#define CUSTOM_LIGHT_BLUE  tft.color565(255-248, 255-79, 255-79)  // Light Blue

// Relay pin
#define relay1 4
#define relay2 15

uint16_t homeKeyColor[4] = {CUSTOM_PURPLE, CUSTOM_LIGHT_PINK, CUSTOM_LIGHT_RED, CUSTOM_LIGHT_BLUE};

// home key instances
TFT_eSPI_Button homeKey[4];

// manual keypad key instance
TFT_eSPI_Button manualKeypadKey[16];
char* manualKeypadKeyLabel[16] = {"1", "2", "3", "U", 
                                  "4", "5", "6", "D",
                                  "7", "8", "9", "<-",
                                  ".", "0", ":", "OK"};

// manual key instance
TFT_eSPI_Button manualKey[4];
char* manualKeyLabel[4] = {"HOME", "SAVE", "START", "RESET"};

// manual phase key instance
TFT_eSPI_Button manualPhaseKey[4];
char* manualPhaseKeyLabel[4] = {"YELLOW", "LAMINA", "COLOR F.", "STEAM"};

// phase key instance
TFT_eSPI_Button phaseKey[4];
char* phaseKeyLabel[6] = {"YOLLOWING", "LAMINA", "COLOR FIXING", "STEAM", "START", "HOME"};
uint16_t phaseKeyColor[6] = {tft.color565(255, 0, 255), tft.color565(255, 0, 255),
                            tft.color565(255, 0, 255), tft.color565(255, 0, 255),
                            CUSTOM_PURPLE, CUSTOM_LIGHT_RED};


// Button Touch variables
uint16_t t_x = 0, t_y = 0;


// Field color controller variable
byte selected_manual_field = 1;


// manual field strings
String manual_dry_temp_str = "";
String manual_wet_temp_str = "";
String manual_time_str = "";

// Phase time value string
String phase_time_value_str = "";

// home page time strings
String home_cur_time_str = "00:00:00";
String home_limit_time_str = "00:00:00";

char* phase_label[5] = {"NO PHASE", "YELLOWING", "LAMINA", "COLOR FIX.", "STEAM"};
char* mode_label[4] = {"NO MODE", "AUTO", "AUTO PHASE", "MANUAL"};


// Phase parameters
const byte phase_temp[4][14][2] = {{{95, 92}, {96, 93}, {98, 94}, {99, 95}, {100, 96}},
                              {{100, 96}, {102, 96}, {104, 97}, {106, 97}, {108, 98}, {110, 98}, {112, 98}, {114, 99}, {116, 99}, {118, 100}, {120, 100}},
                              {{120, 100}, {122, 100}, {124, 101}, {126, 101}, {128, 102}, {130, 102}, {132, 102}, {134, 103}, {136, 103}, {138, 104}, {140, 104}, {142, 104}, {144, 105}, {145, 105}},
                              {{145, 105}, {147, 106}, {151, 107}, {153, 107}, {155, 108}, {157, 108}, {159, 109}, {161, 109}, {163, 110}, {163, 110}, {165, 110}}};

const byte phase_duration_hour[5] = {0, 4, 10, 12, 10};    //4 10 12 10
const byte phase_duration_min[5] = {0, 0, 0, 0, 0};   //0 0 30 0
const byte phase_duration_sec[5] = {0, 0, 0, 0, 0};


// Phase labels
// char* phase_label[5] = {"NO SELECTED", "YELLOWING", "LAMINA", "COLOR FIX.", "STEAM"};


// Keypad variables
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {33, 32, 16, 17}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {14, 27, 26, 25}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);



// Temp sensor variables
#define temp1 12
#define temp2 13
// Onewire objects 
OneWire oneWire1(temp1);
OneWire oneWire2(temp2);

// DallasTemperature objects
DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);

float tempF[2] = {0, 0};

// Temp print string
String dry_temp_str = " --";
String wet_temp_str = " --";

String set_dry_temp_str = " --";
String set_wet_temp_str = " --";



// Controller variables
uint8_t pre_interface = 0;
uint8_t selected_interface = 1;
uint8_t selected_phase_phase = 0;

// main controller variables
uint64_t cur_time = 0;
uint64_t pre_time = 0;
uint64_t pre_eeprom_time = 0;

uint8_t second = 0;
uint8_t minute = 0;
uint8_t hour = 0;

uint8_t limit_sec = 0;
uint8_t limit_min = 0;
uint8_t limit_hour = 0;

int limit_db = 0;
int limit_wb = 0;

byte selected_mode = 0;
byte selected_phase = 0;
byte selected_pre_phase = -1;

bool cur_time_flag = false;

byte temp_phase_select = 0;




//EEPROM address
int selected_mode1 = 0;
int selected_phase1 = 20;
// int selected_pre_phase1 = 40;
int hour1 = 60;
int min1 = 80;
int sec1 = 100;
int final_limit_hour1 = 120;
int final_limit_min1 = 140;
int final_limit_sec1 = 160;
int flag1 = 180;

int selected_mode2 = 200;
int selected_phase2 = 220;
// int selected_pre_phase2 = 240;
int hour2 = 260;
int min2 = 280;
int sec2 = 300;
int final_limit_hour2 = 320;
int final_limit_min2 = 340;
int final_limit_sec2 = 360;
int flag2 = 380;

#define EEPROM 0x50


// EEPROM WRITE
void writeIntToEEPROM(byte deviceAddress, byte memAddress, int data) {
  Wire.beginTransmission(deviceAddress);
  Wire.write(memAddress);           // Memory address
  Wire.write((data >> 24) & 0xFF);  // Write MSB (Most Significant Byte)
  Wire.write((data >> 16) & 0xFF);  // Write next byte
  Wire.write((data >> 8) & 0xFF);   // Write next byte
  Wire.write(data & 0xFF);          // Write LSB (Least Significant Byte)
  Wire.endTransmission();
  delay(10); // Ensure EEPROM write cycle is complete
}


// Function to read an integer from EEPROM
int readIntFromEEPROM(byte deviceAddress, byte memAddress) {
  int value = 0;
  
  Wire.beginTransmission(deviceAddress);
  Wire.write(memAddress);           // Memory address
  Wire.endTransmission();
  
  Wire.requestFrom(deviceAddress, (uint8_t)4);  // Request 4 bytes (size of int)
  if (Wire.available() == 4) {
    value = (Wire.read() << 24);    // Read MSB
    value |= (Wire.read() << 16);   // Read next byte
    value |= (Wire.read() << 8);    // Read next byte
    value |= Wire.read();           // Read LSB
  }
  return value;
}

// check function
void check_eeprom() {
    if(readIntFromEEPROM(EEPROM, flag1) == 200){
        selected_mode = readIntFromEEPROM(EEPROM, selected_mode1);
        selected_phase = readIntFromEEPROM(EEPROM, selected_phase1);
        // selected_pre_phase = readIntFromEEPROM(EEPROM, selected_pre_phase1);
        hour = readIntFromEEPROM(EEPROM, hour1);
        minute = readIntFromEEPROM(EEPROM, min1);
        second = readIntFromEEPROM(EEPROM, sec1);
        limit_hour = readIntFromEEPROM(EEPROM, final_limit_hour1);
        limit_min = readIntFromEEPROM(EEPROM, final_limit_min1);
        limit_sec = readIntFromEEPROM(EEPROM, final_limit_sec1);
        // limit_time_ms = time_to_ms(final_limit_hour, final_limit_min, final_limit_sec);
        cur_time_flag = true;
    }
    else if(readIntFromEEPROM(EEPROM, flag2) == 200){
        selected_mode = readIntFromEEPROM(EEPROM, selected_mode2);
        selected_phase = readIntFromEEPROM(EEPROM, selected_phase2);
        // selected_pre_phase = readIntFromEEPROM(EEPROM, selected_pre_phase2);
        hour = readIntFromEEPROM(EEPROM, hour2);
        minute = readIntFromEEPROM(EEPROM, min2);
        second = readIntFromEEPROM(EEPROM, sec2);
        limit_hour = readIntFromEEPROM(EEPROM, final_limit_hour2);
        limit_min = readIntFromEEPROM(EEPROM, final_limit_min2);
        limit_sec = readIntFromEEPROM(EEPROM, final_limit_sec2);
        // limit_time_ms = time_to_ms(final_limit_hour, final_limit_min, final_limit_sec);
        cur_time_flag = true;
    }
    return;
}

bool flip = true;




// Touch Calibration Function
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}


// Time string converter
String time_to_str(uint8_t h, uint8_t m, uint8_t s) {
    String x = "";
    if(h <= 9){
        x += '0';
    }
    x += String(h) + ':';

    if(m <= 9) {
        x += '0';
    }
    x += String(m) + ':';

    if(s <= 9){
        x += '0';
    }
    x += String(s);

    return x;
}



// Home touch function
void home_touch() {
    bool pressed = tft.getTouch(&t_x, &t_y);

    // MANUAL KEY
    if(pressed && homeKey[0].contains(t_x, t_y)){
        Serial.println("Manual key pressed");
        selected_interface = 2;
        homeKey[0].press(true);
    }
    else{
        homeKey[0].press(false);
    }

    // AUTO KEY
    if(pressed && homeKey[1].contains(t_x, t_y)){
        Serial.println("Manual key pressed");
        homeKey[1].press(true);
        hour = 0;
        minute = 0;
        second = 0;
        selected_mode = 1;
        selected_phase = 1;
        limit_sec = phase_duration_sec[selected_phase];
        limit_min = phase_duration_min[selected_phase];
        limit_hour = phase_duration_hour[selected_phase];

        home_limit_time_str = time_to_str(limit_hour, limit_min, limit_sec);
        cur_time_flag = true;

        limit_db = phase_temp[selected_phase-1][hour][0];
        limit_wb = phase_temp[selected_phase-1][hour][1];

        set_dry_temp_str = String(limit_db);
        set_wet_temp_str = String(limit_wb);

        selected_interface = 1;
        pre_interface = -1;

        writeIntToEEPROM(EEPROM, flag1, 0);
        writeIntToEEPROM(EEPROM, flag2, 0);
        delay(10);

        // home_screen();
        // print_dbset_temp();
        // print_wbset_temp();

        // delay(300);
    }
    else{
        homeKey[1].press(false);
    }

    // PHASE KEY
    if(pressed && homeKey[2].contains(t_x, t_y)){
        Serial.println("Manual key pressed");
        selected_interface = 3;
        homeKey[2].press(true);
    }
    else{
        homeKey[2].press(false);
    }

    // RESET KEY
    if(pressed && homeKey[3].contains(t_x, t_y)){
        Serial.println("Manual key pressed");
        homeKey[3].press(true);
        writeIntToEEPROM(EEPROM, flag1, 0);
        writeIntToEEPROM(EEPROM, flag2, 0);
        delay(10);
        ESP.restart();
    }
    else{
        homeKey[3].press(false);
    }

    if(homeKey[0].justReleased()) homeKey[0].drawButton();
    if(homeKey[0].justPressed()) homeKey[0].drawButton(true);
    if(homeKey[1].justReleased()) homeKey[1].drawButton();
    if(homeKey[1].justPressed()) homeKey[1].drawButton(true);
    if(homeKey[2].justReleased()) homeKey[2].drawButton();
    if(homeKey[2].justPressed()) homeKey[2].drawButton(true);
    if(homeKey[3].justReleased()) homeKey[3].drawButton();
    if(homeKey[3].justPressed()) homeKey[3].drawButton(true);
}

// manual touch control function
void manual_touch() {
    bool pressed = tft.getTouch(&t_x, &t_y);

    // HOME KEY
    if(pressed && manualKey[0].contains(t_x, t_y)){
        Serial.println("home key pressed");
        selected_interface = 1;
        manualKey[0].press(true);
    }
    else{
        manualKey[0].press(false);
    }

    // SAVE KEY
    if(pressed && manualKey[1].contains(t_x, t_y)){
        Serial.println("save key pressed");
        manualKey[1].press(true);
    }
    else{
        manualKey[1].press(false);
    }

    // START KEY
    if(pressed && manualKey[2].contains(t_x, t_y)){
        Serial.println("start key pressed");
        manualKey[2].press(true);

        int t_hour = 0;
        int t_min = 0;
        int t_sec = 0;
        int t_db = 0;
        int t_wb = 0;
        // working code
        for(int i=0; i<manual_dry_temp_str.length(); i++){
            int x = manual_dry_temp_str[i] - '0';
            // if(i != manual_dry_temp_str.length()-1){
            t_db = t_db * 10;
            // }
            t_db += x;
        }
        for(int i=0; i<manual_wet_temp_str.length(); i++){
            int x = manual_wet_temp_str[i] - '0';
            // if(i != manual_wet_temp_str.length()-1){
            t_wb = t_wb*10;
            // }
            t_wb += x;
        }
        int ct = 0;
        for(int i=0; i<manual_time_str.length(); i++){
            if(manual_time_str[i] == ':'){
                ct++;
                continue;
            } 
            else {
                if(ct == 0)
                    t_hour *= 10;
                else if(ct == 1){
                    t_min *= 10;
                }
                else if(ct == 2){
                    t_sec *= 10;
                }
            }
            int x = manual_time_str[i] - '0';
            if(ct == 0){
                t_hour += x; 
            }
            else if(ct == 1){
                t_min += x;
            }
            else if(ct == 2){
                t_sec += x;
            }
        }
        if(t_hour > 60) t_hour = 0; 
        if(t_min >= 60) t_min = 0; 
        if(t_sec >= 60) t_sec = 0; 

        selected_interface = 1;
        pre_interface = -1;
        limit_db = t_db;
        limit_wb = t_wb;
        limit_hour = t_hour;
        limit_min = t_min;
        limit_sec = t_sec;
        hour = 0;
        minute = 0;
        second = 0;
        selected_mode = 3;
        cur_time_flag = true;
        selected_phase = 0;
        set_dry_temp_str = String(limit_db);
        set_wet_temp_str = String(limit_wb);

        writeIntToEEPROM(EEPROM, flag1, 0);
        writeIntToEEPROM(EEPROM, flag2, 0);
        delay(10);
    }
    else{
        manualKey[2].press(false);
    }

    // RESET KEY
    if(pressed && manualKey[3].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        manualKey[3].press(true);

        // reseting values
        pre_interface = 100;
        selected_interface = 2;
        selected_manual_field = 1;
        manual_dry_temp_str = "";
        manual_wet_temp_str = "";
        manual_time_str = "";
        delay(300);
        tft.fillScreen(TFT_BLACK);
        field_change_update();
    }
    else{
        manualKey[3].press(false);
    }

    if(manualKey[0].justReleased()) manualKey[0].drawButton();
    if(manualKey[0].justPressed()) manualKey[0].drawButton(true);
    if(manualKey[1].justReleased()) manualKey[1].drawButton();
    if(manualKey[1].justPressed()) manualKey[1].drawButton(true);
    if(manualKey[2].justReleased()) manualKey[2].drawButton();
    if(manualKey[2].justPressed()) manualKey[2].drawButton(true);
    if(manualKey[3].justReleased()) manualKey[3].drawButton();
    if(manualKey[3].justPressed()) manualKey[3].drawButton(true);



    // Keypad key
    // 1
    if(pressed && manualKeypadKey[0].contains(t_x, t_y)){
        Serial.println("1 key pressed");
        manualKeypadKey[0].press(true);

        // work here
        update_field('1');
        delay(100);
    }
    else{
        manualKeypadKey[0].press(false);
    }

    // 2
    if(pressed && manualKeypadKey[1].contains(t_x, t_y)){
        Serial.println("save key pressed");
        manualKeypadKey[1].press(true);

        // work here
        update_field('2');
        delay(100);
    }
    else{
        manualKeypadKey[1].press(false);
    }

    // 3
    if(pressed && manualKeypadKey[2].contains(t_x, t_y)){
        Serial.println("start key pressed");
        manualKeypadKey[2].press(true);

        // work here
        update_field('3');
        delay(100);
    }
    else{
        manualKeypadKey[2].press(false);
    }

    // U
    if(pressed && manualKeypadKey[3].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        manualKeypadKey[3].press(true);

        // work here
        selected_manual_field--;
        if(selected_manual_field < 1) selected_manual_field = 3;
        field_change_update();
        delay(100);
    }
    else{
        manualKeypadKey[3].press(false);
    }

    // 2nd row keypad
    // 4
    if(pressed && manualKeypadKey[4].contains(t_x, t_y)){
        Serial.println("1 key pressed");
        manualKeypadKey[4].press(true);

        // work here
        update_field('4');
        delay(100);
    }
    else{
        manualKeypadKey[4].press(false);
    }

    // 5
    if(pressed && manualKeypadKey[5].contains(t_x, t_y)){
        Serial.println("save key pressed");
        manualKeypadKey[5].press(true);

        // work here
        update_field('5');
        delay(100);
    }
    else{
        manualKeypadKey[5].press(false);
    }

    // 6
    if(pressed && manualKeypadKey[6].contains(t_x, t_y)){
        Serial.println("start key pressed");
        manualKeypadKey[6].press(true);

        // work here
        update_field('6');
        delay(100);
    }
    else{
        manualKeypadKey[6].press(false);
    }

    // D
    if(pressed && manualKeypadKey[7].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        manualKeypadKey[7].press(true);

        // work here
        selected_manual_field++;
        if(selected_manual_field > 3) selected_manual_field = 1;
        field_change_update();
        delay(100);
    }
    else{
        manualKeypadKey[7].press(false);
    }


    //3rd keypad row
    // 7
    if(pressed && manualKeypadKey[8].contains(t_x, t_y)){
        Serial.println("1 key pressed");
        manualKeypadKey[8].press(true);

        // work here
        update_field('7');
        delay(100);
    }
    else{
        manualKeypadKey[8].press(false);
    }

    // 8
    if(pressed && manualKeypadKey[9].contains(t_x, t_y)){
        Serial.println("save key pressed");
        manualKeypadKey[9].press(true);

        // work here
        update_field('8');
        delay(100);
    }
    else{
        manualKeypadKey[9].press(false);
    }

    // 9
    if(pressed && manualKeypadKey[10].contains(t_x, t_y)){
        Serial.println("start key pressed");
        manualKeypadKey[10].press(true);

        // work here
        update_field('9');
        delay(100);
    }
    else{
        manualKeypadKey[10].press(false);
    }

    // <- backspace
    if(pressed && manualKeypadKey[11].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        manualKeypadKey[11].press(true);

        // work here
        if(selected_manual_field == 1){
            if(manual_dry_temp_str.length() > 0){
                // manual_dry_temp_str[manual_dry_temp_str.length()-1] = '\0';
                // manual_dry_temp_str.setCharAt(manual_dry_temp_str.length() - 1, '\0');
                manual_dry_temp_str = manual_dry_temp_str.substring(0, manual_dry_temp_str.length() - 1);
            }
        }
        if(selected_manual_field == 2){
            if(manual_wet_temp_str.length() > 0){
                // manual_wet_temp_str[manual_wet_temp_str.length()-1] = '\0';
                // manual_wet_temp_str.setCharAt(manual_wet_temp_str.length() - 1, '\0');
                manual_wet_temp_str = manual_wet_temp_str.substring(0, manual_wet_temp_str.length() - 1);
            }
        }
        if(selected_manual_field == 3){
            if(manual_dry_temp_str.length() > 0){
                // manual_time_str[manual_time_str.length()-1] = '\0';
                // manual_time_str.setCharAt(manual_time_str.length() - 1, '\0');
                manual_time_str = manual_time_str.substring(0, manual_time_str.length() - 1);
            }
        }
        field_change_update();
        delay(100);
    }
    else{
        manualKeypadKey[11].press(false);
    }


    // 4th keypad row
    // .
    if(pressed && manualKeypadKey[12].contains(t_x, t_y)){
        Serial.println("1 key pressed");
        manualKeypadKey[12].press(true);

        // work here
        update_field('.');
        delay(100);
    }
    else{
        manualKeypadKey[12].press(false);
    }

    // 0
    if(pressed && manualKeypadKey[13].contains(t_x, t_y)){
        Serial.println("save key pressed");
        manualKeypadKey[13].press(true);

        // work here
        update_field('0');
        delay(100);
    }
    else{
        manualKeypadKey[13].press(false);
    }

    // :
    if(pressed && manualKeypadKey[14].contains(t_x, t_y)){
        Serial.println("start key pressed");
        manualKeypadKey[14].press(true);

        // work here
        update_field(':');
        delay(100);
    }
    else{
        manualKeypadKey[14].press(false);
    }

    // ok
    if(pressed && manualKeypadKey[15].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        manualKeypadKey[15].press(true);

        // work here
        delay(100);
    }
    else{
        manualKeypadKey[15].press(false);
    }

    for(int i=0;i<16;i++){
        if(manualKeypadKey[i].justReleased()) manualKeypadKey[i].drawButton();
        if(manualKeypadKey[i].justPressed()) manualKeypadKey[i].drawButton(true);
    }    
}

// manual field update
void update_field(char ch) {
    switch(selected_manual_field){
        case 1:
            if(manual_dry_temp_str.length() < 3 && (ch != '.' && ch != ':'))
                manual_dry_temp_str += ch;
            print_manual_dry_value();
            break;
        case 2:
            if(manual_wet_temp_str.length() < 3 && (ch != '.' && ch != ':'))
                manual_wet_temp_str += ch;
            print_manual_wet_value();
            break;
        case 3:
            if(manual_time_str.length() < 8 && (ch != '.'))
                manual_time_str += ch;
            print_manual_time_value();
            break;
    }
}

// field change update
void field_change_update() {
    print_manual_dry_value();
    print_manual_wet_value();
    print_manual_time_value();
}


// phase touch control
void phase_touch() {
    bool pressed = tft.getTouch(&t_x, &t_y);

    // YELLOWING KEY
    if(pressed && phaseKey[0].contains(t_x, t_y)){
        Serial.println("home key pressed");
        phaseKey[0].press(true);

        // selected_phase_phase = 1;
        temp_phase_select = 1;
        update_phase_field();
        update_phase_time_field();
        delay(100);
    }
    else{
        phaseKey[0].press(false);
    }

    // LAMINA KEY
    if(pressed && phaseKey[1].contains(t_x, t_y)){
        Serial.println("save key pressed");
        phaseKey[1].press(true);

        // selected_phase_phase = 2;
        temp_phase_select = 2;
        update_phase_field();
        update_phase_time_field();
        delay(100);
    }
    else{
        phaseKey[1].press(false);
    }

    // COLOR FIXING KEY
    if(pressed && phaseKey[2].contains(t_x, t_y)){
        Serial.println("start key pressed");
        // selected_interface = 3;
        phaseKey[2].press(true);

        // selected_phase_phase = 3;
        temp_phase_select = 3;
        update_phase_field();
        update_phase_time_field();
        delay(100);
    }
    else{
        phaseKey[2].press(false);
    }

    // STEAM KEY
    if(pressed && phaseKey[3].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        phaseKey[3].press(true);

        // selected_phase_phase = 4;
        temp_phase_select = 4;
        update_phase_field();
        update_phase_time_field();
        delay(100);
    }
    else{
        phaseKey[3].press(false);
    }

    // START KEY
    if(pressed && phaseKey[4].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        phaseKey[4].press(true);
        selected_phase = temp_phase_select;
        selected_mode = 2;
        pre_interface = -1;
        selected_interface = 1;
        temp_phase_select = 0;
        cur_time_flag = true;
        hour = 0;
        minute = 0;
        second = 0;
        limit_hour = phase_duration_hour[selected_phase];
        limit_min = phase_duration_min[selected_phase];
        limit_sec = phase_duration_sec[selected_phase];

        writeIntToEEPROM(EEPROM, flag1, 0);
        writeIntToEEPROM(EEPROM, flag2, 0);
        delay(10);
    }
    else{
        phaseKey[4].press(false);
    }

    // HOME KEY
    if(pressed && phaseKey[5].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        selected_interface = 1;
        temp_phase_select = 0;
        phaseKey[5].press(true);
    }
    else{
        phaseKey[5].press(false);
    }

    if(phaseKey[0].justReleased()) phaseKey[0].drawButton();
    if(phaseKey[0].justPressed()) phaseKey[0].drawButton(true);

    if(phaseKey[1].justReleased()) phaseKey[1].drawButton();
    if(phaseKey[1].justPressed()) phaseKey[1].drawButton(true);

    if(phaseKey[2].justReleased()) phaseKey[2].drawButton();
    if(phaseKey[2].justPressed()) phaseKey[2].drawButton(true);

    if(phaseKey[3].justReleased()) phaseKey[3].drawButton();
    if(phaseKey[3].justPressed()) phaseKey[3].drawButton(true);

    if(phaseKey[4].justReleased()) phaseKey[4].drawButton();
    if(phaseKey[4].justPressed()) phaseKey[4].drawButton(true);

    if(phaseKey[5].justReleased()) phaseKey[5].drawButton();
    if(phaseKey[5].justPressed()) phaseKey[5].drawButton(true);
}

// Update phase field
void update_phase_field() {
    tft.fillRect(offset_col + 80, offset_row, 160, textHeight*2+5, TFT_BLACK);
    print_phase_select_value();
}

// Update time field
void update_phase_time_field() {
    phase_time_value_str = "";
    String x = "";
    if(phase_duration_hour[temp_phase_select] <= 9)
        phase_time_value_str += '0';
    x = String(phase_duration_hour[temp_phase_select]);
    phase_time_value_str += x + ":";

    if(phase_duration_min[temp_phase_select] <= 9)
        phase_time_value_str += '0';
    x = String(phase_duration_min[temp_phase_select]);
    phase_time_value_str += x + ":";

    if(phase_duration_sec[temp_phase_select] <= 9)
        phase_time_value_str += '0';
    x = String(phase_duration_sec[temp_phase_select]);
    phase_time_value_str += x;
    print_phase_manual_time_value();
}



// Phase Screen Function
void phase_screen() {
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);

    print_phase_select_label();
    print_phase_select_value();

    print_phase_manual_time();
    print_phase_manual_time_value();

    draw_phase_key();
}

// First row
// Phase label print
void print_phase_select_label() {
    tft.setFreeFont(FSB9);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("PHASE :", offset_col + 5, offset_row -5 + 7, GFXFF);
}

void print_phase_select_value() {
    tft.setFreeFont(FSB9);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(phase_label[temp_phase_select], offset_col + 80, offset_row -5 + 7, GFXFF);
}


// Second row
// Time label print
void print_phase_manual_time() {
    tft.setFreeFont(FSB9);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("TIME   :", offset_col + 5, offset_row + 7 + textHeight, GFXFF);
}

void print_phase_manual_time_value() {
    tft.setFreeFont(FSB9);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(phase_time_value_str, offset_col + 80, offset_row + 7 + textHeight, GFXFF);
}

// Phase button draw
void draw_phase_key() {
    int home_key_col_offset = 115;
    int home_key_row_offset = 100;
    int home_key_width = 180;
    int home_key_height = 26;
    int home_key_space_x = 6;
    int home_key_space_y = 12;
    for (uint8_t row = 0; row < 6; row++) {
        for (uint8_t col = 0; col < 1; col++) {
            uint8_t b = col + row * 1;

            tft.setFreeFont(FSB9);

            phaseKey[b].initButton(&tft, home_key_col_offset + col * (home_key_width + home_key_space_x),
                                home_key_row_offset + row * (home_key_height + home_key_space_y),
                                home_key_width, home_key_height, TFT_WHITE, phaseKeyColor[b], TFT_WHITE,
                                phaseKeyLabel[b], 1);
            phaseKey[b].drawButton();
        }
    }
}




// Manual Mode Screen function
void manual_screen() {
    tft.fillScreen(TFT_BLACK);

    tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);

    print_manual_dry_label();
    print_manual_dry_value();

    print_manual_wet_label();
    print_manual_wet_value();

    print_manual_time_label();
    print_manual_time_value();

    draw_manual_keypad();

    draw_manual_key();

    draw_manual_phase_key();
}


// Manual first row
// Manual Dry label print
void print_manual_dry_label() {
    tft.drawRect(offset_col+2, offset_row - 5, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("DRY", offset_col+5, offset_row -5 +7, GFXFF);
}

// Manual Dry Value print
void print_manual_dry_value() {
    tft.setFreeFont(FSB9);                 // Select the font
    if(selected_manual_field == 1) {
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row - 5, textWidth + 111, textHeight, TFT_GREEN);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row - 4, textWidth + 109, textHeight-2, TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
    }
    else {
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row - 5, textWidth + 111, textHeight, TFT_WHITE);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row - 4, textWidth + 109, textHeight-2, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString(manual_dry_temp_str, offset_col + textWidth + 3 + 15, offset_row -5 +7, GFXFF);
}


// Manual second row
// Manual Wet label print
void print_manual_wet_label() {
    tft.drawRect(offset_col+2, offset_row - 5 + textHeight + 5, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("WET", offset_col+5, offset_row - 5 + textHeight + 5 + 7, GFXFF);
}

// Manual Wet value print
void print_manual_wet_value() {
    tft.setFreeFont(FSB9);                 // Select the font
    if(selected_manual_field == 2) {
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row - 5 + textHeight + 5, textWidth + 111, textHeight, TFT_GREEN);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row - 5 + textHeight + 6, textWidth + 109, textHeight-2, TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
    }
    else {
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row - 5 + textHeight + 5, textWidth + 111, textHeight, TFT_WHITE);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row - 5 + textHeight + 6, textWidth + 109, textHeight-2, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString(manual_wet_temp_str, offset_col + textWidth + 3 + 15, offset_row - 5 + textHeight + 5 + 7, GFXFF);
}


// Manual third row
// Manual Wet label print
void print_manual_time_label() {
    tft.drawRect(offset_col+2, offset_row -5 + textHeight*2 + 10, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("TIME", offset_col+5, offset_row -5 + textHeight*2 + 10 + 7, GFXFF);
}

// Manual Time value print
void print_manual_time_value() {
    tft.setFreeFont(FSB9);                 // Select the font
    if(selected_manual_field == 3){
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row -5 + textHeight*2 + 10, textWidth + 111, textHeight, TFT_GREEN);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row -5 + textHeight*2 + 11, textWidth + 109, textHeight-2, TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
    }
    else{
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row -5 + textHeight*2 + 10, textWidth + 111, textHeight, TFT_WHITE);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row -5 + textHeight*2 + 11, textWidth + 109, textHeight-2, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString(manual_time_str, offset_col + textWidth + 3 + 15, offset_row -5 + textHeight*2 + 10 + 7, GFXFF);
}


// Draw manual keypad
void draw_manual_keypad() {
    int home_key_col_offset = 18;
    int home_key_row_offset = 130;
    int home_key_width = 30;
    int home_key_height = 30;
    int home_key_space_x = 6;
    int home_key_space_y = 6;
    for (uint8_t row = 0; row < 4; row++) {
        for (uint8_t col = 0; col < 4; col++) {
            uint8_t b = col + row * 4;

            // if (b < 3) tft.setFreeFont(LABEL1_FONT);
            // else tft.setFreeFont(LABEL2_FONT);

            tft.setFreeFont(FSB9);

            manualKeypadKey[b].initButton(&tft, home_key_col_offset + col * (home_key_width + home_key_space_x),
                                home_key_row_offset + row * (home_key_height + home_key_space_y),
                                home_key_width, home_key_height, TFT_WHITE, TFT_WHITE, TFT_BLACK,
                                manualKeypadKeyLabel[b], 1);

            // key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
            //                     KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
            //                     KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
            //                     keyLabel[b], KEY_TEXTSIZE);
            manualKeypadKey[b].drawButton();
        }
    }
}

// Draw manual key
void draw_manual_key() {
    int home_key_col_offset = 62;
    int home_key_row_offset = 270;
    int home_key_width = 100;
    int home_key_height = 23;
    int home_key_space_x = 10;
    int home_key_space_y = 12;
    for (uint8_t row = 0; row < 2; row++) {
        for (uint8_t col = 0; col < 2; col++) {
            uint8_t b = col + row * 2;

            // if (b < 3) tft.setFreeFont(LABEL1_FONT);
            // else tft.setFreeFont(LABEL2_FONT);

            tft.setFreeFont(FSB9);

            manualKey[b].initButton(&tft, home_key_col_offset + col * (home_key_width + home_key_space_x),
                                home_key_row_offset + row * (home_key_height + home_key_space_y),
                                home_key_width, home_key_height, TFT_WHITE, homeKeyColor[b], TFT_BLACK,
                                manualKeyLabel[b], 1);

            // key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
            //                     KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
            //                     KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
            //                     keyLabel[b], KEY_TEXTSIZE);
            manualKey[b].drawButton();
        }
  }
}

// Draw manual phase key
void draw_manual_phase_key() {
    int home_key_col_offset = 192;
    int home_key_row_offset = 130;
    int home_key_width = 91;
    int home_key_height = 26;
    int home_key_space_x = 6;
    int home_key_space_y = 8;
    for (uint8_t row = 0; row < 4; row++) {
        for (uint8_t col = 0; col < 1; col++) {
            uint8_t b = col + row * 1;

            // if (b < 3) tft.setFreeFont(LABEL1_FONT);
            // else tft.setFreeFont(LABEL2_FONT);

            tft.setFreeFont(FSB9);

            manualPhaseKey[b].initButton(&tft, home_key_col_offset + col * (home_key_width + home_key_space_x),
                                home_key_row_offset + row * (home_key_height + home_key_space_y),
                                home_key_width, home_key_height, TFT_WHITE, tft.color565(255, 0, 255), TFT_WHITE,
                                manualPhaseKeyLabel[b], 1);

            // key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
            //                     KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
            //                     KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
            //                     keyLabel[b], KEY_TEXTSIZE);
            manualPhaseKey[b].drawButton();
        }
    }
}





// Home Screen Function
void home_screen() {
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);

    print_db_label();
    print_db_temp();
    print_wb_label();
    print_wb_temp();

    print_dbset_label();
    print_dbset_temp();
    print_wbset_label();
    print_wbset_temp();

    print_curtime_label();
    print_curtime();

    print_limittime_label();
    print_limittime();

    print_phase_label();
    print_phase();

    print_mode_label();
    print_mode();

    draw_home_keypad();
}


// First Row
// Print DB label
void print_db_label() {
    tft.drawRect(offset_col, offset_row, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("DB", offset_col+3, offset_row+7, GFXFF);
}

// Print DB temp
void print_db_temp() {
    tft.drawRect(offset_col + textWidth + 1, offset_row, textWidth, textHeight, TFT_WHITE);
    tft.fillRect(offset_col + textWidth + 2, offset_row + 1, textWidth - 2, textHeight - 2, TFT_BLACK);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(dry_temp_str, offset_col + textWidth + 3+3, offset_row+7, GFXFF);
}

// Print WB label
void print_wb_label() {
    tft.drawRect(offset_col + textWidth * 2 + 2, offset_row, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("WB", offset_col + textWidth * 2 + 3+3+3, offset_row+7, GFXFF);
}

// Print WB temp
void print_wb_temp() {
    tft.drawRect(offset_col + textWidth * 3 + 3, offset_row, textWidth, textHeight, TFT_WHITE);
    tft.fillRect(offset_col + textWidth * 3 + 4, offset_row + 1, textWidth - 2, textHeight - 2, TFT_BLACK);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(wet_temp_str, offset_col + textWidth * 3 + 3+3+3+3, offset_row+7, GFXFF);
}



// Second Row
// Print DB SET label
void print_dbset_label() {
    tft.drawRect(offset_col, offset_row + textHeight + 5, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("DBSET", offset_col+3, offset_row + textHeight + 5 + 7, GFXFF);
}

// Print DB SET temp
void print_dbset_temp() {
    tft.drawRect(offset_col + textWidth + 1, offset_row + textHeight + 5, textWidth, textHeight, TFT_WHITE);
    tft.fillRect(offset_col + textWidth + 2, offset_row + textHeight + 6, textWidth-2, textHeight-2, TFT_BLACK);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(set_dry_temp_str, offset_col + textWidth + 3+3, offset_row + textHeight + 5 + 7, GFXFF);
}

// Print WB SET label
void print_wbset_label() {
    tft.drawRect(offset_col + textWidth * 2 + 2, offset_row + textHeight + 5, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("WBSET", offset_col + textWidth * 2 + 3+2, offset_row + textHeight + 5 + 7, GFXFF);
}

// Print WB SET temp
void print_wbset_temp() {
    tft.drawRect(offset_col + textWidth * 3 + 3, offset_row + textHeight + 5, textWidth, textHeight, TFT_WHITE);
    tft.drawRect(offset_col + textWidth * 3 + 4, offset_row + textHeight + 6, textWidth-2, textHeight-2, TFT_BLACK);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(set_wet_temp_str, offset_col + textWidth * 3 + 3+3+3+3, offset_row + textHeight + 5 + 7, GFXFF);
}



// Third Row
// Print Current Time label
void print_curtime_label() {
    tft.drawRect(offset_col, offset_row + 70, textWidth + 60, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("CUR. TIME", offset_col+3, offset_row + 7 + 70, GFXFF);
}

// Print Current Time
void print_curtime() {
    home_cur_time_str = time_to_str(hour, minute, second);
    tft.fillRect(offset_col + textWidth*2 + 3, offset_row + 70, textWidth + 60, textHeight, TFT_BLACK);
    // tft.fillRect(offset_col + textWidth*2 + 4, offset_row + 71, textWidth + 58, textHeight - 2, TFT_BLACK);
    tft.setFreeFont(FSB12);                 // Select the font
    // tft.print();
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(home_cur_time_str, offset_col + textWidth*2 + 3 + 3, offset_row + 5 + 70, GFXFF);
}



// Fourth Row
// Print Limit Time label
void print_limittime_label() {
    tft.drawRect(offset_col, offset_row + 70 + textHeight + 5, textWidth + 60, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("LIMIT TIME", offset_col+3, offset_row + 7 + 70 + textHeight + 5, GFXFF);
}

void print_limittime() {
    home_limit_time_str = time_to_str(limit_hour, limit_min, limit_sec);
    tft.fillRect(offset_col + textWidth*2 + 3 + 3, offset_row + 70 + textHeight + 5, textWidth + 60, textHeight, TFT_BLACK);
    // tft.fillRect(offset_col + 1, offset_row + 70 + textHeight + 5, textWidth + 60, textHeight, TFT_WHITE);
    tft.setFreeFont(FSB12);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(home_limit_time_str, offset_col + textWidth*2 + 3 + 3, offset_row + 5 + 70 + textHeight + 5, GFXFF);
}



// Fifth Row
// Print Phase label
void print_phase_label() {
    tft.drawRect(offset_col, offset_row + 70 + textHeight*2 + 10, textWidth + 60, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("PHASE", offset_col+3, offset_row + 7 + 70 + textHeight*2 + 10, GFXFF);
}

void print_phase() {
    tft.fillRect(offset_col + textWidth*2 + 3+3, offset_row + 70 + textHeight*2 + 10, textWidth + 60, textHeight, TFT_BLACK);
    // tft.fillRect(offset_col, offset_row + 70 + textHeight*2 + 10, offset_row + 8 + 70 + textHeight*2 + 10, textWidth + 60, textHeight, TFT_RED);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(phase_label[selected_phase], offset_col + textWidth*2 + 3+3, offset_row + 7 + 70 + textHeight*2 + 10, GFXFF);
}



// Sixth Row
// Print Mode label
void print_mode_label() {
    tft.drawRect(offset_col, offset_row + 70 + textHeight*3 + 15, textWidth + 60, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("MODE", offset_col+3, offset_row + 7 + 70 + textHeight*3 + 15, GFXFF);
}

void print_mode() {
    tft.fillRect(offset_col + textWidth*2 + 3 + 3, offset_row + 70 + textHeight*3 + 15, textWidth + 60, textHeight, TFT_BLACK);
    // tft.fillRect(offset_col + textWidth*2 + 3 + 3, offset_row + 7 + 70 + textHeight*3 + 15, textWidth, textHeight, TFT_RED);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(mode_label[selected_mode], offset_col + textWidth*2 + 3 + 3, offset_row + 7 + 70 + textHeight*3 + 15, GFXFF);
}



// Draw home keypad
void draw_home_keypad() {
    int home_key_col_offset = 62;
    int home_key_row_offset = 250;
    int home_key_width = 100;
    int home_key_height = 23;
    int home_key_space_x = 10;
    int home_key_space_y = 12;
    for (uint8_t row = 0; row < 2; row++) {
        for (uint8_t col = 0; col < 2; col++) {
            uint8_t b = col + row * 2;

            // if (b < 3) tft.setFreeFont(LABEL1_FONT);
            // else tft.setFreeFont(LABEL2_FONT);

            tft.setFreeFont(FSB9);

            homeKey[b].initButton(&tft, home_key_col_offset + col * (home_key_width + home_key_space_x),
                                home_key_row_offset + row * (home_key_height + home_key_space_y),
                                home_key_width, home_key_height, TFT_WHITE, homeKeyColor[b], TFT_BLACK,
                                homeKeyLabel[b], 1);

            // key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
            //                     KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
            //                     KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
            //                     keyLabel[b], KEY_TEXTSIZE);
            homeKey[b].drawButton();
        }
  }
}

// Interface control function
void interface_control() {
    if(pre_interface == selected_interface) return;
    switch(selected_interface){
        case 1:
            home_screen();
            break;
        case 2:
            manual_screen();
            break;
        case 3:
            phase_screen();
            break;
    }
    pre_interface = selected_interface;
}

// Touch control function
void touch_control() {
    switch(selected_interface){
        case 1:
            home_touch();
            break;
        case 2:
            manual_touch();
            break;
        case 3:
            phase_touch();
            break;
    }
}



void setup() {
    Serial.begin(115200);
    Wire.begin();
    tft.init();
    tft.setRotation(0);

    tft.fillScreen(TFT_BLACK);

    keypad.addEventListener(keypadEvent);

    touch_calibrate();

    pinMode(relay1, OUTPUT);
    pinMode(relay2, OUTPUT);
    // home_screen();
    // delay(20000);
    // manual_screen();
    // delay(20000);
    // phase_screen();

    check_eeprom();
}

void loop() {
    if(cur_time_flag && selected_interface == 1 && (selected_mode == 1 || selected_mode == 2)){
        limit_db = phase_temp[selected_phase-1][hour][0];
        limit_wb = phase_temp[selected_phase-1][hour][1];

        set_dry_temp_str = String(limit_db);
        set_wet_temp_str = String(limit_wb);
        // set_wet_temp_str = "lol";
        // print_dbset_temp();
        // print_wbset_temp();
    }
    cur_time = millis();


    if(cur_time - pre_eeprom_time > 60000 && selected_interface == 1 && cur_time_flag){
        pre_eeprom_time = cur_time;
        // Serial.print("Dry temp: "); Serial.println(final_dry_temp);
        // Serial.print("Wet temp: "); Serial.println(final_wet_temp);
        if(flip){
            writeIntToEEPROM(EEPROM, selected_mode1, selected_mode);
            writeIntToEEPROM(EEPROM, selected_phase1, selected_phase);
            // writeIntToEEPROM(EEPROM, selected_pre_phase1, selected_pre_phase);
            writeIntToEEPROM(EEPROM, hour1, hour);
            writeIntToEEPROM(EEPROM, min1, minute);
            writeIntToEEPROM(EEPROM, sec1, second);
            writeIntToEEPROM(EEPROM, final_limit_hour1, limit_hour);
            writeIntToEEPROM(EEPROM, final_limit_min1, limit_min);
            writeIntToEEPROM(EEPROM, final_limit_sec1, limit_sec);
            writeIntToEEPROM(EEPROM, flag1, 200);
            delay(10);
        }
        else{
            writeIntToEEPROM(EEPROM, selected_mode2, selected_mode);
            writeIntToEEPROM(EEPROM, selected_phase2, selected_phase);
            // writeIntToEEPROM(EEPROM, selected_pre_phase2, selected_pre_phase);
            writeIntToEEPROM(EEPROM, hour2, hour);
            writeIntToEEPROM(EEPROM, min2, minute);
            writeIntToEEPROM(EEPROM, sec2, second);
            writeIntToEEPROM(EEPROM, final_limit_hour2, limit_hour);
            writeIntToEEPROM(EEPROM, final_limit_min2, limit_min);
            writeIntToEEPROM(EEPROM, final_limit_sec2, limit_sec);
            writeIntToEEPROM(EEPROM, flag2, 200);
            delay(10);
        }
        flip = !flip;
    }



    interface_control();
    touch_control();
    keypad.getKey();
    // temp_measure();
    temp_control();
    phase_control();
    if(cur_time - pre_time >= 5 && selected_interface == 1 && cur_time_flag){
        pre_time = cur_time;
        second++;
        if(second == 60){
            minute++;
            second = 0;
            print_dbset_temp();
            print_wbset_temp();
            if(minute == 60){
                hour++;
                minute = 0;
            }
        }
        print_curtime();
    }
}


// Temp control
void temp_control() {
    int db = int(tempF[0]);
    int wb = int(tempF[1]);

    if(limit_db > db){
        // db relay on
        digitalWrite(relay1, HIGH);
    } else {
        // db relay off
        digitalWrite(relay1, LOW);
    } 

    if(limit_wb > wb) {
        // wb relay on
        digitalWrite(relay2, HIGH);
    } else {
        // wb relay off
        digitalWrite(relay2, LOW);
    }
}


// Phase control
void phase_control() {
    if(hour >= limit_hour && minute >= limit_min && second >= limit_sec && selected_mode == 3){
        // writeIntToEEPROM(EEPROM, flag1, 0);
        // writeIntToEEPROM(EEPROM, flag2, 0);
        delay(10);
        // warning_section(2);
        hour = 0;
        minute = 0;
        second = 0;
        limit_hour = 0;
        limit_min = 0;
        limit_sec = 0;
        cur_time_flag = false;
        selected_phase = 0;
        selected_pre_phase = 0;
        selected_mode = 0;

        set_dry_temp_str = " --";
        set_wet_temp_str = " --";

        limit_db = 0;
        limit_wb = 0;

        // time_limit_print();
        // phase_print();
        // cur_time_print();
        // mode_print(0);
        pre_interface = -1;
        selected_interface = 1;

        writeIntToEEPROM(EEPROM, flag1, 0);
        writeIntToEEPROM(EEPROM, flag2, 0);
        delay(10);
    }

    else if(hour >= limit_hour && minute >= limit_min && second >= limit_sec && (selected_mode == 1 || selected_mode == 2)){
        selected_phase++;
        if(selected_phase >= 5){
            // writeIntToEEPROM(EEPROM, flag1, 0);
            // writeIntToEEPROM(EEPROM, flag2, 0);
            delay(10);
            // warning_section(2);
            hour = 0;
            minute = 0;
            second = 0;
            limit_hour = 0;
            limit_min = 0;
            limit_sec = 0;
            selected_phase = 0;
            selected_pre_phase = 0;
            selected_mode = 0;
            cur_time_flag = false;

            set_dry_temp_str = " --";
            set_wet_temp_str = " --";

            limit_db = 0;
            limit_wb = 0;

            // time_limit_print();
            // phase_print();
            // cur_time_print();
            // mode_print(0);
            pre_interface = -1;
            selected_interface = 1;

            writeIntToEEPROM(EEPROM, flag1, 0);
            writeIntToEEPROM(EEPROM, flag2, 0);
            delay(10);
        } else{
            // warning_section(1);
            // Setting time according to phase
            switch(selected_phase){
                case 1:
                    hour = 0;
                    minute = 0;
                    second = 0;
                    limit_hour = phase_duration_hour[selected_phase];
                    limit_min = phase_duration_min[selected_phase];
                    limit_sec = phase_duration_sec[selected_phase];
                    // limit_time_ms = time_to_ms(final_limit_hour, final_limit_min, final_limit_sec);
                    pre_time = millis();
                    cur_time_flag = true;
                    // cur_time_print();
                    // Serial.println("hello");
                    // Serial.println(limit_time_ms);
                    print_curtime();
                    break;
                case 2:
                    hour = 0;
                    minute = 0;
                    second = 0;
                    limit_hour = phase_duration_hour[selected_phase];
                    limit_min = phase_duration_min[selected_phase];
                    limit_sec = phase_duration_sec[selected_phase];
                    // limit_time_ms = time_to_ms(final_limit_hour, final_limit_min, final_limit_sec);
                    pre_time = millis();
                    cur_time_flag = true;
                    // cur_time_print();
                    // Serial.println(limit_time_ms);
                    print_curtime();
                    break;
                case 3:
                    hour = 0;
                    minute = 0;
                    second = 0;
                    limit_hour = phase_duration_hour[selected_phase];
                    limit_min = phase_duration_min[selected_phase];
                    limit_sec = phase_duration_sec[selected_phase];
                    // limit_time_ms = time_to_ms(final_limit_hour, final_limit_min, final_limit_sec);
                    pre_time = millis();
                    cur_time_flag = true;
                    // cur_time_print();
                    // Serial.println(limit_time_ms);
                    print_curtime();
                    break;
                case 4:
                    hour = 0;
                    minute = 0;
                    second = 0;
                    limit_hour = phase_duration_hour[selected_phase];
                    limit_min = phase_duration_min[selected_phase];
                    limit_sec = phase_duration_sec[selected_phase];
                    // limit_time_ms = time_to_ms(final_limit_hour, final_limit_min, final_limit_sec);
                    pre_time = millis();
                    cur_time_flag = true;
                    // cur_time_print();
                    // Serial.println(limit_time_ms);
                    print_curtime();
                    break;
            }
            print_limittime();
            print_phase();
        }
    }
}


// temperature measurement function
void temp_measure() {
    static uint64_t temp_pre_time = 0; 
    if(millis() - temp_pre_time >= 5000 && selected_interface == 1){
        sensor1.requestTemperatures();
        sensor2.requestTemperatures();

        tempF[0] = sensor1.getTempFByIndex(0);
        tempF[1] = sensor2.getTempFByIndex(0);

        dry_temp_str = "";
        wet_temp_str = "";
        dry_temp_str = String(int(tempF[0]));
        wet_temp_str = String(int(tempF[1]));

        if(tempF[0] >= -10.0){
            dry_temp_str += 'F';
        }
        if(tempF[1] >= -10.0){
            wet_temp_str += 'F';
        }

        print_db_temp();
        print_wb_temp();

        temp_pre_time = millis();
    }
}


// Keypad event
void keypadEvent(KeypadEvent key) {
    switch (keypad.getState()){
        case PRESSED: 
            if(selected_interface == 1){
                // MANUAL
                if(key == 'A'){
                    selected_interface = 2;
                }
                // AUTO
                else if(key == 'B'){
                    selected_mode = 1;
                    selected_phase = 1;
                    hour = 0;
                    minute = 0;
                    second = 0;
                    limit_sec = phase_duration_sec[selected_phase];
                    limit_min = phase_duration_min[selected_phase];
                    limit_hour = phase_duration_hour[selected_phase];

                    home_limit_time_str = time_to_str(limit_hour, limit_min, limit_sec);
                    cur_time_flag = true;

                    limit_db = phase_temp[selected_phase-1][hour][0];
                    limit_wb = phase_temp[selected_phase-1][hour][1];

                    set_dry_temp_str = String(limit_db);
                    set_wet_temp_str = String(limit_wb);
                    pre_interface = -1;
                    selected_interface = 1;

                    writeIntToEEPROM(EEPROM, flag1, 0);
                    writeIntToEEPROM(EEPROM, flag2, 0);
                    delay(10);
                }
                // PHASE
                else if(key == 'C'){
                    selected_interface = 3;
                }
                // RESET
                else if(key == 'D'){
                    ESP.restart();
                }
            }
            else if(selected_interface == 2){
                // Home
                if(key == 'A'){
                    selected_interface = 1;
                }
                
                // Start
                else if(key == 'B'){
                    // Start process

                    int t_hour = 0;
                    int t_min = 0;
                    int t_sec = 0;
                    int t_db = 0;
                    int t_wb = 0;
                    // working code
                    for(int i=0; i<manual_dry_temp_str.length(); i++){
                        int x = manual_dry_temp_str[i] - '0';
                        // if(i != manual_dry_temp_str.length()-1){
                        t_db = t_db * 10;
                        // }
                        t_db += x;
                    }
                    for(int i=0; i<manual_wet_temp_str.length(); i++){
                        int x = manual_wet_temp_str[i] - '0';
                        // if(i != manual_wet_temp_str.length()-1){
                        t_wb = t_wb*10;
                        // }
                        t_wb += x;
                    }
                    int ct = 0;
                    for(int i=0; i<manual_time_str.length(); i++){
                        if(manual_time_str[i] == ':'){
                            ct++;
                            continue;
                        } 
                        else {
                            if(ct == 0)
                                t_hour *= 10;
                            else if(ct == 1){
                                t_min *= 10;
                            }
                            else if(ct == 2){
                                t_sec *= 10;
                            }
                        }
                        int x = manual_time_str[i] - '0';
                        if(ct == 0){
                            t_hour += x; 
                        }
                        else if(ct == 1){
                            t_min += x;
                        }
                        else if(ct == 2){
                            t_sec += x;
                        }
                    }
                    if(t_hour > 60) t_hour = 0; 
                    if(t_min >= 60) t_min = 0; 
                    if(t_sec >= 60) t_sec = 0; 

                    selected_interface = 1;
                    pre_interface = -1;
                    limit_db = t_db;
                    limit_wb = t_wb;
                    limit_hour = t_hour;
                    limit_min = t_min;
                    limit_sec = t_sec;
                    hour = 0;
                    minute = 0;
                    second = 0;
                    selected_mode = 3;
                    cur_time_flag = true;
                    selected_phase = 0;
                    set_dry_temp_str = String(limit_db);
                    set_wet_temp_str = String(limit_wb);

                    writeIntToEEPROM(EEPROM, flag1, 0);
                    writeIntToEEPROM(EEPROM, flag2, 0);
                    delay(10);
                    delay(300);
                }

                // Backspace
                else if(key == 'C'){
                    if(selected_manual_field == 1){
                        if(manual_dry_temp_str.length() > 0){
                            // manual_dry_temp_str[manual_dry_temp_str.length()-1] = '\0';
                            // manual_dry_temp_str.setCharAt(manual_dry_temp_str.length() - 1, '\0');
                            manual_dry_temp_str = manual_dry_temp_str.substring(0, manual_dry_temp_str.length() - 1);
                        }
                    }
                    if(selected_manual_field == 2){
                        if(manual_wet_temp_str.length() > 0){
                            // manual_wet_temp_str[manual_wet_temp_str.length()-1] = '\0';
                            // manual_wet_temp_str.setCharAt(manual_wet_temp_str.length() - 1, '\0');
                            manual_wet_temp_str = manual_wet_temp_str.substring(0, manual_wet_temp_str.length() - 1);
                        }
                    }
                    if(selected_manual_field == 3){
                        if(manual_time_str.length() > 0){
                            // manual_time_str[manual_time_str.length()-1] = '\0';
                            // manual_time_str.setCharAt(manual_time_str.length() - 1, '\0');
                            manual_time_str = manual_time_str.substring(0, manual_time_str.length() - 1);
                        }
                    }
                    field_change_update();

                    delay(300);
                }

                // Field change
                else if(key == 'D'){
                    selected_manual_field++;
                    if(selected_manual_field > 3) selected_manual_field = 1;
                    field_change_update();

                    delay(300);
                }

                if(key == '1') {
                    update_field('1');
                    delay(300);
                }
                if(key == '2') {
                    update_field('2');
                    delay(300);
                }
                if(key == '3') {
                    update_field('3');
                    delay(300);
                }
                if(key == '4') {
                    update_field('4');
                    delay(300);
                }
                if(key == '5') {
                    update_field('5');
                    delay(300);
                }
                if(key == '6') {
                    update_field('6');
                    delay(300);
                }
                if(key == '7') {
                    update_field('7');
                    delay(300);
                }
                if(key == '8') {
                    update_field('8');
                    delay(300);
                }
                if(key == '9') {
                    update_field('9');
                    delay(300);
                }
                if(key == '*') {
                    update_field('.');
                    delay(300);
                }
                if(key == '0') {
                    update_field('0');
                    delay(300);
                }
                if(key == '#') {
                    update_field(':');
                    delay(300);
                }
            }
            else if(selected_interface == 3){
                // Select phase 1
                if(key == '1'){
                    temp_phase_select = 1;
                    update_phase_field();
                    update_phase_time_field();
                    delay(300);
                }

                // Select phase 2
                else if(key == '2'){
                    temp_phase_select = 2;
                    update_phase_field();
                    update_phase_time_field();
                    delay(300);
                }

                // Select phase 3
                else if(key == '3'){
                    temp_phase_select = 3;
                    update_phase_field();
                    update_phase_time_field();
                    delay(300);
                }

                // Select phase 4
                else if(key == '4'){
                    temp_phase_select = 4;
                    update_phase_field();
                    update_phase_time_field();
                    delay(300);
                }

                // Start key
                else if(key == 'A'){
                    // do some start process
                    // selected_phase_phase = 0;

                    // selected_phase = temp_phase_select;
                    // temp_phase_select = 0;
                    // selected_interface = 1;

                    selected_phase = temp_phase_select;
                    selected_mode = 2;
                    pre_interface = -1;
                    selected_interface = 1;
                    temp_phase_select = 0;
                    cur_time_flag = true;
                    hour = 0;
                    minute = 0;
                    second = 0;
                    limit_hour = phase_duration_hour[selected_phase];
                    limit_min = phase_duration_min[selected_phase];
                    limit_sec = phase_duration_sec[selected_phase];

                    writeIntToEEPROM(EEPROM, flag1, 0);
                    writeIntToEEPROM(EEPROM, flag2, 0);
                    delay(10);
                    delay(300);
                }

                // Home key
                else if(key == 'B'){
                    // selected_phase_phase = 0;
                    temp_phase_select = 0;
                    selected_interface = 1;
                    delay(300);
                }
            }
            break;
    }
}