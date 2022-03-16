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
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <mysql++.h>

#include "App.h"
#include "App_Log.h"
#include "Web.h"
#include "DBI_Web.h"

#include "webCommands.h"

namespace modweb {
  using namespace mysqlpp;

/**************************************************************************
 ** fcgiLASTPOSITION Class                                               **
 **************************************************************************/
  class DigiCoord {
    public:
      DigiCoord(const std::string &source) : _found(false), _lat(""), _lng(""), _source(source), _created("") { }
      virtual ~DigiCoord() { }

      const bool found() const { return _found; }
      void found(const bool found) { _found = found; }
      const std::string coords() { return (_lat + string(",") + _lng); }
      const std::string source() const { return _source; }
      const std::string lat() const { return _lat; }
      void lat(const std::string &lat) { _lat = lat; }
      const std::string lng() const { return _lng; }
      void lng(const std::string &lng) { _lng = lng; }
      const bool compare(const std::string &source) { return !strcasecmp(_source.c_str(), source.c_str()); }
      const std::string created() const { return _created; }
      void created(const std::string &created) { _created = created; }

    protected:
      bool _found;
      std::string _lat;
      std::string _lng;
      std::string _source;
      std::string _created;
  }; // DigiCoord

  typedef list<DigiCoord> posListType;
  typedef map<string, posListType> posMapType;

  inline double fround(double n, unsigned d) {
    return floor(n * pow(10., d) + .5) / pow(10., d);
  } // fround

  const bool createTxtLastsequenceForWebChat(string &ret) {
    // process last web_chat sequence
    openapi::DBI::resultType res;
    bool isOK = web->dbi_r()->getLastsequenceFromWebChat(res);

    if (!isOK)
      return false;

    Vars v;
    v.add("TY", "WC");
    v.add("SQ", res[0][0].c_str());

    ret = v.compile();

    return true;
  } // createTxtLastsequenceForWebChat

  const bool createTxtPositionsBySource(const std::string &source, const posMapType &pm, std::string &ret) {
    posMapType::const_iterator pptr;

    pptr = pm.find(source);
    if (pptr == pm.end()) return false;

    const posListType *pl = &pptr->second;

    std::string pstr = "";
    double plat = 0;
    double plng = 0;

    for(posListType::const_reverse_iterator ptr=pl->rbegin(); ptr != pl->rend(); ptr++) {
      const DigiCoord *dc = &(*ptr);
      Vars v;
      double lat, lng, dlat, dlng;

      lat = double(atof(dc->lat().c_str())) * 1e6;
      lng = double(atof(dc->lng().c_str())) * 1e6;

      dlat = fround(lat - plat, 6);
      dlng = fround(lng - plng, 6);

      if (dlat == 0.0 && dlng == 0.0) continue;

      v.add("L", dc->lat());
      v.add("G", dc->lng());
      v.add("T", dc->created());
      pstr += v.compile() + "\n";

      plat = lat;
      plng = lng;
    } // for

    if (pstr.length() < 1)
      return false;

    ret = pstr;
    return true;
  } // createTxtPositionsBySource

/*
  const bool createTxtPositionsBySource(const std::string &source, const int save_id, const std::string &age, const int limit_tracks,
                                        std::string &ret) {
    mysqlpp::StoreQueryResult res;
    bool isOK;

    if (save_id)
      isOK = web->dbi()->getSavedPositionsBySource(save_id, source, age, res);
    else
      isOK = web->dbi()->getMemPositionsBySource(source, age, limit_tracks, res);

    std::string pstr = "";
    double plat = 0;
    double plng = 0;

    for(DBI::resultType::const_reverse_iterator ptr=res.rbegin(); ptr != res.rend(); ptr++) {
      mysqlpp::Row row = *ptr;
      Vars v;
      double lat, lng, dlat, dlng;

      if (row["latitude"].is_null() || row["longitude"].is_null())
        continue;

      lat = double(row["latitude"]) * 1e6;
      lng = double(row["longitude"]) * 1e6;

      dlat = fround(lat - plat, 6);
      dlng = fround(lng - plng, 6);

      if (dlat == 0.0 && dlng == 0.0)
        continue;

      v.add("L", row["latitude"].c_str());
      v.add("G", row["longitude"].c_str());
      v.add("T", row["create_ts"].c_str());
      pstr += v.compile() + "\n";

      plat = lat;
      plng = lng;
    } // for

    if (pstr.length() < 1)
      return false;

    ret = pstr;
    return true;
  } // createTxtPositionsBySource
*/

  const bool createTxtPaths(const std::string &path, const std::string &lat, const std::string &lng, std::string &ret, int &hitrate) {
    openframe::StringToken st;
    bool isOK;
    std::string lpath = path;
    vector<DigiCoord> pa;
    unsigned int tries = 0;
    unsigned int hits = 0;

    if (path.length() < 1)
      return false;

    openframe::StringTool::replace("*", "", lpath);
    st.setDelimiter(',');
    st = lpath;
    if (st.size() < 1)
      return 0;

    // create digi list
    openframe::StringTool::regexMatchListType rl;
    for(size_t i=0; i < st.size(); i++) {
      if (openframe::StringTool::ereg("^(WIDE.*|qA.*|APRS.*|TCPIP.*|TCPXX|CORE.*|CWOP.*|FIRST|SECOND|THIRD|FOURTH|BEACON|AP.*)", st[i], rl))
        continue;

      DigiCoord digicoord(st[i]);
      pa.push_back( digicoord );
    } // for

    // first pass use memcached to populate digi list
    std::stringstream leftovers;
    for(size_t i=0; i < pa.size(); i++) {
      std::string lat, lng;
      bool fixed;
      DigiCoord *dc = &pa[i];

      tries++;
      isOK = web->dbi_r()->getDigiCoordinatesFromMemcached(dc->source(), lat, lng, fixed);

      if (!isOK) {
        if (leftovers.str().length())
          leftovers << ",";

        leftovers << "'" << dc->source() << "'";
        continue;
      } // if

      // not a position we know about, skip
      if (!fixed) continue;

      dc->found(true);
      dc->lat(lat);
      dc->lng(lng);
      hits++;
    } // for

    // seconds pass take leftovers and find in mysql
    if (leftovers.str().length()) {
      DBI_Web::resultType res;
      isOK = web->dbi_r()->getCoordinatesBySourceList(leftovers.str(), res);
      if (isOK) {
        for(openapi::DBI::resultSizeType i=0; i < res.num_rows(); i++) {
          for(size_t j=0; j < pa.size(); j++) {
            DigiCoord *dc = &pa[j];
            if (dc->found() || !dc->compare(res[i]["source"].c_str()))
              continue;

            if (res[i]["latitude"].is_null() || res[i]["longitude"].is_null())
              continue;

            dc->found(true);
            dc->lat(res[i]["latitude"].c_str());
            dc->lng(res[i]["longitude"].c_str());
            // store in cache yay!
            web->dbi_w()->setDigiCoordinatesInMemcached(dc->source(), dc->lat(), dc->lng());
          } // for
        } // for
      } // if
    } // if

    std::stringstream digis;
    digis << lat << "," << lng;
    for(size_t i=0; i < pa.size(); i++) {
      DigiCoord *dc = &pa[i];
      if (!dc->found()) {
        // don't look for them for a while
        web->dbi_w()->setDigiCoordinatesInMemcached(dc->source());
        continue;
      } // if

      digis << "," <<  dc->coords();
    } // for

    if (digis.str().length() < 1)
      return false;

    ret = digis.str();

    if (tries)
      hitrate = double(hits / tries * 100);
    else
      hitrate = 0;

    return true;
  } // createTxtPaths

  const int fcgiLASTPOSITION::Execute(COMMAND_ARGUMENTS) {
    openapi::App_Log *log = static_cast<openapi::App_Log *>( ePacket->getVariable("log") );
    openframe::Stopwatch sw;
    openframe::StringToken st;
    openframe::StringTool::regexMatchListType rl;
    openframe::Stopwatch total;
    Url *u = (Url *) ePacket->getVariable("url");
    mysqlpp::StoreQueryResult res;
    bool isOK;
    bool wantTrack = false;
    bool wantLoad = false;
    int limit = app->cfg->get_int("module.web.result.limit", Web::DEFAULT_RESULT_LIMIT);
    int limit_tracks = app->cfg->get_int("module.web.result.limit", Web::DEFAULT_RESULT_LIMIT);
    int limitWontTrack = app->cfg->get_int("module.web.wonttrack.limit", Web::DEFAULT_WONTTRACK_LIMIT);
    std::string age = app->cfg->get_string("module.web.age", Web::DEFAULT_AGE);
    std::string hash;
    int wantLimit;
    int save_id = 0;
    std::string id;
    map<string, double> stats;

    total.Start();
    stats["totalPositions"] = 0.0;
    stats["totalPath"] = 0.0;
    stats["totalLastposition"] = 0.0;

    if (!u->exists("bbox"))
      return CMDERR_SYNTAX;

    st.setDelimiter(',');
    st = (*u)["bbox"];

    if (st.size() != 4)
      return CMDERR_SYNTAX;

    if (u->exists("limit")) {
      wantLimit = atoi((*u)["limit"].c_str());
      if (wantLimit < limit)
        limit = wantLimit;
    } // if

    if (u->exists("limit_tracks")) {
      wantLimit = atoi((*u)["limit_tracks"].c_str());
      if (wantLimit < limit_tracks)
        limit_tracks = wantLimit;
    } // if

    if (u->exists("trk") && (*u)["trk"] == "yes")
      wantTrack = true;

    if (u->exists("age") && openframe::StringTool::ereg("^[0-9]{1,2} (YEAR|MONTH|DAY|HOUR|MINUTE|SECOND)$", (*u)["age"], rl) > 0)
      age = (*u)["age"];

    DBI_Web::resultType save;
    if (u->exists("load")) {
      hash = (*u)["load"];
      if (hash.length() != 32)
        return CMDERR_SYNTAX;

      isOK = web->dbi_r()->getViewByHash(hash, save);
      if (!isOK) {
        log->warn("no view found for " + hash);
        return CMDERR_SUCCESS;
      } // if

      wantLoad = true;
      save_id = atoi(save[0]["id"].c_str());
    } // if

    // initialize variables
    Coordinates begin(atof(st[3].c_str()), atof(st[0].c_str()));
    Coordinates end(atof(st[1].c_str()), atof(st[2].c_str()));
    BoundingBox bb(begin, end);

    sw.Start();
    if (wantLoad)
      isOK = web->dbi_r()->getSavedLastpositionsByCoordinates(save_id, bb, age, limit, res);
    else
      isOK = web->dbi_r()->getLastpositionsByCoordinates(bb, age, limit, res);
    FCGI_printf("# getLastpositionsByCoordinates %0.5fs\n", sw.Time());
    stats["totalLastposition"] += sw.Time();

    if (!isOK)
      return CMDERR_SUCCESS;

    // don't track for large results
    if (res.num_rows() > (unsigned) limitWontTrack) {
      log->info("results too large, not tracking");
      wantTrack = false;
    } // if


    // get positions for all stations and assemble into list
    sw.Start();
    std::stringstream ss;
    for(openapi::DBI::resultSizeType i=0; i < res.num_rows(); i++) {
      if (ss.str().length())
        ss << ",";
      ss << "'" << res[i]["source"].c_str() << "'";
    } // for
    DBI_Web::resultType pos;
    web->dbi_r()->getPositionsBySourceList(ss.str(), age, pos);
    stats["totalPositions"] += sw.Time();

    posMapType posMap;
    for(openapi::DBI::resultSizeType i=0; i < pos.num_rows(); i++) {
      if (pos[i]["latitude"].is_null() || pos[i]["longitude"].is_null())
        continue;

      std::string src = pos[i]["source"].c_str();
      if (posMap.find(src) != posMap.end() && posMap[src].size() > limit_tracks)
        continue;

      DigiCoord dc(src);
      dc.lat(pos[i]["latitude"].c_str());
      dc.lng(pos[i]["longitude"].c_str());
      dc.created(pos[i]["create_ts"].c_str());
      posMap[src].push_front(dc);
    } // for



//    DBI::print(res);
    for(openapi::DBI::resultSizeType i=0; i < res.num_rows(); i++) {
      Vars v;
      v.add("TY", "LP");
      v.add("SR", res[i]["source"].c_str());

      if (res[i]["name"].length())
        v.add("NM", res[i]["name"].c_str());
      else
        v.add("NM", res[i]["source"].c_str());

      v.add("ID", res[i]["packet_id"].c_str());
      v.add("LA", res[i]["latitude"].c_str());
      v.add("LO", res[i]["longitude"].c_str());
      v.add("CT", res[i]["create_ts"].c_str());

      if (!wantLoad && !res[i]["phg_range"].is_null())
        v.add("PRNG", res[i]["phg_range"].c_str());

      if (!res[i]["icon"].is_null())
        v.add("IMG", res[i]["icon"].c_str());

      if (res[i]["symbol_table"] != "\\" && res[i]["symbol_table"] != "/")
        v.add("OVR", res[i]["symbol_table"].c_str());

      if (!wantLoad && !res[i]["phg_direction"].is_null() && res[i]["phg_direction"] != "-1")
        v.add("PDIR", res[i]["phg_direction"].c_str());

      // paths
      std::string path;
      sw.Start();
      int hitrate;
      isOK = createTxtPaths(res[i]["path"].c_str(), res[i]["latitude"].c_str(), res[i]["longitude"].c_str(), path, hitrate);
      if (isOK)
        v.add("DP", path);
      FCGI_printf("# getCoordinatesByPath %0.5fs, hitrate %d%%\n", sw.Time(), hitrate);
      stats["totalPath"] += sw.Time();

      // track
      if (wantTrack) {
        std::string pos;
        sw.Start();
//        isOK = createTxtPositionsBySource(res[i]["source"].c_str(), save_id, age, limit_tracks, pos);
        isOK = createTxtPositionsBySource(res[i]["source"].c_str(), posMap, pos);
        v.add("POS", pos);
        stats["totalPositions"] += sw.Time();
        FCGI_printf("# getPositionsBySource %0.5fs\n", sw.Time());
      } // if

      FCGI_printf("%s\n", v.compile().c_str());
    } // for

    sw.Start();
    std::string seq;
    isOK = createTxtLastsequenceForWebChat(seq);
    if (isOK)
      FCGI_printf("%s\n", seq.c_str());
    FCGI_printf("# getLastsequenceFromWebChat %0.5fs\n", sw.Time());

    stats["pa+pos"] = stats["totalPath"]+stats["totalPositions"];
    stats["pa+pos+lp"] = stats["totalPath"]+stats["totalPositions"]+stats["totalLastposition"];
    stats["total"] = total.Time();
    std::stringstream s;
    s << std::fixed << std::setprecision(0) << "stats lp " << (stats["totalLastposition"]*1000)
      << "ms, pa " << (stats["totalPath"]*1000)
      << "ms, pos " << (stats["totalPositions"]*1000)
      << "ms, pa+pos " << (stats["pa+pos"]*1000)
      << "ms, pa+pos+lp " << (stats["pa+pos+lp"]*1000)
      << "ms, total " << (stats["total"]*1000) << "ms";

    if (stats["total"] > 2.0)
      log->warn(s.str());
    else
      log->info(s.str());

    FCGI_printf("# Total getCoordinatesByPath %0.5fs\n", stats["totalPath"]);
    FCGI_printf("# Total getPositionsBySource %0.5fs\n", stats["totalPositions"]);
    FCGI_printf("# Total path+positions %0.5fs\n", stats["pa+pos"]);
    FCGI_printf("# Total path+positions+lastposition %0.5fs\n", stats["pa+pos+lp"]);

    return CMDERR_SUCCESS;
  } // fcgiLASTPOSITION::Execute
} // namespace modweb
