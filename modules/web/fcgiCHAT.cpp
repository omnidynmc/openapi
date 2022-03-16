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
 ** fcgiCHAT Class                                                       **
 **************************************************************************/

  bool isValidCallsign(const std::string &callsign) {
    openframe::StringTool::regexMatchListType rl;
    return openframe::StringTool::ereg("^([a-zA-Z0-9]{2,6})([-]{1}[0-9]+)?[*]*$", callsign, rl) > 0;
  } // isValidCallsign

  const int fcgiCHAT::Execute(COMMAND_ARGUMENTS) {
    openapi::App_Log *log = static_cast<openapi::App_Log *>( ePacket->getVariable("log") );
    openframe::StringTool::regexMatchListType rl;
    openframe::Url *u = (openframe::Url *) ePacket->getVariable("url");
    map<string, double> stats;

    openframe::Stopwatch sw_total;
    sw_total.Start();

    // check for broken implementation
    if (!u->exists("chat_sequence")) return CMDERR_SYNTAX;
    std::string chat_sequence = (*u)["chat_sequence"];

    bool ok = openframe::StringTool::ereg("^([0-9]+)$", chat_sequence, rl);
    if (!ok) {
      FCGI_printf("# ERROR: invalid chat sequence\n");
      return CMDERR_SYNTAX;
    } // if

    openframe::Stopwatch sw;
    sw.Start();

    // grab next chat sequence
    mysqlpp::StoreQueryResult res;
    ok = web->dbi_w()->get("getNextChatSeq", res);
    unsigned int next_sequence = 0;
    if (ok) next_sequence = atoi( res[0]["sequence"].c_str() );

    stats["next_seq"] = sw.Time();

    // process nick/ip
    std::string nick = u->exists("nick") ? (*u)["nick"] : "";
    std::string ip = ePacket->getString("request.remote.addr");
    bool is_user_call_valid = isValidCallsign(nick);
    if (!is_user_call_valid) {
      ok = openframe::StringTool::ereg("^([0-9]{1,3}[.][0-9]{1,3}[.])[0-9]{1,3}[.][0-9]{1,3}", ip, rl);
      if (!ok) return CMDERR_SYNTAX;
      nick = "[" + rl[1].matchString + "*]";
    } // if

    sw.Start();

    // process session if there
    std::string hash = u->exists("session") ? (*u)["session"] : "";

    std::string timezone = u->exists("timezone") ? (*u)["timezone"] : "unknown";
    std::string idle = u->exists("idle") && (*u)["idle"].length() ? (*u)["idle"] : "0";
    ok = openframe::StringTool::ereg("^([0-9a-zA-Z]{32})$", hash, rl);
    if (ok) {
      web->dbi_w()->set("setWebWho", hash, "openapi", ip, nick, timezone, "0", idle);
    } // if
    else
      FCGI_printf("# ERROR: invalid or missing hash\n");

    stats["set_who"] = sw.Time();

    sw.Start();

    // handle chat insertion
    std::string message = u->exists("message") ? (*u)["message"] : "";
    if (is_user_call_valid && message.length()) {
      bool inserted = web->dbi_w()->set("setChat", ip, nick, message, stringify<unsigned int>(next_sequence) );
      if (inserted) ++next_sequence;
    } // if
    else
      FCGI_printf("# ERROR: could not insert message, missing or invalid nick\n");

    stats["set_chat"] = sw.Time();

    sw.Start();

    // get current sequence
    ok = web->dbi_r()->get("getChatBySequence", res, chat_sequence);
    if (ok) {
      for(DBI_Web::resultSizeType i=res.num_rows() - 1; ; i--) {
        openframe::Vars v;
        v.add("TY", "CH");
        v.add("NK", res[i]["nick"].c_str());
        v.add("MS", res[i]["message"].c_str());
        v.add("SQ", res[i]["sequence"].c_str());
        v.add("DT", res[i]["post_date"].c_str());
        FCGI_printf("%s\n", v.compile().c_str());
        if (i == 0) break;
      } // for
    } // if

    stats["get_chat"] = sw.Time();

    sw.Start();

    // process web who
    time_t active_ts = time(NULL) - 60;
    DBI_Web::resultSizeType num_users = web->dbi_r()->get("getWhoByTime",
                                                          res,
                                                          stringify<time_t>(active_ts) );

    size_t hidden_count = 0;
    if (num_users) {
      for(DBI_Web::resultSizeType i=0; i < res.num_rows(); i++) {
        openframe::Vars v;
        v.add("TY", "OL");

        time_t idle = atoi( res[i]["idle"].c_str() );
        std::string css_class;
        if (idle < 300) {
          v.add("ISI", "0");
          css_class = "chatHere";
        } // if
        else {
          v.add("ISI", "1");
          css_class = "chatIdle";
        } // else

        std::stringstream url;
        std::string nick = openframe::Vars::Urlencode( res[i]["nick"].c_str(), false );
        bool is_call_valid = isValidCallsign(res[i]["nick"].c_str() );
        int count = atoi( res[i]["count"].c_str() );
        if (is_call_valid) {
          url << "<span class=\"" << css_class << "\"><a "
              << "href=\"javascript:changeFieldById(\'openaprs_form_chat_message\',\'"
              << nick << ", \')\">"
              << nick << "</a>";
          if (count > 1) url << " (" << count << ")";
          url << "</span>";

        } // if
        else {
          url << "<span class=\"" << css_class << "\">" << nick;
          if (count > 1) url << " (" << count << ")";
          url << "</span>";
          hidden_count++;
        } // if

        v.add("URL", url.str());
        v.add("NK", nick);
        v.add("IDL", res[i]["idle"].c_str() );
        FCGI_printf("%s\n", v.compile().c_str());
      } //for
    } // if

    openframe::Vars v;
    v.add("TY", "ST");
    v.add("NU", stringify<DBI_Web::resultSizeType>(num_users) );
    v.add("HC", stringify<size_t>(hidden_count) );
    v.add("SQ", stringify<unsigned int>(next_sequence) );
    FCGI_printf("%s\n", v.compile().c_str());

    stats["get_who"] = sw.Time();

    stats["total"] = sw_total.Time();

    std::stringstream s;
    s << std::fixed << std::setprecision(0) << "stats chat "
      << "seq " << (stats["next_seq"]*1000) << "ms"
      << ", who " << (stats["set_who"]*1000) << "ms"
      << ", post " << (stats["set_chat"]*1000) << "ms"
      << ", msgs " << (stats["get_chat"]*1000) << "ms"
      << ", whos " << (stats["get_who"]*1000) << "ms"
      << ", total " << (stats["total"]*1000) << "ms";

    if (stats["total"] > 2.0)
      log->warn(s.str());
    else
      log->info(s.str());

    FCGI_printf("# Total getNextChatSeq %0.5fs\n", stats["next_seq"]);
    FCGI_printf("# Total setWho %0.5fs\n", stats["set_who"]);
    FCGI_printf("# Total setChat %0.5fs\n", stats["set_chat"]);
    FCGI_printf("# Total getChatBySequence %0.5fs\n", stats["get_chat"]);
    FCGI_printf("# Total %0.5f\n", sw_total.Time());

    return CMDERR_SUCCESS;
  } // fcgiCHAT::Execute
} // namespace modweb
