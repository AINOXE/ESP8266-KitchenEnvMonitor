#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <Arduino.h>
void PeripheralDevices_Init();
void PeripheralDevices_GetDataLoop();

/* 使用DHT11 */
#define DHT11_PIN D6
/* 使用BH1750 */
#define BH1750_I2C Wire
/* 使用OLED_DISPLAY */
#define OLED_DISPLAY_I2C Wire
/* 使用火焰 */
#define FIRE_PIN D7
/* 使用人体 */
#define BODY_PIN D5
/* 使用天然气 */
#define MQ_4_AO A0

/* 风扇 */
#define FAN D0
/* 灯泡 */
#define LAMP D1
/* 蜂鸣器 */
#define BUZZER D4
/* RGB灯 红 */
#define RGB_LAMP_RED D8
/* RGB灯 绿 */
#define RGB_LAMP_GREEN D4
/* RGB灯 蓝 */
//#define RGB_LAMP_BLUE

#define RgbLamp_OpenAll()               \
    digitalWrite(RGB_LAMP_RED, HIGH);   \
    digitalWrite(RGB_LAMP_GREEN, HIGH); \
    //    digitalWrite(RGB_LAMP_BLUE,HIGH);

#define RgbLamp_CloseAll()             \
    digitalWrite(RGB_LAMP_RED, LOW);   \
    digitalWrite(RGB_LAMP_GREEN, LOW); \
    //    digitalWrite(RGB_LAMP_BLUE,LOW);

#define RgbLamp_Switch(Lamp)               \
    digitalWrite(Lamp, HIGH);              \
    if (Lamp != RGB_LAMP_RED)              \
        digitalWrite(RGB_LAMP_RED, LOW);   \
    if (Lamp != RGB_LAMP_GREEN)            \
        digitalWrite(RGB_LAMP_GREEN, LOW); \
    // if (Lamp != RGB_LAMP_BLUE)             
    //     digitalWrite(RGB_LAMP_BLUE, LOW);

struct _DevicesData_t
{
#ifdef DHT11_PIN
    float Temperature;
    float Humidity;
#endif
#ifdef BH1750_I2C
    float Light;
#endif
#ifdef FIRE_PIN
    int Fire;
    int FireAlarm;
#endif
#ifdef BODY_PIN
    int Body;
#endif
#ifdef FAN
    int Fan;
#endif
#ifdef LAMP
    int Lamp;
#endif
#ifdef BUZZER
    int Buzzer;
#endif
#ifdef MQ_4_AO
    float NaturalGas;
    int NaturalGasAlarm;
#endif
};

#ifdef FAN
#define Fan_Read() DevicesData.Fan
#define Fan_Open() digitalWrite(FAN,LOW); DevicesData.Fan=1
#define Fan_Close() digitalWrite(FAN,HIGH); DevicesData.Fan=0
#define Fan_Control(Value) if(Value) {Fan_Open();} else {Fan_Close();}
#define Fan_Toggle() digitalWrite(FAN,!digitalRead(FAN))
#endif


#ifdef LAMP
#define Lamp_Read() DevicesData.Lamp
#define Lamp_Open() digitalWrite(LAMP,LOW); DevicesData.Lamp=1
#define Lamp_Close() digitalWrite(LAMP,HIGH); DevicesData.Lamp=0
#define Lamp_Control(Value) if(Value){ Lamp_Open(); }else{Lamp_Close();}
#define Lamp_Toggle() digitalWrite(LAMP,!digitalRead(LAMP))
#endif

#ifdef BUZZER
#define Buzzer_Read() DevicesData.Buzzer
#define Buzzer_Open() digitalWrite(BUZZER,LOW); DevicesData.Buzzer=1
#define Buzzer_Close() digitalWrite(BUZZER,HIGH); DevicesData.Buzzer=0
#define Buzzer_Control(Value) if(Value){ Buzzer_Open(); }else{Buzzer_Close();}
#define Buzzer_Toggle() Buzzer_Control(!Buzzer_Read())
#endif

#ifdef FIRE_PIN
#define Fire_Read() !digitalRead(FIRE_PIN);
#endif
extern struct _DevicesData_t DevicesData;

#define I2C1_SDA D2
#define I2C1_CLK D3

#define I2C2_SDA D2
#define I2C2_CLK D3

#ifdef DHT11_PIN
#include "DHT.h"
#include "Adafruit_Sensor.h"

extern DHT Dht11;
#endif

#ifdef BH1750_I2C
#include <Wire.h>
#include "BH1750.h"
#define BH1750_ADDR 0x23
#define BH1750_CONTINUOUS_MODE BH1750::CONTINUOUS_HIGH_RES_MODE
extern BH1750 Bh1750;
#endif

#ifdef OLED_DISPLAY_I2C
#include <Wire.h>
#include "Adafruit_SSD1306.h"
extern Adafruit_SSD1306 OledDisplay;
#endif

#endif