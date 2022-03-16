#include <fcgi_stdio.h>

#include <string>

#include <stdlib.h>

#include <openframe/openframe.h>

#include "App.h"
#include "DBI.h"

#include "config.h"
#include "openapi.h"
#include "openapiCommands.h"

namespace openapi {

  const char *App::kPidFile     = "openapi.pid";

  App::App(const std::string &prompt, const std::string &config)
      : super(prompt, config) {
    _dbi_w = NULL;
    _dbi_r = NULL;
    _numRequests = 0;
    _stats = NULL;
    _debug = false;

    _env["QUERY_STRING"] = "request.query.string";
    _env["REQUEST_METHOD"] = "request.method";
    _env["CONTENT_TYPE"] = "request.content.type";
    _env["CONTENT_LENGTH"] = "request.content.length";
    _env["SCRIPT_NAME"] = "request.script.name";
    _env["REQUEST_URI"] = "request.uri";
    _env["DOCUMENT_URI"] = "request.document.uri";
    _env["DOCUMENT_ROOT"] = "request.document.root";
    _env["SERVER_PROTOCOL"] = "request.server.protocol";
    _env["GATEWAY_INTERFACE"] = "request.gateway.interface";
    _env["SERVER_SOFTWARE"] = "request.server.software";
    _env["REMOTE_ADDR"] = "request.remote.addr";
    _env["REMOTE_PORT"] = "request.remote.port";
    _env["SERVER_ADDR"] = "request.server.addr";
    _env["SERVER_PORT"] = "request.server.port";
    _env["SERVER_NAME"] = "request.server.name";

  } // App::App

  App::~App() {
  } // App:~App

  bool App::onRun() {
    while(FCGI_Accept() >= 0 && !is_done() ) {
      _accept();
    } // while

    return true;
  } // App::onRun

  void App::onInitializeCommands() { }

  void App::onInitializeSystem() {
    const char *opts[] = { "openapi.syslog.perror", "openapi.syslog.console", "openapi.syslog.pid", NULL };
    const int optmask[] = { SyslogController::SYSCLOG_PERROR, SyslogController::SYSCLOG_CONS, SyslogController::SYSCLOG_PID, 0 };

    try {
      SyslogController::logOptionType mask = 0;
      for(size_t i=0; opts[i] != NULL; i++)
        mask |= optmask[i];
      _syslog = new SyslogController(cfg->get_string("openapi.syslog.ident", "openapi"),
                                     mask,
                                     cfg->get_string("openapi.syslog.facility", "local0"));
      _syslog->setlogmask(cfg->get_int("openapi.syslog.level", 6));
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch
  } // App::onInitializeSystem

  void App::onInitializeConfig() { }

  void App::onInitializeDatabase() {
    try {
      _dbi_r = new DBI(new App_Log(""), SQL_READ_DB, SQL_READ_HOST, SQL_READ_USER, SQL_READ_PASS);
      _dbi_w = new DBI(new App_Log(""), SQL_WRITE_DB, SQL_WRITE_HOST, SQL_WRITE_USER, SQL_WRITE_PASS);
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch
  } // App::onInitializeDatabase

  void App::onInitializeModules() { }

  void App::onInitializeThreads() {
    std::string source = app->cfg->get_string("app.stompstats.source", "/topic/stats.openstomp");
    std::string instance = app->cfg->get_string("app.stompstats.instance", "prod");
    time_t update_interval = app->cfg->get_int("app.stompstats.interval", 300);
    std::string hosts = app->cfg->get_string("app.stompstats.hosts", "localhost:61613");
    std::string login = app->cfg->get_string("app.stompstats.login", "openstomp-stompstats");
    std::string passcode = app->cfg->get_string("app.stats.passcode", "openstomp-stompstats");
    int maxqueue = app->cfg->get_int("app.stompstats.maxqueue", 100);
/*
    _stats = new stomp::StompStats(source,
                                   instance,
                                   update_interval,
                                   maxqueue,
                                   hosts,
                                   login,
                                   passcode);

//    _stats->elogger(&elog, "app");
    _stats->start();
*/
  } // App::onInitializeThreads

  void App::onDeinitializeCommands() { }
  void App::onDeinitializeSystem() { }

  void App::onDeinitializeDatabase() {
    if (_dbi_r) delete _dbi_r;
    if (_dbi_w) delete _dbi_w;
  } // App::onDeinitializeDatabase

  void App::onDeinitializeThreads() {
    if (_stats) delete _stats;
  } // App::onDeinitializeThreads

  void App::onDeinitializeModules() { }

  void App::_accept() {
    envMapType::iterator ptr;
    openframe::Packet packet;
    openframe::Stopwatch sw;

    _numRequests++;

    sw.Start();

    FCGI_printf("Content-type: text/plain\r\n\r\n");

    for(ptr = _env.begin(); ptr != _env.end(); ptr++) {
      char *env = getenv(ptr->first.c_str());
      if (env == NULL) {
        _infof("ERROR: Missing %s", ptr->first.c_str());
        return;
      } // if

      packet.addString(ptr->second, env);
    } // for

    _infof("Accepted request %d from %s", _numRequests, packet.getString("request.remote.addr").c_str());

    if (is_debug()) {
      std::list<string> slist;
      packet.showStringTree("root", 0, "# ", slist);
      FCGI_printf("# root\n");
      while(!slist.empty()) {
        FCGI_printf("%s\n", slist.front().c_str());
        slist.pop_front();
      } // while
    } // if

    openframe::Url u(packet.getString("request.query.string"));

    _infof("%s requested %s", packet.getString("request.remote.addr").c_str(), packet.getString("request.uri").c_str());
    if (is_debug() ) {

      openframe::Url::varMapType::iterator uptr;
      openframe::Result res("key,value");
      for(uptr = u.begin(); uptr != u.end(); uptr++) {
        openframe::Serialize s;
        s.add(uptr->first);
        s.add(uptr->second);
        openframe::Row row(s.compile());
        res.add(row);
      } // for
      openframe::Result::print(res, 80, sw.Time());
    } // if

    packet.addVariable("url", &u);
    packet.addVariable("log", _logger);
    if (!u.exists("c")) {
      FCGI_printf("Command not found.\n");
      return;
    } // if

    // initialize variables
    openframe::StringToken parseCommand = u["c"];
    unsigned int nodeLevel = 0;
    int returnResult = time(NULL);

    // find the command
    openframe::Command *tCommand = command()->findCommand("FCGI", parseCommand, nodeLevel, NULL);

    double total = 0.0;
    if (tCommand != NULL) {
      packet.setSource("FCGI");
      packet.setArguments(parseCommand.getRange(nodeLevel, parseCommand.size()));
      packet.setPath(tCommand->getPath());

      sw.Start();
      returnResult = tCommand->getCommandHandler()->Execute(&packet);
      total = sw.Time();
    } // if

    switch(returnResult) {
      case CMDERR_SUCCESS:
//        _fcgif("# Total %0.5fs", total);
        break;
      case CMDERR_SYNTAX:
        std::cout << "Syntax Error: " << tCommand->getSyntax() << std::endl;
        std::cout << "Type /HELP for a list of available commands." << std::endl;
        break;
      case CMDERR_ACCESS:
        break;
      default:
        std::cout << "Invalid Command: " << u["c"] << std::endl;
        std::cout << "Type /HELP for a list of available commands." << std::endl;
        break;
    } // switch

  } // App::_accept

  void App::rcvSighup() {
//    LOG(LogNotice, << "### SIGHUP Received" << std::endl);
//    elogger()->hup();
  } // App::rcvSighup
  void App::rcvSigusr1() {
//    LOG(LogNotice, << "### SIGHUS1 Received" << std::endl);
  } // App::Sigusr1
  void App::rcvSigusr2() {
//    LOG(LogNotice, << "### SIGUSR2 Received" << std::endl);
  } // App::Sigusr2
  void App::rcvSigint() {
//    LOG(LogNotice, << "### SIGINT Received" << std::endl);
    set_done(true);
  } // App::rcvSigint
  void App::rcvSigpipe() {
//    LOG(LogNotice, << "### SIGPIPE Received" << std::endl);
  } // App::rcvSigpipe

} // openapi
