#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__
#include<ESP8266WebServer.h>
#include"base_sys.h"
extern ESP8266WebServer WebServer;
void WebServer_Init();

extern const char *CONTENT_TYPE_TEXT;
extern const char *CONTENT_TYPE_HTML;
extern const char *CONTENT_TYPE_JSON;

extern const char *WEB_RESULT_SUCCESS_JSON;
extern const char *WEB_RESULT_ERROR_TEMPLATE_JSON;

#define WebServerCheckAuth()                                                                                         \
    if (!WebServer.authenticate((const char *)SystemConfig["admin_account"], (const char *)SystemConfig["admin_password"])) \
        return WebServer.requestAuthentication();
#endif