#include <Wire.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <TimerOne.h>
#include <Time.h>
#include <ClickEncoder.h>
#include <TEA5767.h>
#include <TDA7439.h>
#define NEC
#include <IRremote.h>
#include <SSD1306_text.h>
#include "remote.h"

ClickEncoder *encoder;
int16_t encoder_last_value, encoder_current_value;

unsigned long lastChange = 0;
unsigned int current_menu = DEFAULT_MENU;
boolean eqmute = false;
boolean save_station = false;
unsigned long last_ir_command;

#define encResDivider 100
void timerIsr() { encoder->service(); }

IRrecv irrecv(INFRARED_PIN);
decode_results results;

TDA7439 equ;
TEA5767 Radio;
int search_mode = 0;
unsigned char buf[10];
SSD1306_text oled;

void setup() {
  pinMode(DEBUG_LED, OUTPUT); 

  EEPROM_readAnything(0, configuration);
  encoder = new ClickEncoder(ENCODER_CLK, ENCODER_DT, ENCODER_SW, ENCODER_STEPS);
  encoder->setAccelerationEnabled(false);
  
  if(digitalRead(ENCODER_SW) == LOW){
    for(int g=0; g<=10; g++){
      digitalWrite(DEBUG_LED, !digitalRead(DEBUG_LED));
      delay(500);
    }
    eq_init();
  }

  digitalWrite(DEBUG_LED, HIGH);
  Wire.begin();

  Radio.init();
  Radio.set_frequency(configuration.frequency);

  oled.init(); oled.clear();
  oled.setTextTransparent(false);
  oled.sendCommand(SSD1306_SETCONTRAST);
  oled.sendCommand(configuration.oled);

  equ.setInput(configuration.activeInput);
  equ.inputGain(configuration.gainLevel);
  equ.setSnd(configuration.bassLevel, 1);
  equ.setSnd(configuration.midLevel, 2);
  equ.setSnd(configuration.trebLevel, 3);
  equ.spkAtt(configuration.attLevel);
  //configuration.volumeLevel = MAX_START_VOLUME;
  for(byte i = 0; i <= configuration.volumeLevel; i++ ){ equ.setVolume(i); delay(6); }

  irrecv.enableIRIn();

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  encoder_last_value = 0;
  
}

void eq_init(){
    configuration.activeInput   = 4;
    configuration.volumeLevel   = 3;
    configuration.bassLevel     = 5;
    configuration.midLevel      = 1;
    configuration.trebLevel     = 3;
    configuration.attLevel      = 0;
    configuration.gainLevel     = 0;
    configuration.frequency     = VIVIDH_BHARATHI;
    configuration.station_1     = VIVIDH_BHARATHI;
    configuration.station_2     = MIRCHI;
    configuration.station_3     = RADIO_CITY;
    configuration.station_4     = BIG_FM;
    configuration.station_5     = RED_FM;
    configuration.station_6     = IGNOU;
    configuration.station_7     = RAINBOW;
    configuration.station_8     = VIVIDH_BHARATHI;
    configuration.station_9     = VIVIDH_BHARATHI;
    configuration.oled          = 93;
    EEPROM_writeAnything(0, configuration);  
}

boolean oled_display = true;
unsigned int station_index = 0;

void loop(){
  encoder_current_value += encoder->getValue();

  if(irrecv.decode(&results)){
    if(results.decode_type == NEC){
      process_ir(results.value);
      delay(50);
    }
    irrecv.resume(); // Receive the next value
  }

  if(encoder_current_value != encoder_last_value){
    process_encoder((encoder_current_value - encoder_last_value));
    encoder_last_value = encoder_current_value;
    lastChange = millis();
    oled_display = true;
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    switch (b) {
      case ClickEncoder::Pressed:
        break;

      case ClickEncoder::Clicked:

          if(configuration.activeInput == FM_RADIO){
            station_index = station_index + 1;
            if(station_index > 7) station_index = 1;

            if(station_index == 1) configuration.frequency = VIVIDH_BHARATHI;
            if(station_index == 2) configuration.frequency = RAINBOW;
            if(station_index == 3) configuration.frequency = MIRCHI;
            if(station_index == 4) configuration.frequency = RADIO_CITY;
            if(station_index == 5) configuration.frequency = BIG_FM;
            if(station_index == 6) configuration.frequency = RED_FM;
            if(station_index == 7) configuration.frequency = IGNOU;

            current_menu = FM_FREQUENCY;
            Radio.set_frequency(configuration.frequency);
            for(int i = configuration.volumeLevel; i >= 0; i-- ){ equ.setVolume(i); delay(6); }
            configuration.activeInput = FM_RADIO;
            equ.setInput(configuration.activeInput);
            for(int i = 0; i <= configuration.volumeLevel; i++ ){ equ.setVolume(i); delay(6); }
            EEPROM_writeAnything(0, configuration);
            lastChange = 0;
          }else{
            current_menu++;
            choose_menu();
          }  
        break;
      
      case ClickEncoder::DoubleClicked:
        if(configuration.activeInput != FM_RADIO){
          current_menu = FM_FREQUENCY;
          Radio.set_frequency(configuration.frequency);
          for(int i = configuration.volumeLevel; i >= 0; i-- ){ equ.setVolume(i); delay(6); }
          configuration.activeInput = FM_RADIO;
          equ.setInput(configuration.activeInput);
          for(int i = 0; i <= configuration.volumeLevel; i++ ){ equ.setVolume(i); delay(6); }
          EEPROM_writeAnything(0, configuration);
          lastChange = 0;
        }else{
          for(int i = configuration.volumeLevel; i >= 0; i-- ){ equ.setVolume(i); delay(6); }
          configuration.activeInput = AUX_INPUT_ONE;
          equ.setInput(configuration.activeInput);
          for(int i = 0; i <= configuration.volumeLevel; i++ ){ equ.setVolume(i); delay(6); }
          EEPROM_writeAnything(0, configuration);
          lastChange = 0;
        }
        break;
    }
  }    

  unsigned long currentMillis = millis();

  if((currentMillis - lastChange >= MENU_TIMEOUT && oled_display) || lastChange == 0){
    current_menu = DEFAULT_MENU;
    char dbuffer[10];

    sprintf(dbuffer, "%-4s %02d", InputChannels[configuration.activeInput],configuration.volumeLevel);
    oled.setTextSize(3,3);
    oled.setCursor(5, 4);
    oled.print(dbuffer);
    oled.setTextSize(1,2);
    
    switch(configuration.activeInput){
      case AUX_INPUT_ONE:
      case AUX_INPUT_TWO:
        oled.setCursor(5, 42);
        oled.print("INPUT-"); oled.print(configuration.activeInput);
        oled.setCursor(7, 42);
        oled.print("AUX  IN");
        break;
        
      case BLUETOOTH:
        oled.setCursor(5, 42);
        oled.print("UHF RFI");
        oled.setCursor(7, 42);
        oled.print("2G485UL");
        break;
        
      case FM_RADIO:
        oled.setCursor(5, 46);
        oled.print(configuration.frequency);
        oled.setCursor(7, 46);
      
        unsigned char buf[10];
        if (Radio.read_status(buf) == 1) {
          if(Radio.stereo(buf)){
              oled.print("STEREO");
          }else{
            oled.print("MONO");
          }
        }else{
          oled.print("ERROR*");
        }

        break;
    }

    lastChange = millis(); oled_display = false; save_station = false;
  }


}

void process_ir(unsigned long ir_command){

  if(ir_command != CA_REPEAT) last_ir_command = ir_command;

  switch(ir_command){
    case CA_PLAY_NEXT:
      configuration.activeInput = configuration.activeInput + 1;
      if(configuration.activeInput > MAX_INPUTS) configuration.activeInput = 1;
      for(int i = configuration.volumeLevel; i >= 0; i-- ){ equ.setVolume(i); delay(5); }
      equ.setInput(configuration.activeInput);
      for(int i = 0; i <= configuration.volumeLevel; i++ ){ equ.setVolume(i); delay(5); }
      process_ir(CA_CANCEL);
      break;

    case CA_MENU:
        current_menu++; choose_menu();  
      break;

    case CA_REWIND:
      if(configuration.activeInput == FM_RADIO){
        current_menu = FM_FREQUENCY;
        oled.setTextSize(3,3); oled.setCursor(5, 4); oled.print("       ");
        process_encoder(-1);
      }
      break;

    case CA_PREVIOUS:
      configuration.activeInput = configuration.activeInput - 1;
      if(configuration.activeInput < 1) configuration.activeInput = MAX_INPUTS;
      for(int i = configuration.volumeLevel; i >= 0; i-- ){ equ.setVolume(i); delay(5); }
      equ.setInput(configuration.activeInput);
      for(int i = 0; i <= configuration.volumeLevel; i++ ){ equ.setVolume(i); delay(5); }
      process_ir(CA_CANCEL);
      break;

    case CA_FORWARD:
      if(configuration.activeInput == FM_RADIO){
        current_menu = FM_FREQUENCY;
        oled.setTextSize(3,3); oled.setCursor(5, 4); oled.print("       ");
        process_encoder(1);
      }
      break;

    case CA_KEY_ONE:
    case CA_KEY_TWO:
    case CA_KEY_THREE:
    case CA_KEY_FOUR:
    case CA_KEY_FIVE:
    case CA_KEY_SIX:
    case CA_KEY_SEVEN:
    case CA_KEY_EIGHT:
    case CA_KEY_NINE:
      if(save_station && configuration.activeInput == FM_RADIO){
        save_fm_station(ir_command);
      }else if(configuration.activeInput == FM_RADIO){
        if(ir_command == CA_KEY_ONE)    configuration.frequency = configuration.station_1;
        if(ir_command == CA_KEY_TWO)    configuration.frequency = configuration.station_2;
        if(ir_command == CA_KEY_THREE)  configuration.frequency = configuration.station_3;
        if(ir_command == CA_KEY_FOUR)   configuration.frequency = configuration.station_4;
        if(ir_command == CA_KEY_FIVE)   configuration.frequency = configuration.station_5;
        if(ir_command == CA_KEY_SIX)    configuration.frequency = configuration.station_6;
        if(ir_command == CA_KEY_SEVEN)  configuration.frequency = configuration.station_7;
        if(ir_command == CA_KEY_EIGHT)  configuration.frequency = configuration.station_8;
        if(ir_command == CA_KEY_NINE)   configuration.frequency = configuration.station_9;
        Radio.set_frequency(configuration.frequency);
        EEPROM_writeAnything(0, configuration);
        lastChange = 0;
        oled_display = true;
      }else{
        if(ir_command == CA_KEY_ONE)    current_menu = VOLUME_LEVEL; choose_menu();
        if(ir_command == CA_KEY_TWO)    current_menu = BASS_LEVEL; choose_menu();
        if(ir_command == CA_KEY_THREE)  current_menu = MID_LEVEL; choose_menu();
        if(ir_command == CA_KEY_FOUR)   current_menu = TREBLE_LEVEL; choose_menu();
        if(ir_command == CA_KEY_FIVE)   current_menu = SELECT_INPUT; choose_menu();
        if(ir_command == CA_KEY_SIX)    current_menu = FM_FREQUENCY; choose_menu();
        if(ir_command == CA_KEY_SEVEN)  current_menu = INPUT_GAIN; choose_menu();
        if(ir_command == CA_KEY_EIGHT)  current_menu = BRIGHTNESS; choose_menu();
      }
      break;
    
    case CA_MUTE:
      if(eqmute){
        for(int i = 0; i <= configuration.volumeLevel; i++ ){ equ.setVolume(i); delay(6); }
        oled.setTextSize(1, 2);
        oled.setCursor(3, 99);
        oled.print("    ");
        eqmute = false;
      }else{
        for(int i = configuration.volumeLevel; i >= 0; i-- ){ equ.setVolume(i); delay(6); }
        oled.setTextSize(1, 2);
        oled.setCursor(3, 99);
        oled.print("MUTE");
        eqmute = true;
      }
      break;

    case CA_VOL_UP:
      process_encoder(1);
      break;

    case CA_VOL_DOWN:
      process_encoder(-1);
      break;

    case CA_SHIFT:
      if(configuration.activeInput == FM_RADIO){
        oled.setTextSize(3,3);
        oled.setCursor(5, 4);
        oled.print("SAVE #?");
        save_station = true;
        oled_display = true;
      }
      break;

    case CA_CANCEL:
      lastChange = 0;
      oled_display = true;
      break;

    case CA_ENTER:
      break;

    case CA_SEARCH:
      break;

    case CA_REPEAT:
      process_ir(last_ir_command);
      break;
    
  }
  
}

void save_fm_station(unsigned long fm_number){
  lastChange = millis();
  oled_display = true;
  byte i;

  if(fm_number == CA_KEY_ONE){ configuration.station_1 = configuration.frequency; i = 1; }
  if(fm_number == CA_KEY_TWO){ configuration.station_2 = configuration.frequency; i = 2; }
  if(fm_number == CA_KEY_THREE){ configuration.station_3 = configuration.frequency; i = 3; }
  if(fm_number == CA_KEY_FOUR){ configuration.station_4   = configuration.frequency; i = 4; }
  if(fm_number == CA_KEY_FIVE){  configuration.station_5   = configuration.frequency; i = 5; }
  if(fm_number == CA_KEY_SIX){   configuration.station_6   = configuration.frequency; i = 6; }
  if(fm_number == CA_KEY_SEVEN){ configuration.station_7   = configuration.frequency; i = 7; }
  if(fm_number == CA_KEY_EIGHT){ configuration.station_8   = configuration.frequency; i = 8; }
  if(fm_number == CA_KEY_NINE){  configuration.station_9   = configuration.frequency; i = 9; }
  
  EEPROM_writeAnything(0, configuration); 
  oled.setTextSize(3,3);
  oled.setCursor(5, 4);
  oled.print("SAVED "); oled.print(i);

  save_station = false;
}

void choose_menu(){
  lastChange = millis();
  oled_display = true;
  if(configuration.activeInput != FM_RADIO && current_menu == FM_FREQUENCY) current_menu++;
  if(current_menu > MAX_MENU_ITEMS) current_menu = DEFAULT_MENU;
  char dbuffer[10];

  switch(current_menu){
    case VOLUME_LEVEL:
      sprintf(dbuffer, "%s %02d", MenuItems[current_menu],configuration.volumeLevel);
      break;
      
    case BASS_LEVEL:
      sprintf(dbuffer, "%s %02d", MenuItems[current_menu],configuration.bassLevel);
      break;
      
    case MID_LEVEL:
      sprintf(dbuffer, "%s %02d", MenuItems[current_menu],configuration.midLevel);
      break;
      
    case TREBLE_LEVEL:
      sprintf(dbuffer, "%s %02d", MenuItems[current_menu],configuration.trebLevel);
      break;
      
    case SELECT_INPUT:
      sprintf(dbuffer, "%s %s", MenuItems[current_menu], InputChannels[configuration.activeInput]);
      break;
      
    case FM_FREQUENCY:
      oled.setTextSize(3,3);
      oled.setCursor(5, 4);
      oled.print("       ");
      if (Radio.read_status(buf) == 1) {
        char str_fm[10];
        dtostrf(configuration.frequency, 5, 1, str_fm);
        sprintf(dbuffer, "%s%s",MenuItems[current_menu], str_fm);
      }else{
        sprintf(dbuffer, "%s", "FM ERR!");
      }
      break;
      
    case INPUT_GAIN:
      sprintf(dbuffer, "%s %02d", MenuItems[current_menu], configuration.gainLevel);
      break;

    case BRIGHTNESS:
      sprintf(dbuffer, "%s %03d", MenuItems[current_menu], configuration.oled);
      break;

    default:
      sprintf(dbuffer, "%s", "ERROR!!");
  }
  oled.setTextSize(3,3);
  oled.setCursor(5, 4);
  oled.print(dbuffer);
  EEPROM_writeAnything(0, configuration);
}


void process_encoder(int displacement){
  lastChange = millis();
  oled_display = true;
  char dbuffer[10];
  
  switch(current_menu){
    case VOLUME_LEVEL:
      configuration.volumeLevel = constrain((configuration.volumeLevel + displacement), 0, MAX_VOLUME);
      equ.setVolume(configuration.volumeLevel);
      if(eqmute){
        oled.setTextSize(1, 2);
        oled.setCursor(3, 99);
        oled.print("    ");
        eqmute = false;
      }
      sprintf(dbuffer, "%s %02d", MenuItems[current_menu], configuration.volumeLevel);
      break;
      
    case BASS_LEVEL:
      configuration.bassLevel = constrain((configuration.bassLevel + displacement), -7, 7);
      equ.setSnd(configuration.bassLevel,1);
      sprintf(dbuffer, "%s %02d", MenuItems[current_menu], configuration.bassLevel);
      break;
      
    case MID_LEVEL:
      configuration.midLevel  = constrain((configuration.midLevel  + displacement), -7, 7);
      equ.setSnd(configuration.midLevel,2);
      sprintf(dbuffer, "%s %02d", MenuItems[current_menu], configuration.midLevel);
      break;
      
    case TREBLE_LEVEL:
      configuration.trebLevel  = constrain((configuration.trebLevel  + displacement), -7, 7);
      equ.setSnd(configuration.trebLevel,3);
      sprintf(dbuffer, "%s %02d", MenuItems[current_menu], configuration.trebLevel);
      break;
      
    case SELECT_INPUT:
      configuration.activeInput = configuration.activeInput + displacement;
      if(configuration.activeInput < 1) configuration.activeInput = MAX_INPUTS;
      if(configuration.activeInput > MAX_INPUTS) configuration.activeInput = 1;
      for(int i = configuration.volumeLevel; i >= 0; i-- ){ equ.setVolume(i); delay(5); }
      equ.setInput(configuration.activeInput);
      for(int i = 0; i <= configuration.volumeLevel; i++ ){ equ.setVolume(i); delay(5); }
      sprintf(dbuffer, "%s %s", MenuItems[current_menu], InputChannels[configuration.activeInput]);
      break;
      
    case FM_FREQUENCY:
      if(displacement == -1) configuration.frequency -= 0.1;
      if(displacement == 1) configuration.frequency += 0.1;
      if(configuration.frequency < 88) configuration.frequency = 108;
      if(configuration.frequency > 108) configuration.frequency = 88;
      Radio.set_frequency(configuration.frequency);

      if (Radio.read_status(buf) == 1) {
        char str_fm[10];
        dtostrf(configuration.frequency, 5, 1, str_fm);
        sprintf(dbuffer, "%s%s", MenuItems[current_menu], str_fm);
      }else{
        sprintf(dbuffer, "%s", "FM ERR!");
      }
      break;
      
    case INPUT_GAIN:
      configuration.gainLevel = constrain((configuration.gainLevel + displacement), 0, 15);
      equ.inputGain(configuration.gainLevel);
      sprintf(dbuffer, "%s %02d", MenuItems[current_menu], configuration.gainLevel);
      break;

    case BRIGHTNESS:
      configuration.oled = constrain((configuration.oled + displacement), 1, 255);
      oled.sendCommand(SSD1306_SETCONTRAST);
      oled.sendCommand(configuration.oled);
      sprintf(dbuffer, "%s %03d", MenuItems[current_menu], configuration.oled);
      break;

    default:
      sprintf(dbuffer, "%s", "ERROR!!");
  }
  oled.setTextSize(3,3);
  oled.setCursor(5, 4);
  oled.print(dbuffer);
  
  EEPROM_writeAnything(0, configuration);
}
