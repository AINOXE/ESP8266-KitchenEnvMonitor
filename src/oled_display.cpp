#include <Arduino.h>
#include "oled_display.h"
#include "periphera_devices.h"
#include <ESP8266WiFi.h>

extern void OledDisplay_UpdateNetworkStatus();
extern void OledDisplay_UpdateEnvStatus();
extern void OledDisplay_UpdateDevicesData();


extern const unsigned char img_caixukun[];
void OledDisplay_Init()
{
    OledDisplay.clearDisplay();
    /*设置字体颜色 不写无法使用OledDisplay.print等方法 */
    OledDisplay.setTextColor(WHITE);
    OledDisplay.drawBitmap(0,0,img_caixukun,128,64,1);
    OledDisplay.display();
}

void OledDisplay_Update()
{
    OledDisplay.clearDisplay();
    OledDisplay_UpdateDevicesData();
    OledDisplay_UpdateEnvStatus();
    OledDisplay_UpdateNetworkStatus();
    OledDisplay.display();
}

extern const unsigned char font_lib16[][32];
extern const unsigned char symbol_lib16[][32];
extern const unsigned char symbol_lib8[][8];

#define OledDisplay_Show16x16(X, Y, CONTENT) \
     OledDisplay.drawBitmap((X-1)*16,(Y-1)*16,CONTENT,16,16,1);
/* 更新网络显示 */
inline void OledDisplay_UpdateNetworkStatus()
{
    /* 显示WiFi是否连接图标 */
    OledDisplay_Show16x16(1, 1, symbol_lib16[WiFi.status() == WL_CONNECTED ? 2 : 3]);
    /* 显示热点是否开启 */
    OledDisplay.setTextSize(1);
    OledDisplay.setCursor(64, 32);
    OledDisplay.printf("AP:%d", 1);
    /* 显示IP地址第四段 */
    OledDisplay.setCursor(52, 40);
    OledDisplay.printf("IP:%d", (int)ipaddr_addr(WiFi.localIP().toString().c_str()) >> 24);
}
/* 更新环境状态显示 */
inline void OledDisplay_UpdateEnvStatus()
{
    /* 显示燃气报警 和 异常图标*/
    if (DevicesData.NaturalGasAlarm)
    {
        OledDisplay_Show16x16(3, 1, font_lib16[15]);
        OledDisplay_Show16x16(4, 1, font_lib16[16]);
        OledDisplay_Show16x16(5, 1, font_lib16[28]);
        OledDisplay_Show16x16(6, 1, font_lib16[29]);
        OledDisplay_Show16x16(8, 1, symbol_lib16[8]);
    }
    /* 显示火焰报警 和 异常图标*/
    else if (DevicesData.FireAlarm)
    {
        OledDisplay_Show16x16(3, 1, font_lib16[12]);
        OledDisplay_Show16x16(4, 1, font_lib16[13]);
        OledDisplay_Show16x16(5, 1, font_lib16[28]);
        OledDisplay_Show16x16(6, 1, font_lib16[29]);
        OledDisplay_Show16x16(8, 1, symbol_lib16[8]);
    }
    /* 显示环境正常 和 正常图标*/
    else
    {
        OledDisplay_Show16x16(3, 1, font_lib16[0]);
        OledDisplay_Show16x16(4, 1, font_lib16[1]);
        OledDisplay_Show16x16(5, 1, font_lib16[2]);
        OledDisplay_Show16x16(6, 1, font_lib16[3]);
        OledDisplay_Show16x16(8, 1, symbol_lib16[7]);
    }
}
/* 更新外设数据 */
inline void OledDisplay_UpdateDevicesData()
{
    /* 切换字体大小 12x16 */
    OledDisplay.setTextSize(2);

    /* 温度:*/
    OledDisplay_Show16x16(1, 2, font_lib16[5]);
    OledDisplay_Show16x16(2, 2, font_lib16[7]);
    OledDisplay.drawBitmap(30, 16, symbol_lib8[0], 8, 16, 1);

    /* 温度值 */
    OledDisplay.setCursor(37, 16);
    OledDisplay.printf("%.0f", DevicesData.Temperature);

    /* 湿度 */
    OledDisplay_Show16x16(5, 2, font_lib16[6]);
    OledDisplay_Show16x16(6, 2, font_lib16[7]);
    OledDisplay.drawBitmap(94, 16, symbol_lib8[0], 8, 16, 1);

    /* 湿度值 */
    OledDisplay.setCursor(101, 16);
    OledDisplay.printf("%.0f", DevicesData.Humidity);

    /* 切换字体大小 6x8 */
    OledDisplay.setTextSize(1);

    /* Light:{光照值} */
    OledDisplay.setCursor(0, 32);
    OledDisplay.printf("Light:%.0f", DevicesData.Light);

    /* Gas:{燃气值} */
    OledDisplay.setCursor(0, 40);
    OledDisplay.printf("Gas:%.0f", DevicesData.NaturalGas);

    /* 人 */
    OledDisplay_Show16x16(2, 4, font_lib16[10]);
    /* 人体 状态 */
    OledDisplay_Show16x16(1, 4, font_lib16[DevicesData.Body ? 25 : 24]);

    /* 火 */
    OledDisplay_Show16x16(5, 4, font_lib16[12]);
    /* 火焰 状态 */
    OledDisplay_Show16x16(4, 4, font_lib16[DevicesData.Fire ? 25 : 24]);

    /* 灯泡Icon */
    OledDisplay_Show16x16(7, 3, symbol_lib16[4]);
    /* 灯泡 开关状态 */
    OledDisplay_Show16x16(8, 3, font_lib16[DevicesData.Lamp ? 26 : 27]);

    /* 风扇Icon */
    OledDisplay_Show16x16(7, 4, symbol_lib16[5]);
    /* 风扇 开关状态*/
    OledDisplay_Show16x16(8, 4, font_lib16[DevicesData.Fan ? 26 : 27]);
}

