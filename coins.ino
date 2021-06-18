#include <LiquidCrystal.h>
#include "EEPROMex.h"
#include "LCD_1602_RUS.h"

#define LED 8
#define IRpin 10
#define IRsens A0
#define coin_amount 5
#define button 7

float coin_value[coin_amount] = {0.5, 1.0, 2.0, 5.0, 10.0};
String currency = "RUB";
LiquidCrystal lcd(11, 12, 5, 4, 3, 2);

int coin_signal[coin_amount];
int coin_quantity[coin_amount];
int empty_signal = 300;
unsigned long standby_timer, reset_timer;
float summ_money = 0;
boolean recogn_flag, sleep_flag = true;
int sens_signal, last_sens_signal;
boolean coin_flag = false;

void setup() {
  lcd.begin(16, 2);
  pinMode(button, INPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  Serial.begin(9600);
  pinMode(IRpin, OUTPUT);
  digitalWrite(IRpin, 1);

  // === НАЧАЛО РАБОТЫ ===
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Servis");
  delay(500);
  reset_timer = millis();
  while (1) {
    for (byte i = 0; i < coin_amount; i++) {
      coin_quantity[i] = 0;
      EEPROM.writeInt(20 + i * 2, 0);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Memory clear");
    delay(100);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Calibrovka");
    break;
  }
  empty_signal = analogRead(IRsens);
  Serial.print("empty_signal = ");
  Serial.println(empty_signal);

  while (1) {
    for (byte i = 0; i < coin_amount; i++) {
      lcd.setCursor(0, 1); lcd.print(coin_value[i]);
      lcd.setCursor(13, 1); lcd.print(currency);
      empty_signal = analogRead(IRsens);
      last_sens_signal = empty_signal;
      while (1) {
        sens_signal = analogRead(IRsens);
        Serial.println(sens_signal);
        if (sens_signal > last_sens_signal) last_sens_signal = sens_signal;
        if (sens_signal - empty_signal > 3) coin_flag = true;
        if (coin_flag && (abs(sens_signal - empty_signal)) < 2) {
          coin_signal[i] = last_sens_signal;
          EEPROM.writeInt(i * 2, coin_signal[i]);
          coin_flag = false;
          break;
        }
      }
      Serial.print("coin_signal ");
      Serial.print(i);
      Serial.print(" = ");
      Serial.println(coin_signal[i]);
    }
    break;
  }

  for (byte i = 0; i < coin_amount; i++) {
    coin_signal[i] = EEPROM.readInt(i * 2);
    coin_quantity[i] = EEPROM.readInt(20 + i * 2);
    Serial.print(coin_signal[i]);
    Serial.print(" ");
  }
  summ_money = 18.5;
  Serial.print("sum: ");
  Serial.println(summ_money);
}


void loop() {
  // === ОСНОВНАЯ ЧАСТЬ ===
  delay(500);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("GOLD");
  lcd.setCursor(0, 1); lcd.print(summ_money);
  lcd.setCursor(13, 1); lcd.print(currency);
  empty_signal = analogRead(IRsens);
  sleep_flag = false;
  reset_timer = millis();
  last_sens_signal = empty_signal;
  while (1) {
    if (!digitalRead(button)) {
      if (millis() - reset_timer > 3000) {
        setup();
      }
    }
    sens_signal = analogRead(IRsens);  
    if (sens_signal > last_sens_signal) last_sens_signal = sens_signal;
    if (sens_signal - empty_signal > 3) coin_flag = true;
    if (coin_flag && (abs(sens_signal - empty_signal)) < 2) {
      recogn_flag = false; 
      for (byte i = 0; i < coin_amount; i++) {
        int delta = abs(last_sens_signal - coin_signal[i]);   
        if (delta < 30) {   
          summ_money += coin_value[i]; 
          lcd.setCursor(0, 1); lcd.print(summ_money);
          coin_quantity[i]++; 
          recogn_flag = true;
          break;
        }
      }
      coin_flag = false;
      standby_timer = millis(); 
      break;
    }
  }
  Serial.print("sum: ");
  Serial.println(summ_money);
}
