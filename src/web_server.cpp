#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <functional>
#include "web_server.h"
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>

#include "periphera_devices.h"

ESP8266WebServer WebServer(80);

const char *CONTENT_TYPE_TEXT = "text/plan;charset=UTF-8";
const char *CONTENT_TYPE_HTML = "text/html;charset=UTF-8";
const char *CONTENT_TYPE_JSON = "application/json;charset=UTF-8";

const char *WEB_RESULT_SUCCESS_JSON = "{\"code\":0,\"msg\":null}";
const char *WEB_RESULT_ERROR_TEMPLATE_JSON = "{\"code\":%d,\"msg\":\"%s\"}";

extern void WebApi_File_Download();
extern void WebApi_File_Upload();

#define MapUri(URI, METHOD) \
    Serial.printf("MapUri: %s\n", URI); WebServer.on(URI, METHOD, []() -> void

void WebServer_Init()
{
    Serial.println("Web服务器初始化中...");
    /* 主页 */
    MapUri("/", HTTP_GET)
    {
        WebServer.sendHeader("Location", "/main.html.gz");
        WebServer.send(303);
    });

    /* 数据获取API */
    MapUri("/api/dev/datas", HTTP_GET)
    {
        char buf[512];
        // Serial.printf("light: %.0f,%.1f",DevicesData.Light,DevicesData.Light);
        sprintf(buf,
                "{\"temperature\":%.1f,\"humidity\":%.1f,\"light\":%.1f,\"body\":%d,\"fire\":%d,\"natural_gas\":%.1f,\"fan\":%d,\"lamp\":%d}",
                DevicesData.Temperature,
                DevicesData.Humidity,
                DevicesData.Light,
                DevicesData.Body,
                DevicesData.Fire,
                DevicesData.NaturalGas,
                DevicesData.Fan,
                DevicesData.Lamp);
        WebServer.send(200, CONTENT_TYPE_JSON, buf);
    });
    /* 灯光控制API */
    MapUri("/api/dev/lamp-con", HTTP_GET)
    {
        String value = WebServer.arg("value");
        int cmd = value.equals("1") ? 1 : 0;
        Lamp_Control(cmd);
        Serial.printf("灯光API: %s Lamp=%d\n", cmd ? "开" : "关", DevicesData.Lamp);
        WebServer.send(200);
    });
    /* 排风扇控制API */
    MapUri("/api/dev/fan-con", HTTP_GET)
    {
        String value = WebServer.arg("value");
        int cmd = value.equals("1") ? 1 : 0;
        Fan_Control(cmd);
        Serial.printf("排风扇API: %s  Fan=%d\n", cmd ? "开" : "关", DevicesData.Fan);
        WebServer.send(200);
    });
    /* 设置应用程序配置 */
    MapUri("/api/app/set-config", HTTP_GET)
    {
        int lamp_on_light = WebServer.arg("lamp_on_light").toInt();
        int lamp_off_light = WebServer.arg("lamp_off_light").toInt();
        int on_lamp_must_has_body = WebServer.arg("on_lamp_must_has_body").toInt();
        int no_body_close_lamp_time = WebServer.arg("no_body_close_lamp_time").toInt();
        int natural_gas_alarm = WebServer.arg("natural_gas_alarm").toInt();
        String alarm_email = WebServer.arg("alarm_email");

        SystemConfig["lamp_on_light"] = lamp_on_light;
        SystemConfig["lamp_off_light"] = lamp_off_light;
        SystemConfig["on_lamp_must_has_body"] = on_lamp_must_has_body;
        SystemConfig["no_body_close_lamp_time"] = no_body_close_lamp_time;
        SystemConfig["natural_gas_alarm"] = natural_gas_alarm;
        SystemConfig["alarm_email"] = alarm_email;

        Serial.printf("设置应用程序配置API: "
                      "\n\t自动开灯光照值=%d"
                      "\n\t自动关灯光照值=%d"
                      "\n\t开灯时必须有人=%d"
                      "\n\t无人自动关灯延时=%d"
                      "\n\t天然气报警阈值=%d"
                      "\n\t报警通知邮箱地址=%s\n",
                      lamp_on_light,
                      lamp_off_light,
                      on_lamp_must_has_body,
                      no_body_close_lamp_time,
                      natural_gas_alarm,
                      alarm_email.c_str());
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
        SystemConfig_Save();
    });
    /* 获取应用程序配置 */
    MapUri("/api/app/get-config", HTTP_GET)
    {

        char buf[1024];
        int lamp_on_light = SystemConfig["lamp_on_light"].as<int>();
        int lamp_off_light = SystemConfig["lamp_off_light"].as<int>();
        int on_lamp_must_has_body = SystemConfig["on_lamp_must_has_body"].as<int>();
        int no_body_close_lamp_time = SystemConfig["no_body_close_lamp_time"].as<int>();
        int natural_gas_alarm = SystemConfig["natural_gas_alarm"].as<int>();
        String alarm_email = SystemConfig["alarm_email"];
        sprintf(buf, "{\"lamp_on_light\":%d,\"lamp_off_light\":%d,\"on_lamp_must_has_body\":%d,\"no_body_close_lamp_time\":%d,\"natural_gas_alarm\":%d,\"alarm_email\":\"%s\"}",
                lamp_on_light,
                lamp_off_light,
                on_lamp_must_has_body,
                no_body_close_lamp_time,
                natural_gas_alarm,
                alarm_email.c_str());
        Serial.printf("获取应用程序配置API: "
                      "\n\t自动开灯光照值=%d"
                      "\n\t自动关灯光照值=%d"
                      "\n\t开灯时必须有人=%d"
                      "\n\t无人自动关灯延时=%d"
                      "\n\t天然气报警阈值=%d"
                      "\n\t报警通知邮箱地址=%s\n",
                      lamp_on_light,
                      lamp_off_light,
                      on_lamp_must_has_body,
                      no_body_close_lamp_time,
                      natural_gas_alarm,
                      alarm_email.c_str());
        WebServer.send(200, CONTENT_TYPE_JSON, buf);
    });
    /* 安全认证API 认证完成后跳转主页*/
    MapUri("/api/admin/auth", HTTP_GET)
    {

        WebServerCheckAuth();
        WebServer.sendHeader("Location", "/main.html.gz");
        WebServer.send(303);
    });
    /* 系统重启API */
    MapUri("/api/sys/restart", HTTP_GET)
    {
        Serial.println("系统重启API: 重启中...");
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
        system_restart();
    });
    /* 系统重置API */
    MapUri("/api/sys/restore", HTTP_GET)
    {
        Serial.println("系统重置API: 重置中...");
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
        LittleFS.remove(SYSTEM_CONFIG_PATH);
        system_restart();
    });
    /* WiFi 连接API */
    MapUri("/api/wifi/connect", HTTP_GET)
    {
        String ssid = WebServer.arg("ssid");
        String password = WebServer.arg("password");
        Serial.printf("WiFi连接API: SSID=%s  密码=%s\n\t连接中", ssid.c_str(), password.c_str());
        WiFi.begin(ssid, password);
        uint32_t wifiConnectStartTime = system_get_time();
        while (WiFi.status() != WL_CONNECTED)
        {
            if (system_get_time() - wifiConnectStartTime > WIFI_CONNECT_TIME_OUT)
            {
                Serial.println("WiFi连接API: 连接超时！");
                WiFi.setAutoConnect(false);
                WiFi.setAutoReconnect(false);
                WebServer.send(200, CONTENT_TYPE_JSON, "{\"code\":-1,\"msg\": \"连接超时！\"}");
                return;
            }
            Serial.print(".");
            delay(500);
        }
        WiFi.setAutoConnect(true);
        WiFi.setAutoReconnect(true);
        Serial.printf("\nWiFi连接API: 连接成功 IP=%s\n", WiFi.localIP().toString().c_str());
        SystemConfig["wifi_ssid"] = ssid;
        SystemConfig["wifi_password"] = password;
        SystemConfig_Save();
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
    });
    // /* WiFi 连接信息保存 API*/
    // MapUri("/api/wifi/save-con-info", HTTP_GET)
    // {
    //     String ssid = WebServer.arg("ssid");
    //     String password = WebServer.arg("password");
    //     SystemConfig["wifi_ssid"] = ssid;
    //     SystemConfig["wifi_password"] = password;
    //     Serial.printf("保存WiFi连接配置 SSID: %s  Pwd: %s\n", ssid.c_str(), password.c_str());
    //     SystemConfig_Save();
    //     WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
    // });
    /* 设置静态IP地址API */
    MapUri("/api/wifi/set-static-ip", HTTP_GET)
    {
        String ip = WebServer.arg("ip");
        String netmask = WebServer.arg("netmask");
        String gateway = WebServer.arg("gateway");
        Serial.printf("设置静态IP: IP=%s  MASK=%s GT=%s\n", ip.c_str(), netmask.c_str(), gateway.c_str());
        SystemConfig["wifi_sta_ip"] = ip;
        SystemConfig["wifi_sta_mask"] = netmask;
        SystemConfig["wifi_sta_gt"] = gateway;
        SystemConfig_Save();
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
    });
    /* 获取STA IP地址信息API */
    MapUri("/api/wifi/get-ip-info", HTTP_GET)
    {
        String localIp = WiFi.localIP().toString();
        String netmask = WiFi.subnetMask().toString();
        String gateway = WiFi.gatewayIP().toString();
        Serial.printf("获取IP信息API: IP=%s  MASK=%s  GT=%s\n", localIp.c_str(), netmask.c_str(), gateway.c_str());
        char buf[100];
        ;
        sprintf(buf, "{\"ip\":\"%s\",\"netmask\":\"%s\",\"gateway\":\"%s\"}",
                localIp.c_str(), netmask.c_str(), gateway.c_str());
        WebServer.send(200, CONTENT_TYPE_JSON, buf);
    });
    /* WiFi扫描API */
    MapUri("/api/wifi/scan", HTTP_GET)
    {
        Serial.println("WiFi扫描API: 开始扫描中...");
        int count = WiFi.scanNetworks();
        char buf[1024];
        char *bufPtr = buf;
        *(bufPtr++) = '[';
        for (int i = 0; i < count; i++)
        {
            bufPtr += sprintf(bufPtr, "{\"ssid\":\"%s\",\"rssi\":%d},",
                              WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            Serial.printf("\t%s   (%d)\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        }

        Serial.printf("WiFi扫描API: 扫描结束，共%d个\n", count);
        if (*(bufPtr - 1) == ',')
            *(bufPtr - 1) = ']';
        else
        {
            *bufPtr = ']';
            *(++bufPtr) = '\0';
        }
        WebServer.send(200, CONTENT_TYPE_JSON, buf);
    });
    /*文件上传页面*/
    MapUri("/fsup", HTTP_GET)
    {
        WebServer.send(200, CONTENT_TYPE_HTML,
                       "<form action=\"/api/fsup\" method=\"POST\" enctype=\"multipart/form-data\">"
                       "<input type=\"file\" name=\"mufile\">"
                       "<input type=\"submit\" name=\"mufile\">"
                       "</form>");
    });
    /*文件上传API*/
    MapUri("/api/fsup", HTTP_POST)
    {
        WebServer.send(200, CONTENT_TYPE_TEXT, "文件上传成功！");
    },WebApi_File_Upload);

    /*文件下载API*/
    WebServer.onNotFound(WebApi_File_Download);

    /*启动服务器*/
    WebServer.begin();
    WebServer.keepAlive(true);
    Serial.println("Web服务器初始化完成...");
}

String getContentTypeByFileName(String filename)
{
    if (filename.endsWith(".html") || filename.endsWith(".html.gz"))
        return "text/html;charset=UTF-8";
    if (filename.endsWith(".css") || filename.endsWith(".css.gz"))
        return "text/css;charset=UTF-8";
    if (filename.endsWith(".js") || filename.endsWith(".js.gz"))
        return "text/javascript;charset=UTF-8";
    return "text/plan;charset=UTF-8";
}
File fsUploadFile;
void WebApi_File_Download()
{
    WebServer.keepAlive(false);
    String uri = WebServer.uri();
    if (!LittleFS.exists(uri))
    {
        Serial.printf("文件下载-404: 文件 %s 不存在\n", uri.c_str());
        WebServer.send(404, CONTENT_TYPE_TEXT, "文件不存在！");
        return;
    }
    File downloadFile = LittleFS.open(uri, "r");
    Serial.printf("文件下载-开始: %s  发送中...\n", uri.c_str());

    size_t sendSize = WebServer.streamFile(downloadFile, getContentTypeByFileName(uri));
    if (sendSize == downloadFile.size())
    {
        Serial.printf("文件下载-成功: %s  大小:%d\n", uri.c_str(),sendSize);
    }
    else
    {
        Serial.printf("文件下载-异常: %s  发送失败！重试中...\n", uri.c_str());
        size_t sendSize = WebServer.streamFile(downloadFile, getContentTypeByFileName(uri));
        if (sendSize != downloadFile.size())
        {
            Serial.printf("文件下载-失败: %s  重试发送失败！", uri.c_str(), sendSize);
        }
    }
    downloadFile.close();
}

void WebApi_File_Upload()
{
    HTTPUpload &upload = WebServer.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        // 如果上传状态为UPLOAD_FILE_START
        String filename = upload.filename;
        // 建立字符串变量用于存放上传文件名
        if (!filename.startsWith("/"))
            filename = "/" + filename;
        // 为上传文件名前加上 "/"
        Serial.printf("文件上传-开始: %s\n", filename.c_str());
        // 通过串口监视器输出上传文件的名称
        fsUploadFile = LittleFS.open(filename, "w");
        // 在SPIFFS中建立文件用于写入用户上传的文件数据
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        // 如果上传状态为UPLOAD_FILE_WRITE
        if (fsUploadFile)
            // 向SPIFFS文件写入浏览器发来的文件数据
            fsUploadFile.write(upload.buf, upload.currentSize);

        Serial.printf("文件上传-写入: %d 字节\n", upload.currentSize);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        // 如果上传状态为UPLOAD_FILE_END
        if (fsUploadFile)
        {
            // 如果文件成功建立
            fsUploadFile.close();
            // 将文件关闭
            Serial.printf("文件上传-完成: 大小: %d 字节\n", upload.totalSize);
            WebServer.send(200);
        }
        else
        {
            // 如果文件未能成功建立
            Serial.println("文件上传-失败: 文件创建失败！");
            // 通过串口监视器输出报错信息
            WebServer.send(500, "text/plain", "500 文件创建失败！");
        }
    }
}