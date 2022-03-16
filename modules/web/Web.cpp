#include <config.h>

#include <new>
#include <string>

#include <stdarg.h>
#include <stdio.h>

#include <openframe/openframe.h>

#include "App.h"
#include "App_Log.h"
#include "Web.h"
#include "DBI_Web.h"

#include "web.h"
#include "openapi.h"

#include "webCommands.h"

namespace modweb {
  const int Web::DEFAULT_RESULT_LIMIT			= 200;
  const int Web::DEFAULT_WONTTRACK_LIMIT		= 100;
  const char *Web::DEFAULT_AGE				= "1 HOUR";
  const char *Web::DEFAULT_PATH_HTML			= "html/";
  const char *Web::DEFAULT_FILE_BALLOON			= "balloon.html";
  const char *Web::DEFAULT_FILE_HNYBALLOON		= "hnyballoon.html";

  Web::Web() {
    _dbi_r = NULL;
    _dbi_w = NULL;
  } // Web::Web

  Web::~Web() {
  } // Web::~Web

  void Web::initializeSystem() {
    initializeCommands();
    initializeDatabase();
    _infof("*** Module Web: System Initialized");
  } // Web::initializeSystem

  void Web::initializeCommands() {
    CommandTree *command = app->command();

    command->addCommand("BALLOON", "FCGI", "BALLOON", -1, Command::FLAG_CLEANUP, new fcgiBALLOON);
    command->addCommand("CHAT", "FCGI", "CHAT", -1, Command::FLAG_CLEANUP, new fcgiCHAT);
    command->addCommand("HNYBALLOON", "FCGI", "HNYBALLOON", -1, Command::FLAG_CLEANUP, new fcgiHNYBALLOON);
    command->addCommand("LASTPOSITION", "FCGI", "LASTPOSITION", -1, Command::FLAG_CLEANUP, new fcgiLASTPOSITION);
/*
      command->addCommand("ACTIVATE", "STDIN/GSMA", "/GSMA ACTIVATE <signature id>", -1, Command::FLAG_CLEANUP, new stdinACTIVATE);
      command->addCommand("CLEAR", "STDIN/GSMA", "/GSMA CLEAR", -1, Command::FLAG_CLEANUP, new stdinDTREE);
        command->addCommand("CLUSTERS", "STDIN/GSMA/CLEAR", "/GSMA CLEAR CLUSTERS - clears clusters and signature associations with clusters", -1, Command::FLAG_CLEANUP, new stdinCLEARCLUSTERS);
        command->addCommand("GSMA", "STDIN/GSMA/CLEAR", "/GSMA CLEAR GMSA - clears gsma database", -1, Command::FLAG_CLEANUP, new stdinCLEARGSMA);
        command->addCommand("SIGNATURES", "STDIN/GSMA/CLEAR", "/GSMA CLEAR SIGNATURES - clears signatures out of gsma database", -1, Command::FLAG_CLEANUP, new stdinCLEARSIGNATURES);
      command->addCommand("DEACTIVATE", "STDIN/GSMA", "/GSMA DEACTIVATE <signature id>", -1, Command::FLAG_CLEANUP, new stdinDEACTIVATE);
      command->addCommand("GENSIGS", "STDIN/GSMA", "/GSMA GENSIGS <spam message id>", -1, Command::FLAG_CLEANUP, new stdinGENSIGS);
      command->addCommand("MAKEATTACK", "STDIN/GSMA", "/GSMA MAKEATTACK", -1, Command::FLAG_CLEANUP, new stdinMAKEATTACK);
      command->addCommand("SHOW", "STDIN/GSMA", "/GSMA SHOW", -1, Command::FLAG_CLEANUP, new stdinDTREE);
        command->addCommand("CLASS", "STDIN/GSMA/SHOW", "/GSMA SHOW CLASS <name>", -1, Command::FLAG_CLEANUP, new stdinSHOWCLASS);
        command->addCommand("CLUSTER", "STDIN/GSMA/SHOW", "/GSMA SHOW CLUSTER <id>", -1, Command::FLAG_CLEANUP, new stdinSHOWCLUSTER);
        command->addCommand("CLUSTERS", "STDIN/GSMA/SHOW", "/GSMA SHOW CLUSTERS", -1, Command::FLAG_CLEANUP, new stdinSHOWCLUSTERS);
        command->addCommand("REPORTER", "STDIN/GSMA/SHOW", "/GSMA SHOW REPORTER <id>", -1, Command::FLAG_CLEANUP, new stdinSHOWREPORTER);
        command->addCommand("SPAM", "STDIN/GSMA/SHOW", "/GSMA SHOW SPAM <id>", -1, Command::FLAG_CLEANUP, new stdinSHOWSPAM);
        command->addCommand("SENDER", "STDIN/GSMA/SHOW", "/GSMA SHOW SENDER <id>", -1, Command::FLAG_CLEANUP, new stdinSHOWSENDER);
*/
    _infof("*** Module Web: Commands Initialized");
  } // Web::initializeCommands

  void Web::initializeDatabase() {
    try {
      _dbi_w = new DBI_Web(new openapi::App_Log(""), MODULE_WEB_SQL_WRITE_DB, MODULE_WEB_SQL_WRITE_HOST, MODULE_WEB_SQL_WRITE_USER, MODULE_WEB_SQL_WRITE_PASS);
      _dbi_w->logger(new openapi::App_Log(""));

      _dbi_r = new DBI_Web(new openapi::App_Log(""), MODULE_WEB_SQL_READ_DB, MODULE_WEB_SQL_READ_HOST, MODULE_WEB_SQL_READ_USER, MODULE_WEB_SQL_READ_PASS);
      _dbi_r->logger(new openapi::App_Log(""));
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch

    _infof("*** Module Web: Database Deinitialized");
  } // Web::initializeDatabase

  void Web::deinitializeSystem() {
    deinitializeCommands();
    deinitializeDatabase();
    _infof("*** Module Web: System Deinitialized");
  } // Web::deinitializeSystem

  void Web::deinitializeCommands() {
    app->command()->removeCommand("FCGI/BALLOON");
    app->command()->removeCommand("FCGI/LASTPOSITION");
    _infof("*** Module Web: Commands Deinitialized");
  } // Web::initializeCommands

  void Web::deinitializeDatabase() {
    if (_dbi_w) delete _dbi_w;
    if (_dbi_r) delete _dbi_r;
    _infof("*** Module Web: Database Deinitialized");
  } // Web::deinitializeDatabase
} // namespace web
