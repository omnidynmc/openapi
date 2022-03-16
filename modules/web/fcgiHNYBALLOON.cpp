#include <fcgi_stdio.h>

#include <list>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <iostream>
#include <string>
#include <exception>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <mysql++.h>

#include <openframe/openframe.h>

#include "App.h"
#include "EarthTools.h"
#include "DBI_Web.h"
#include "Tag.h"
#include "Web.h"

#include "webCommands.h"

namespace modweb {
  using namespace mysqlpp;

/**************************************************************************
 ** fcgiBALLOON Class                                                    **
 **************************************************************************/

  const int fcgiHNYBALLOON::Execute(COMMAND_ARGUMENTS) {
    openapi::App_Log *log = static_cast<openapi::App_Log *>( ePacket->getVariable("log") );
    openframe::StringToken st;
    openframe::StringTool::regexMatchListType rl;
    openframe::Url *u = (openframe::Url *) ePacket->getVariable("url");
    mysqlpp::StoreQueryResult res;
    std::string hash;
    map<string, double> stats;

    openframe::Stopwatch sw;

    if (!u->exists("id"))
      return CMDERR_SYNTAX;

    std::string id = (*u)["id"];

    if (openframe::StringTool::ereg("^([0-9]+)$", id, rl) < 1)
      return CMDERR_SYNTAX;

    // initialize variables

    std::stringstream html;
//    std::string track_url = "http://www.oaprs.net/search/" + string(res[0]["source"].c_str());

    openframe::Stopwatch sw_uls;
    sw_uls.Start();
    DBI_Web::resultType uls;
    bool ok = web->dbi_r()->getUlsByUsi(id, uls);
    if (!ok) return CMDERR_SUCCESS;

    stats["uls"] = sw_uls.Time();

    openframe::Vars v;
    v.add("TY", "BL");

    sw.Start();
    std::string file;

    try {
      file = openframe::StringTool::getFileContents(app->cfg->get_string("module.web.html", Web::DEFAULT_PATH_HTML)
                                                    + app->cfg->get_string("module.web.html.hnyballoon", Web::DEFAULT_FILE_HNYBALLOON));
    } // try
    catch(openapi::Tag_Exception e) {
      std::cout << "ERROR: opening balloon template; " << e.message() << std::endl;
      return CMDERR_SUCCESS;
    } // catch

    openframe::StringTool::replace("${callsign}", uls[0]["callsign"].c_str(), file);
    openframe::StringTool::replace("${entity_name}", uls[0]["entity_name"].c_str(), file);
    openframe::StringTool::replace("${class}", uls[0]["class"].c_str(), file);
    openframe::StringTool::replace("${street_address}", uls[0]["street_address"].c_str(), file);
    openframe::StringTool::replace("${city}", uls[0]["city"].c_str(), file);
    openframe::StringTool::replace("${state}", uls[0]["state"].c_str(), file);
    openframe::StringTool::replace("${zip_code}", uls[0]["zip_code"].c_str(), file);
    FCGI_printf("# Total Scripting %0.5f\n", sw.Time());

    v.add("BL", file);

    FCGI_printf("%s\n", v.compile().c_str());

    stats["total"] = sw.Time();

    std::stringstream s;
    s << std::fixed << std::setprecision(0) << "stats hny balloon "
      << "uls " << (stats["uls"]*1000) << "ms"
      << ", total " << (stats["total"]*1000) << "ms";

    if (stats["total"] > 2.0)
      log->warn(s.str());
    else
     log->info(s.str());

    openapi::DBI::print(uls);
    FCGI_printf("# Total getUlsByCallsign %0.5fs\n", stats["uls"]);

    return CMDERR_SUCCESS;
  } // fcgiHNYBALLOON::Execute
} // namespace modweb
