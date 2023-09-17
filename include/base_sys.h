#ifndef __BASE_SYS_H__
#define __BASE_SYS_H__

#include <ArduinoJson.h>

/* 全局配置文件保存路径 */
#define SYSTEM_CONFIG_PATH "/v1-config.json"
/* 全局配置文件版本 */
#define CONFIG_FILE_VERSION 1
/* 默认的全局配置文件 */
#define DEFAULT_SYSYTEM_CONFIG_CONTENT                    \
    "{"                                                   \
    "\"version\": 1,"                                     \
    "\"admin_account\": \"admin\","                       \
    "\"admin_password\": \"02130213\","                   \
    "\"wifi_ssid\": \".v\","                            \
    "\"wifi_password\": \"02130213\","                    \
    "\"wifi_sta_ip\": null,"                              \
    "\"wifi_sta_mask\": null,"                            \
    "\"wifi_sta_gt\": null,"                              \
    "\"wifi_ap_ssid\": \"GDW-厨房环境监测系统\"," \
    "\"wifi_ap_password\": \"02130213\"," \
    "\"lamp_on_light\": 10,"                              \
    "\"lamp_off_light\": 500,"                            \
    "\"on_lamp_must_has_body\": 1,"                            \
    "\"no_body_close_lamp_time\": 2000,"                  \
    "\"natural_gas_alarm\": 400,"                         \
    "\"alarm_email\": null"                               \
    "}";

/* 全局 Json 配置文件 */
extern DynamicJsonDocument SystemConfig;
/* 配置文件缓存 */
extern char SystemConfigBuf[];

void FileSystem_Init();
void SystemConfig_Init();
void SystemConfig_Save();
void Network_Init();

#define WIFI_CONNECT_TIME_OUT 10000000

#endif