#include <fstream>
#include <string>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <list>
#include <map>
#include <new>
#include <iostream>
#include <fstream>

#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>

#include "SyslogController.h"
#include "OpenAPI_Abstract.h"

namespace openapi {
  using namespace std;

/**************************************************************************
 ** SyslogController Class                                            **
 **************************************************************************/

  const SyslogController::logFacilityType SyslogController::SYSCLOG_AUTH		=	LOG_AUTH;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_AUTHPRIV		=	LOG_AUTHPRIV;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_CRON		=	LOG_CRON;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_DAEMON		=	LOG_DAEMON;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_FTP			=	LOG_FTP;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_KERN		=	LOG_KERN;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_LOCAL0		=	LOG_LOCAL0;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_LOCAL1		=	LOG_LOCAL1;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_LOCAL2		=	LOG_LOCAL2;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_LOCAL3		=	LOG_LOCAL3;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_LOCAL4		=	LOG_LOCAL4;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_LOCAL5		=	LOG_LOCAL5;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_LOCAL6		=	LOG_LOCAL6;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_LOCAL7		=	LOG_LOCAL7;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_LPR			=	LOG_LPR;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_MAIL		=	LOG_MAIL;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_NEWS		=	LOG_NEWS;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_USER		=	LOG_USER;
  const SyslogController::logFacilityType SyslogController::SYSCLOG_UUCP		=	LOG_UUCP;

  const SyslogController::logLevelType SyslogController::SYSCLOG_EMERG			=	LOG_EMERG;
  const SyslogController::logLevelType SyslogController::SYSCLOG_ALERT			=	LOG_ALERT;
  const SyslogController::logLevelType SyslogController::SYSCLOG_CRIT			=	LOG_CRIT;
  const SyslogController::logLevelType SyslogController::SYSCLOG_ERR			=	LOG_ERR;
  const SyslogController::logLevelType SyslogController::SYSCLOG_WARNING		=	LOG_WARNING;
  const SyslogController::logLevelType SyslogController::SYSCLOG_NOTICE			=	LOG_NOTICE;
  const SyslogController::logLevelType SyslogController::SYSCLOG_INFO			=	LOG_INFO;
  const SyslogController::logLevelType SyslogController::SYSCLOG_DEBUG			=	LOG_DEBUG;

  const SyslogController::logOptionType SyslogController::SYSCLOG_CONS			= 	LOG_CONS;
  const SyslogController::logOptionType SyslogController::SYSCLOG_NDELAY		=	LOG_NDELAY;
  const SyslogController::logOptionType SyslogController::SYSCLOG_NOWAIT		=	LOG_NOWAIT;
  const SyslogController::logOptionType SyslogController::SYSCLOG_ODELAY		=	LOG_ODELAY;
  const SyslogController::logOptionType SyslogController::SYSCLOG_PERROR		=	LOG_PERROR;
  const SyslogController::logOptionType SyslogController::SYSCLOG_PID			=	LOG_PID;

  const size_t SyslogController::SYSCLOG_MAXBUF						= 	4096;

  SyslogController::SyslogController(const string &ident, const logOptionType options, const logFacilityType facility) :
                                     _facility(facility), _options(options), _ident(ident) {
    open();
  } // SyslogController::SyslogController

  SyslogController::SyslogController(const string &ident, const logOptionType options, const string &facility) :
                                     _options(options), _ident(ident) {
    const char *names[] = { "auth", "authpriv", "cron", "daemon", "ftp", "kern",
                                 "local0", "local1", "local2", "local3", "local4", "local5",
                                 "local6", "local7", "lpr", "mail", "news",
                                 "user", "uucp", NULL };
    const logFacilityType values[] = { SYSCLOG_AUTH, SYSCLOG_AUTHPRIV, SYSCLOG_CRON, SYSCLOG_DAEMON, SYSCLOG_FTP,
                                       SYSCLOG_KERN, SYSCLOG_LOCAL0, SYSCLOG_LOCAL1, SYSCLOG_LOCAL2,
                                       SYSCLOG_LOCAL3, SYSCLOG_LOCAL4, SYSCLOG_LOCAL5, SYSCLOG_LOCAL6,
                                       SYSCLOG_LOCAL7, SYSCLOG_LPR, SYSCLOG_MAIL, SYSCLOG_NEWS,
                                       SYSCLOG_USER, SYSCLOG_UUCP, -1 };
    int facit = -1;

    for(size_t i=0; names[i] != NULL; i++) {
      if (strcasecmp(facility.c_str(), names[i]) == 0) {
        facit = values[i];
        break;
      } // if
    } // for

    if (facit == -1)
      throw SyslogController_Exception("invalid facility");

    _facility = facit;

    open();
  } // SyslogController::SyslogController

  SyslogController::~SyslogController() {
    close();
  } // SyslogController::~SyslogController

  void SyslogController::open() {
    openlog(_ident.c_str(), _options, _facility);
  } // SyslogController::open

  void SyslogController::close() {
    closelog();
  } // SyslogController::close

  void SyslogController::setlogmask(const int level) {
    int upto[] = { SYSCLOG_EMERG, SYSCLOG_ALERT, SYSCLOG_CRIT, SYSCLOG_ERR,
                   SYSCLOG_WARNING, SYSCLOG_NOTICE, SYSCLOG_INFO, SYSCLOG_DEBUG,
                   0 };

    if (level < 0 || level > 7)
      throw SyslogController_Exception("mask level not between 0-7");

    ::setlogmask(LOG_UPTO(level));
  } // SyslogController::setlogmask

  void SyslogController::log(const logLevelType level, const string &buf) {
    string prefix = "";

    switch(level) {
      case SYSCLOG_EMERG:
        prefix = "EMERGENCY";
        break;
      case SYSCLOG_ALERT:
        prefix = "ALERT";
        break;
      case SYSCLOG_CRIT:
        prefix = "CRITICAL";
        break;
      case SYSCLOG_ERR:
        prefix = "ERROR";
        break;
      case SYSCLOG_WARNING:
        prefix = "WARNING";
        break;
      case SYSCLOG_NOTICE:
        prefix = "NOTICE";
        break;
      case SYSCLOG_INFO:
        prefix = "INFO";
        break;
      case SYSCLOG_DEBUG:
        prefix = "DEBUG";
        break;
      default:
        assert(false);		// bug
        break;
    } // switch

    syslog(level, "%s: %s", prefix.c_str(), buf.c_str());
  } // SyslogController::log

  void SyslogController::logf(const logLevelType level, const char *writeFormat, ...) {
    char writeBuffer[SYSCLOG_MAXBUF] = {0};
    va_list va;

    va_start(va, writeFormat);
    vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
    va_end(va);

    log(level, writeBuffer);
  } // _debugf


} // namespace openapi
