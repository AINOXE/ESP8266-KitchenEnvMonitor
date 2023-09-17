#include <Arduino.h>
#include "web_server.h"
#include "periphera_devices.h"
#include "email_send_handler.h"
#include "oled_display.h"

void setup()
{
    Serial.begin(921600);
    FileSystem_Init();
    SystemConfig_Init();
    PeripheralDevices_Init();
    OledDisplay_Init();
    Network_Init();
    WebServer_Init();
    EmailSendHandler_Init();
    // SendEmail(SystemConfig["alarm_email"].as<String>().c_str(), "系统启动", "启动！");
}

void loop()
{
    static unsigned int lampCloseTime = 0;
    WebServer.handleClient();
    PeripheralDevices_GetDataLoop();

#pragma region 配置获取
    int lamp_on_light = SystemConfig["lamp_on_light"];
    int lamp_off_light = SystemConfig["lamp_off_light"];
    int on_lamp_must_has_body = SystemConfig["on_lamp_must_has_body"];
    int no_body_close_lamp_time = SystemConfig["no_body_close_lamp_time"];
    int natural_gas_alarm = SystemConfig["natural_gas_alarm"];
    String alarm_email = SystemConfig["alarm_email"];
#pragma endregion

#pragma region 灯光控制
    /* 到达关灯时间关灯 */
    if (lampCloseTime != 0 && system_get_time() > lampCloseTime)
    {
        Serial.println("关灯: 到达设定时间！");
        Lamp_Close();
        lampCloseTime = 0;
    }
    /* 光照低于开灯设定值*/
    if (DevicesData.Light < lamp_on_light)
    {
        /* 光照低于开灯设定值 开灯 : 开灯不需要 必须有人 */
        if (!on_lamp_must_has_body)
        {
            /* 未开灯时再开 */
            if (!Lamp_Read())
            {
                Lamp_Open();
                lampCloseTime = 0;
                Serial.printf("开灯: [光照低于设定值] -> %.0f<%d\n", DevicesData.Light, lamp_on_light);
            }
        }
        /* 光照低于开灯设定值 开灯 : 开灯时必须有人 */
        else if (DevicesData.Body)
        {
            /* 未开灯时再开 */
            if (!Lamp_Read())
            {
                Lamp_Open();
                lampCloseTime = 0;
                Serial.printf("开灯: [有人]+[光照低于设定值] -> %.0f<%d\n", DevicesData.Light, lamp_on_light);
            }
        }
        /* 光照低于开灯设定值 无人 关灯 : 开灯时必须有人，当前无人 所以关灯 */
        else if (lampCloseTime == 0)
        {

            /* 未关灯时再关 */
            if (Lamp_Read())
            {
                /* 计算关灯时间 */
                lampCloseTime = system_get_time() + no_body_close_lamp_time * 1000;
                if (lampCloseTime != 0 && system_get_time() > lampCloseTime)
                {

                    Lamp_Close();
                    lampCloseTime = 0;
                }
                Serial.printf("关灯: [无人]+[光照低于设定值] -> %.0f<%d  %d毫秒后自动关灯!\n", DevicesData.Light, lamp_off_light, no_body_close_lamp_time);
            }
        }
    }
    /* 光照高于关灯设定值 关灯 并且当前灯已开 */
    else if (DevicesData.Light > lamp_off_light && Lamp_Read())
    {
        lampCloseTime = 0;
        Lamp_Close();
        Serial.printf("关灯: [光照高于设定值] -> %.0f>%d\n", DevicesData.Light, lamp_off_light);
    }
    /* 开灯时必须有人条件启用时  [无人] 并且 [光照值不在管理区间内] = 延时关灯 */
    else if (on_lamp_must_has_body && lampCloseTime == 0 && Lamp_Read())
    {
        /* 计算关灯时间 */
        lampCloseTime = system_get_time() + no_body_close_lamp_time * 1000;
        if (lampCloseTime != 0 && system_get_time() > lampCloseTime)
        {
            Lamp_Close();
            lampCloseTime = 0;
        }
        Serial.printf("关灯: [无人]  %d毫秒后自动关灯!\n", no_body_close_lamp_time);
    }
#pragma endregion

#pragma region 火焰检查
    /* 在没有人的时候有火 则触发火焰报警 */
    if (DevicesData.Body == 0 && DevicesData.Fire == 1)
    {
        if (DevicesData.FireAlarm == 0)
        {
            /* 开启火焰报警 */
            DevicesData.FireAlarm = 1;
            /* 打开蜂鸣器 */
            Buzzer_Open();
            /* 发送报警通知 */
            // SendEmail("lzy-0213@outlook.com", "厨房着火报警通知", "Test");
            OledDisplay_Update();
            SendEmail(
                SystemConfig["alarm_email"].as<String>().c_str(),
                "厨房着火报警通知", "尊敬的用户你好，厨房检测到火焰。请及时处理");
        }
    }
    /* 不满足报警条件 并且 当前已开启报警 */
    /* 执行关闭报警 */
    else if (DevicesData.FireAlarm == 1)
    {
        DevicesData.FireAlarm = 0;
        /* 关闭蜂鸣器 */
        Buzzer_Close();
        // /* 发送报警解除通知 */
        // SendEmail("lzy-0213@outlook.com", "厨房着火报警通知", "Test");
        OledDisplay_Update();
        SendEmail(
            SystemConfig["alarm_email"].as<String>().c_str(),
            "厨房着火报警解除通知", "尊敬的用户你好，厨房的火焰已熄灭。请及时复查！");
    }
#pragma endregion

#pragma region 燃气检查
    /* 当前燃气值大于燃气报警值 */
    if (DevicesData.NaturalGas > natural_gas_alarm)
    {
        /* 当前未触发燃气报警 执行开启燃气报警 */
        /* 开启燃气异常警报 */
        if (DevicesData.NaturalGasAlarm == 0)
        {
            DevicesData.NaturalGasAlarm = 1;
            /* 打开蜂鸣器 */
            Buzzer_Open();
            /* 打开风扇 */
            Fan_Open();
            OledDisplay_Update();
            /* 发送报警通知 */
            char buf[1024];
            sprintf(buf, "尊敬的用户你好，厨房的燃气值 %.1f 高于设定值 %s ，现已开启排风扇和蜂鸣器。请及时处理",
                    DevicesData.NaturalGas, SystemConfig["natural_gas_alarm"].as<String>().c_str());
            SendEmail(
                SystemConfig["alarm_email"].as<String>().c_str(),
                "燃气异常报警通知", buf);
        }
    }
    /* 当前燃气值小于燃气报警值 并且 当前处于燃气报警状态 */
    /* 执行关闭燃气报警 */
    else if (DevicesData.NaturalGasAlarm == 1)
    {
        /* 关闭燃气异常警报 */
        DevicesData.NaturalGasAlarm = 0;
        /* 关闭蜂鸣器 */
        Buzzer_Close();
        /* 关闭蜂鸣器 */
        Fan_Close();
        OledDisplay_Update();
        /* 发送报警解除通知 */
        char buf[1024];
        sprintf(buf, "尊敬的用户你好，厨房的燃气值 %.1f 已低于设定值 %s ，现已关闭排风扇和蜂鸣器。请及时复查！",
                DevicesData.NaturalGas, SystemConfig["natural_gas_alarm"].as<String>().c_str());
        delay(1);
        SendEmail(
            SystemConfig["alarm_email"].as<String>().c_str(),
            "燃气报警解除通知", buf);
    }
#pragma endregion

    OledDisplay_Update();
    delay(1);
}