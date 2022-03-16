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

  const std::string extractCallsign(const std::string &aprsCallsign) {
    openframe::StringTool::regexMatchListType regexList;         // Map of regex matches.
    std::string call;

    if (openframe::StringTool::ereg("^([A-Z0-9]{3,7})([-][0-9]{1,2})$", aprsCallsign, regexList) > 0) {
      call = regexList[1].matchString;
    } // if
    else
      call = aprsCallsign;

    return call;
  } // extractCallsign


  const bool parseMotion(openapi::DBI::resultType &res, const std::string &units, std::string &ret) {
    std::stringstream s;

    if (res.size() != 1)
      return false;

    if (!res[0]["speed"].is_null()) {
      double speed = atof(res[0]["speed"].c_str());
      s << openapi::EarthTools::convertBySystem(speed, "speed", units, 1);
    } // if

    if (!res[0]["course"].is_null()) {
      int course = atoi(res[0]["course"].c_str());
      if (course > 0) {
        if (s.str().length())
          s << " ";
        s << course << "&deg;";
      } // if
    } // if

    if (!res[0]["altitude"].is_null()) {
      double altitude = atof(res[0]["altitude"].c_str());
      if (s.str().length())
        s << " ";

      s << openapi::EarthTools::convertBySystem(altitude, "ruler", units, 1);
    } // if

    ret = s.str();

    return (ret.length() > 0 ? true : false);
  } // parseMotion

  const int fcgiBALLOON::Execute(COMMAND_ARGUMENTS) {
    openapi::App_Log *log = static_cast<openapi::App_Log *>( ePacket->getVariable("log") );
    openframe::Stopwatch sw;
    openframe::Stopwatch sw_total;
    openframe::StringToken st;
    openframe::StringTool::regexMatchListType rl;
    openframe::Url *u = (openframe::Url *) ePacket->getVariable("url");
    mysqlpp::StoreQueryResult res;
    bool isOK;
    bool isTime = false;
    bool wantLoad = false;
    std::string age = app->cfg->get_string("module.web.age", Web::DEFAULT_AGE);
    std::string name;
    std::string units;
    std::string hash;
    time_t seconds;
    map<string, double> stats;

    sw_total.Start();

    if (!u->exists("n"))
      return CMDERR_SYNTAX;

    if (!u->exists("units"))
      return CMDERR_SYNTAX;

    name = (*u)["n"];
    units = (*u)["units"];

    if (openframe::StringTool::ereg("^([\x20-\x7e]+)$", name, rl) < 1)
      return CMDERR_SYNTAX;

    if (u->exists("t")) {
      isTime = true;
      if (openframe::StringTool::ereg("^([0-9]+)$", (*u)["t"], rl) < 1)
        return CMDERR_SYNTAX;
      seconds = atoi( (*u)["t"].c_str() );
    } // if

    if (u->exists("timezone"))
      web->dbi_r()->setTimezone( (*u)["timezone"] );

    DBI_Web::resultType save;
    int save_id = 0;
    if (u->exists("load")) {
      hash = (*u)["load"];
      if (hash.length() != 32)
        return CMDERR_SYNTAX;

      isOK = web->dbi_r()->getViewByHash(hash, save);
      if (!isOK)
        return CMDERR_SUCCESS;

      wantLoad = true;
      save_id = atoi(save[0]["id"].c_str());
    } // if

    // initialize variables
    sw.Start();
    if (isTime) {
      if (wantLoad)
        isOK = web->dbi_r()->getSavedPositionBySourceAndTime(save_id, name, seconds, res);
      else
        isOK = web->dbi_r()->getPositionBySourceAndTime(name, seconds, res);
    } // if
    else {
      if (wantLoad)
        isOK = web->dbi_r()->getSavedLastpositionBySource(save_id, name, res);
      else
        isOK = web->dbi_r()->getLastpositionBySource(name, age, res)
               || web->dbi_r()->getLastpositionByName(name, age, res);
    } // else

    FCGI_printf("# getLastpositionBySource %0.5fs\n", sw.Time());
    stats["lastposition"] = sw.Time();

    if (!isOK)
      return CMDERR_SUCCESS;

    std::stringstream html;
    std::string track_url = "http://www.oaprs.net/search/" + string(res[0]["source"].c_str());

    sw.Start();
    DBI_Web::resultType uls;
    isOK = web->dbi_r()->getUlsByCallsign(extractCallsign(res[0]["source"].c_str()), uls);
    if (isOK) {
      if (html.str().length())
        html << "<br />";
      html << openframe::StringTool::toUpper(uls[0]["entity_name"].c_str()) << " (" << uls[0]["class"].c_str() << ")";
    } // if
    stats["uls"] = sw.Time();

    std::string motion;
    if (parseMotion(res, units, motion)) {
      if (html.str().length())
        html << "<br />";
      html << motion;
    } // if

    sw.Start();
    DBI_Web::resultType wx;
    isOK = web->dbi_r()->getWeatherStringBySource(name, units, wx);
    if (isOK) {
      if (html.str().length())
        html << "<br />";
      html << wx[0]["weather"].c_str();
    } // if
    stats["wx"] = sw.Time();

    if (!res[0]["status"].is_null() && strlen(res[0]["status"].c_str())) {
      if (html.str().length())
        html << "<br />";
      html << res[0]["status"].c_str();
    } // if

    openframe::Vars v;
    v.add("TY", "BL");
    v.add("CT", res[0]["create_ts"].c_str());

    sw.Start();
    std::string file;

    try {
      file = openframe::StringTool::getFileContents(app->cfg->get_string("module.web.html", Web::DEFAULT_PATH_HTML)
                                                    + app->cfg->get_string("module.web.html.balloon", Web::DEFAULT_FILE_BALLOON));
    } // try
    catch(openapi::Tag_Exception e) {
      std::cout << "ERROR: opening balloon template; " << e.message() << std::endl;
      return CMDERR_SUCCESS;
    } // catch

    std::string objsrc = "";

    if (!isTime && !res[0]["name"].is_null() && res[0]["type"] == "O") {
      objsrc = "(from: " + string(res[0]["source"].c_str()) + ")";
      name = res[0]["name"].c_str();
    } // if
    else
      name = res[0]["source"].c_str();

    openframe::StringTool::replace("${source}", name, file);
    openframe::StringTool::replace("${objsrc}", objsrc, file);
    openframe::StringTool::replace("${lat}", res[0]["latitude"].c_str(), file);
    openframe::StringTool::replace("${lng}", res[0]["longitude"].c_str(), file);
    openframe::StringTool::replace("${icon}", "http://www.openaprs.net/images/icons/"+string(res[0]["icon"].c_str()), file);
    openframe::StringTool::replace("${info}", track_url, file);
    openframe::StringTool::replace("${path}", res[0]["path"].c_str(), file);
    openframe::StringTool::replace("${date}", res[0]["packet_date"].c_str(), file);
    openframe::StringTool::replace("${locator}", "", file);
    openframe::StringTool::replace("${extra}", html.str(), file);
    FCGI_printf("# Total Scripting %0.5f\n", sw.Time());

    v.add("BL", file);

    FCGI_printf("%s\n", v.compile().c_str());

    stats["total"] = sw_total.Time();

    std::stringstream s;
    s << std::fixed << std::setprecision(0) << "stats balloon "
      << "wx " << (stats["wx"]*1000) << "ms"
      << ", uls " << (stats["uls"]*1000) << "ms"
      << ", lp " << (stats["lastposition"]*1000) << "ms"
      << ", total " << (stats["total"]*1000) << "ms";

    if (stats["total"] > 2.0)
      log->warn(s.str());
    else
      log->info(s.str());

    openapi::DBI::print(res);
    FCGI_printf("# getWeatherStringBySource %0.5fs\n", stats["wx"]);
    FCGI_printf("# Total getUlsByCallsign %0.5fs\n", stats["uls"]);
    FCGI_printf("# Total getLastpositionBySource %0.5fs\n", stats["lastposition"]);

    return CMDERR_SUCCESS;
  } // fcgiBALLOON::Execute
} // namespace modweb
