#include <Arduino.h>
#include "web_server.h"
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "BH1750.h"
#include "Adafruit_SSD1306.h"
#include "periphera_devices.h"

_DevicesData_t DevicesData;

#ifdef DHT11_PIN
DHT Dht11(DHT11_PIN, DHT11);
#endif

#ifdef BH1750_I2C
BH1750 Bh1750;
#endif

#ifdef OLED_DISPLAY_I2C
Adafruit_SSD1306 OledDisplay(128, 64, &OLED_DISPLAY_I2C, -1);
#endif

void PeripheralDevices_Init()
{
    DevicesData.NaturalGas = 0;
    Serial.println("外设初始化中...");

    Serial.printf("I2C-1初始化中...  SDA: %d    CLK:%d\n", I2C1_SDA, I2C1_CLK);
    Wire.begin(I2C1_SDA, I2C1_CLK);

#ifdef BUZZER
    Serial.println("蜂鸣器初始化中...");
    pinMode(BUZZER, OUTPUT);
    Buzzer_Close();
#endif
#ifdef DHT11_PIN
    Serial.println("DHT11传感器初始化中...");
    Dht11.begin(DHT11_PIN);
#endif

#ifdef BH1750_I2C
    Serial.println("BH1750传感器初始化中...");
    Bh1750.begin(BH1750_CONTINUOUS_MODE, BH1750_ADDR, &BH1750_I2C);
#endif

#ifdef FIRE_PIN
    Serial.println("火焰传感器初始化中...");
    pinMode(FIRE_PIN, INPUT);
#endif
#ifdef BODY_PIN
    Serial.println("人体红外传感器初始化中...");
    pinMode(BODY_PIN, INPUT);
#endif
#ifdef MQ_4_AO
    Serial.println("MQ-4传感器初始化中...");
    pinMode(A0, INPUT);
#endif
#ifdef RGB_LAMP_RED
    Serial.println("RGB灯红 初始化中...");
    pinMode(RGB_LAMP_RED, OUTPUT);
#endif
#ifdef RGB_LAMP_GREEN
    Serial.println("RGB灯绿 初始化中...");
    pinMode(RGB_LAMP_GREEN, OUTPUT);
#endif
#ifdef RGB_LAMP_BLUE
    Serial.println("RGB灯蓝 初始化中...");
    pinMode(RGB_LAMP_BLUE, OUTPUT);
#endif
#ifdef FAN
    Serial.println("风扇初始化中...");
    pinMode(FAN, OUTPUT);
    digitalWrite(FAN, HIGH);
#endif
#ifdef LAMP
    Serial.println("灯泡初始化中...");
    pinMode(LAMP, OUTPUT);
    digitalWrite(LAMP, HIGH);
#endif

#ifdef OLED_DISPLAY_I2C
    Serial.println("OledDisplay初始化中...");
    if (!OledDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("OledDisplay初始失败!"));
        for (;;)
            delay(100);
    }

#endif
    Serial.println("外设初始化完成!");
}

void PeripheralDevices_GetDataLoop()
{
#ifdef DHT11_PIN
    DevicesData.Humidity = Dht11.readHumidity();
    DevicesData.Temperature = Dht11.readTemperature();
#endif

#ifdef BH1750_I2C
    if (Bh1750.measurementReady())
    {
        DevicesData.Light = Bh1750.readLightLevel();
    }
#endif
#ifdef FIRE_PIN
    DevicesData.Fire = Fire_Read();
#endif
#ifdef BODY_PIN
    DevicesData.Body = digitalRead(BODY_PIN);
#endif
#ifdef FAN
    // DevicesData.Fan=Fan_Read();
#endif
#ifdef LAMP
    // DevicesData.Lamp=Lamp_Read();
#endif
#ifdef MQ_4_AO
    // 无天然气的环境下，实测AOUT端的电压为0.5V
    // 当检测到天然气时，电压每升高0.1V,实际被测气体浓度增加200ppm
    // 电阻分压3/1
    float ad = 3.3 * (analogRead(A0) / 1024.0);
    if (ad < 0.33)
    {
        DevicesData.NaturalGas = (ad / 0.33) * 300;
    }
    else
    {
        DevicesData.NaturalGas = ad / 0.066 * 200;
    }

    //Serial.printf("gas:%.1f  ad:%.2f\n", DevicesData.NaturalGas, ad);
#endif
}