#ifndef OPENAPI_OPENAPI_ABSTRACT_H
#define OPENAPI_OPENAPI_ABSTRACT_H

#include <string>

#include <stdio.h>
#include <stdarg.h>

#include "OpenAPI_Log.h"

namespace openapi {
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define OPENAPILOG_MAXBUF	1024

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class OpenAPI_Exception : public std::exception {
    public:
      OpenAPI_Exception(const string message) throw() {
        if (!message.length())
          _message = "An unknown message exception occured.";
        else
          _message = message;
      } // OpenAbstract_Exception

      virtual ~OpenAPI_Exception() throw() { }
      virtual const char *what() const throw() { return _message.c_str(); }

      const char *message() const throw() { return _message.c_str(); }

    private:
      string _message;                    // Message of the exception error.
  }; // class OpenAPI_Exception

  class OpenAPI_Abstract {
    public:
      OpenAPI_Abstract(OpenAPI_Log *logger) : _logger(logger) { }
      OpenAPI_Abstract() { _logger = new OpenAPI_Log("Abstract"); }
      virtual ~OpenAPI_Abstract() {
        if (_logger != NULL)
          delete _logger;
      } // OpenAPI_Abstract

      void logger(OpenAPI_Log *logger) {
        if (_logger != NULL)
          delete _logger;
        _logger = logger;
      } // logger

    protected:
      void _logf(const char *writeFormat, ...) {
        char writeBuffer[OPENAPILOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _log(writeBuffer);
      } // log

      void _emergf(const char *writeFormat, ...) {
        char writeBuffer[OPENAPILOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _emerg(writeBuffer);
      } // emerg

      void _alertf(const char *writeFormat, ...) {
        char writeBuffer[OPENAPILOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _alert(writeBuffer);
      } // alertf

      void _critf(const char *writeFormat, ...) {
        char writeBuffer[OPENAPILOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _crit(writeBuffer);
      } // critf

      void _errorf(const char *writeFormat, ...) {
        char writeBuffer[OPENAPILOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _error(writeBuffer);
      } // errf

      void _warnf(const char *writeFormat, ...) {
        char writeBuffer[OPENAPILOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _warn(writeBuffer);
      } // warnf

      void _noticef(const char *writeFormat, ...) {
        char writeBuffer[OPENAPILOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _notice(writeBuffer);
      } // noticef

      void _infof(const char *writeFormat, ...) {
        char writeBuffer[OPENAPILOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _info(writeBuffer);
      } // infof

      void _debugf(const char *writeFormat, ...) {
        char writeBuffer[OPENAPILOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _debug(writeBuffer);
      } // _debugf

      void _consolef(const char *writeFormat, ...) {
        char writeBuffer[OPENAPILOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _console(writeBuffer);
      } // _consolef

      void _fcgif(const char *writeFormat, ...) {
        char writeBuffer[OPENAPILOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _fcgi(writeBuffer);
      } // _fcgif

      virtual void _log(const string &message) {
        _logger->log(message);
      } // _log

      virtual void _emerg(const string &message) {
        _logger->emerg(message);
      } // _emerg

      virtual void _alert(const string &message) {
        _logger->alert(message);
      } // _alert

      virtual void _crit(const string &message) {
        _logger->crit(message);
      } // _crit

      virtual void _error(const string &message) {
        _logger->error(message);
      } // _err

      virtual void _warn(const string &message) {
        _logger->warn(message);
      } // _warn

      virtual void _notice(const string &message) {
        _logger->notice(message);
      } // _notice

      virtual void _info(const string &message) {
        _logger->info(message);
      } // _info

      virtual void _debug(const string &message) {
        _logger->debug(message);
      } // _debug

      virtual void _console(const string &message) {
        _logger->console(message);
      } // _console

      virtual void _fcgi(const string &message) {
        _logger->fcgi(message);
      } // _fcgi

      // ### Variables ###
      OpenAPI_Log *_logger;

    private:

  }; // OpenAPI_Abstract

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/


} // openapi

#endif
