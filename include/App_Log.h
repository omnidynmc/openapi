#ifndef OPENAPI_APP_LOG_H
#define OPENAPI_APP_LOG_H

#include <fcgi_stdio.h>

#include "App.h"
#include "OpenAPI_Log.h"
#include "SyslogController.h"

namespace openapi {

#define OA_LOG_EMERG	SyslogController::SYSCLOG_EMERG
#define OA_LOG_ALERT	SyslogController::SYSCLOG_INFO
#define OA_LOG_CRIT	SyslogController::SYSCLOG_CRIT
#define OA_LOG_ERR	SyslogController::SYSCLOG_ERR
#define OA_LOG_WARNING	SyslogController::SYSCLOG_WARNING
#define OA_LOG_NOTICE	SyslogController::SYSCLOG_NOTICE
#define OA_LOG_INFO	SyslogController::SYSCLOG_INFO
#define OA_LOG_DEBUG	SyslogController::SYSCLOG_DEBUG

  class App_Log : public OpenAPI_Log {
    public:
      App_Log(const std::string &ident) : OpenAPI_Log(ident) { }
      virtual ~App_Log() { }

      virtual void log(const std::string &message) {
        app->syslog()->log(OA_LOG_INFO, message);
      } // log

      virtual void emerg(const std::string &message) {
        app->syslog()->log(OA_LOG_EMERG, message);
      } // emerg

      virtual void alert(const std::string &message) {
        app->syslog()->log(OA_LOG_ALERT, message);
      } // alert

      virtual void crit(const std::string &message) {
        app->syslog()->log(OA_LOG_CRIT, message);
      } // crit

      virtual void error(const std::string &message) {
        app->syslog()->log(OA_LOG_ERR, message);
      } // err

      virtual void warn(const std::string &message) {
        app->syslog()->log(OA_LOG_WARNING, message);
      } // warning

      virtual void notice(const std::string &message) {
        app->syslog()->log(OA_LOG_NOTICE, message);
      } // notice

      virtual void info(const std::string &message) {
        app->syslog()->log(OA_LOG_INFO, message);
      } // info

      virtual void debug(const std::string &message) {
        app->syslog()->log(OA_LOG_DEBUG, message);
      } // debug

      virtual void console(const std::string &message) {
        std::cout << message << std::endl;
      } // console

      virtual void fcgi(const std::string &message) {
        FCGI_printf("%s", message.c_str());
      } // log

    protected:
    private:

  }; // OpenAPI_Log

} // openapi

#endif
