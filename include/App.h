#ifndef OPENAPI_APP_H
#define OPENAPI_APP_H

#include <list>
#include <string>
#include <map>

#include <openframe/openframe.h>
#include <openframe/App/Application.h>
#include <stomp/StompStats.h>

#include "OpenAPI_Abstract.h"
#include "SyslogController.h"

namespace openapi {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  class App_Exception : public openframe::OpenFrame_Exception {
    public:
      App_Exception(const std::string message) throw() : openframe::OpenFrame_Exception(message) {
      } // OpenAbstract_Exception

   private:
  }; // class App_Exception

  class DBI;
  class App : public openframe::App::Application,
              public OpenAPI_Abstract {
    public:
      typedef openframe::App::Application super;

      static const char *kPidFile;

      App(const std::string &prompt, const std::string &config);
      virtual ~App();

      // ### Type Definitions ###
      typedef std::map<string, string> envMapType;

      // ### Public Members ###

      DBI *dbi() { return _dbi_w; }
      DBI *dbi_w() { return _dbi_w; }
      DBI *dbi_r() { return _dbi_r; }

      SyslogController *syslog() { return _syslog; }

      // ### Public Members ###
      stomp::StompStats *stats() { return _stats; }

      // ### App_Abstract Virtuals ###
      void onInitializeSystem();
      void onInitializeConfig();
      void onInitializeCommands();
      void onInitializeDatabase();
      void onInitializeModules();
      void onInitializeThreads();

      void onDeinitializeSystem();
      void onDeinitializeCommands();
      void onDeinitializeDatabase();
      void onDeinitializeModules();
      void onDeinitializeThreads();

      void rcvSighup();
      void rcvSigusr1();
      void rcvSigusr2();
      void rcvSigint();
      void rcvSigpipe();

      bool onRun();

      bool is_debug() const { return _debug; }
      void set_debug(const bool debug) { _debug = debug; }

      // ### Public Variables ###

    protected:
      void _accept();
      const unsigned int _parse(const std::string &);
      const unsigned int _loadModules();

    private:
      bool _debug;

      DBI *_dbi_w;
      DBI *_dbi_r;
      SyslogController *_syslog;
      envMapType _env;
      unsigned int _numRequests;

      stomp::StompStats *_stats;
  }; // OpenAPI_Abstract

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // openapi

extern openapi::App *app;
extern openframe::Logger elog;
#endif
