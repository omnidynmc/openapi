/**************************************************************************
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                                                                      **
 ** This program is free software; you can redistribute it and/or modify **
 ** it under the terms of the GNU General Public License as published by **
 ** the Free Software Foundation; either version 1, or (at your option)  **
 ** any later version.                                                   **
 **                                                                      **
 ** This program is distributed in the hope that it will be useful,      **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of       **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with this program; if not, write to the Free Software          **
 ** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            **
 **************************************************************************
 $Id$
 **************************************************************************/

#ifndef OPENAPI_SYSLOGCONTROLLER_H
#define OPENAPI_SYSLOGCONTROLLER_H

#include <string>

#include <openframe/openframe.h>

#include "OpenAPI_Abstract.h"

namespace openapi {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

class SyslogController_Exception : public OpenAPI_Exception {
  public:
    SyslogController_Exception(const std::string message) throw() : OpenAPI_Exception(message) {
    } // OpenAbstract_Exception

  private:
}; // class SyslogController_Exception

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class SyslogController : public OpenAPI_Abstract {
  public:
    // ### Type Definitions ###
    typedef int logFacilityType;
    typedef int logLevelType;
    typedef int logOptionType;

    SyslogController(const std::string &, const logOptionType, const logFacilityType);
    SyslogController(const std::string &, const logOptionType, const std::string &);
    virtual ~SyslogController();

    static const logFacilityType SYSCLOG_AUTH;
    static const logFacilityType SYSCLOG_AUTHPRIV;
    static const logFacilityType SYSCLOG_CRON;
    static const logFacilityType SYSCLOG_DAEMON;
    static const logFacilityType SYSCLOG_FTP;
    static const logFacilityType SYSCLOG_KERN;
    static const logFacilityType SYSCLOG_LOCAL0;
    static const logFacilityType SYSCLOG_LOCAL1;
    static const logFacilityType SYSCLOG_LOCAL2;
    static const logFacilityType SYSCLOG_LOCAL3;
    static const logFacilityType SYSCLOG_LOCAL4;
    static const logFacilityType SYSCLOG_LOCAL5;
    static const logFacilityType SYSCLOG_LOCAL6;
    static const logFacilityType SYSCLOG_LOCAL7;
    static const logFacilityType SYSCLOG_LPR;
    static const logFacilityType SYSCLOG_MAIL;
    static const logFacilityType SYSCLOG_NEWS;
    static const logFacilityType SYSCLOG_USER;
    static const logFacilityType SYSCLOG_UUCP;

    static const logLevelType SYSCLOG_EMERG;
    static const logLevelType SYSCLOG_ALERT;
    static const logLevelType SYSCLOG_CRIT;
    static const logLevelType SYSCLOG_ERR;
    static const logLevelType SYSCLOG_WARNING;
    static const logLevelType SYSCLOG_NOTICE;
    static const logLevelType SYSCLOG_INFO;
    static const logLevelType SYSCLOG_DEBUG;

    static const logOptionType SYSCLOG_CONS;
    static const logOptionType SYSCLOG_NDELAY;
    static const logOptionType SYSCLOG_NOWAIT;
    static const logOptionType SYSCLOG_ODELAY;
    static const logOptionType SYSCLOG_PERROR;
    static const logOptionType SYSCLOG_PID;

    static const size_t SYSCLOG_MAXBUF;

    // ### Members ###
    void open();
    void close();
    void setlogmask(const logLevelType);
    void logf(const logLevelType, const char *, ...);
    void log(const logLevelType, const std::string &);

    // ### Variables ###

  protected:
  private:
    // ### Variables ###
    logFacilityType _facility;
    logOptionType _options;
    std::string _ident;
}; // SyslogController

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openapi
#endif
