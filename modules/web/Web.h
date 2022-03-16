#ifndef MODULE_WEB_CLASS_WEB_H
#define MODULE_WEB_CLASS_WEB_H

#include <list>
#include <string>

#include "App_Log.h"
#include "OpenAPI_Abstract.h"

namespace modweb {
/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  class DBI_Web;

  class Web : public openapi::OpenAPI_Abstract {
    public:
      Web();
      virtual ~Web();

      // ### Type Definitions ###

      static const int DEFAULT_RESULT_LIMIT;
      static const int DEFAULT_WONTTRACK_LIMIT;
      static const char *DEFAULT_AGE;
      static const char *DEFAULT_PATH_HTML;
      static const char *DEFAULT_FILE_BALLOON;
      static const char *DEFAULT_FILE_HNYBALLOON;

      // ### Public Members ###
      void initializeSystem();
      void initializeCommands();
      void initializeDatabase();
      void deinitializeSystem();
      void deinitializeCommands();
      void deinitializeDatabase();
      const bool done() const { return _done; }
      void done(const bool done) { _done = done; }
      DBI_Web *dbi() { return _dbi_w; }
      DBI_Web *dbi_w() { return _dbi_w; }
      DBI_Web *dbi_r() { return _dbi_r; }

    protected:
    private:
      DBI_Web *_dbi_w;
      DBI_Web *_dbi_r;
      bool _done;
  }; // OpenAPI_Abstract

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace modweb
extern "C" {
  extern modweb::Web *web;
} // extern "C"
#endif
