#include <config.h>

#include <new>
#include <string>
#include <map>

#include <mysql++.h>

#include <openframe/openframe.h>

#include "App_Log.h"
#include "DBI_Web.h"

namespace modweb {
  DBI_Web::DBI_Web(openapi::App_Log *logger, const std::string &db, const std::string &host, const std::string &user, const std::string &pass) : 
                   DBI(logger, db, host, user, pass) {
    std::string lastposition = "\
callsign c \
LEFT JOIN lastposition lp ON c.id=lp.callsign_id \
LEFT JOIN statuses s ON s.id=lp.status_id \
LEFT JOIN path p ON p.id=lp.path_id \
LEFT JOIN aprs_icons ai ON (BINARY ai.symbol_table=IF(lp.symbol_table != '\\\\' && lp.symbol_table != '/', '\\\\', lp.symbol_table) AND BINARY ai.icon=lp.symbol_code) ";

    std::string fields = "\
  lp.packet_id, \
  c.source, \
  lp.name, \
  lp.destination, \
  p.body AS path, \
  lp.latitude, \
  lp.longitude, \
  lp.course, \
  lp.speed, \
  lp.altitude, \
  s.body AS status, \
  lp.symbol_table, \
  lp.symbol_code, \
  lp.overlay, \
  lp.phg_power, \
  lp.phg_haat, \
  lp.phg_gain, \
  lp.phg_range, \
  lp.phg_direction, \
  lp.phg_beacon, \
  lp.dfs_power, \
  lp.dfs_gain, \
  lp.dfs_direction, \
  lp.range, \
  lp.type, \
  lp.weather, \
  lp.telemetry, \
  lp.igate, \
  lp.postype, \
  lp.mbits, \
  IF(lp.course > 0 AND ai.direction = 'Y', \
    CONCAT(ai.icon_path, '/compass/', REPLACE(ai.image, '.png', ''), '-', getDirectionFromCourse(lp.course), '.png'), \
    CONCAT(ai.icon_path, '/', ai.image) \
  ) AS icon, \
  lp.create_ts";

    _queryMap["getLastpositionById"] = "SELECT "+fields+" FROM "+lastposition+" WHERE packet_id=%0q:id LIMIT 1";
    //string uni = " UNION SELECT "+fields+" FROM "+lastposition+" WHERE (lp.name = %0q:source AND lp.type = 'O')";
    //_queryMap["getLastpositionBySource"] = "SELECT "+fields+" FROM "+lastposition+" WHERE (c.source = %0q:source AND lp.type = 'P') "+uni;

    std::string where = "(lp.latitude <= %0q:lat1 AND lp.longitude >= %1q:lng1 AND lp.latitude >= %2q:lat2 AND lp.longitude <= %3q:lng2)";
    where += " AND (lp.packet_date >= NOW() - INTERVAL %4:age)";
    //_queryMap["getLastpositionsByCoordinates"] = "SELECT "+fields+" FROM "+lastposition+" WHERE "+where;
    _queryMap["getLastpositionsByCoordinates"] = "CALL getLastpositionByLatLng(%0q:lat1, %1q:lng1, %2q:lat2, %3q:lng2, NOW() - INTERVAL %4:age, %5:limit)";
    _queryMap["getMemLastpositionsByCoordinates"] = "CALL getMemLastpositionByLatLng(%0q:lat1, %1q:lng1, %2q:lat2, %3q:lng2, NOW() - INTERVAL %4:age, %5:limit)";
    _queryMap["getSavedLastpositionsByCoordinates"] = "CALL getSavedLastpositionByLatLng(%0q:save_id, %1q:lat1, %2q:lng1, %3q:lat2, %4q:lng2, NOW() - INTERVAL %5:age, %6:limit)";
//    _queryMap["getPositionsBySource"] = "SELECT t.latitude, t.longitude, pa.body AS path, t.create_ts FROM callsign c LEFT JOIN mem_position t ON c.id=t.callsign_id LEFT JOIN path pa ON pa.id=t.path_id WHERE c.source = %0q:source AND t.packet_date >= NOW() - INTERVAL %1:age ORDER BY t.packet_date DESC LIMIT %2:limit";
    _queryMap["getPositionsBySource"] = "CALL getPositionsBySource(%0q:source, NOW() - INTERVAL %1:age, %2:limit)";
    _queryMap["getMemPositionsBySource"] = "CALL getMemPositionsBySource(%0q:source, NOW() - INTERVAL %1:age, %2:limit)";
    _queryMap["getSavedPositionsBySource"] = "CALL getSavedPositionsBySource(%0:save_id, %1q:source, NOW() - INTERVAL %2:age)";
    _queryMap["getPositionBySourceAndTime"] = "CALL getPositionBySourceAndtime(%0q:source, %1:seconds)";
    _queryMap["getSavedPositionBySourceAndTime"] = "CALL getSavedPositionBySourceAndtime(%0q:save_id, %1q:source, %2:seconds)";
    _queryMap["getLastpositionBySource"] = "CALL getLastpositionBySource(%0q:source)";
    _queryMap["getLastpositionByName"] = "CALL getLastpositionByName(%0q:name)";
    _queryMap["getSavedLastpositionBySource"] = "CALL getSavedLastpositionBySource(%0q:save_id, %1q:source)";
    _queryMap["getWeatherStringBySource"] = "CALL getWeatherStringBySource(%0q:source, %1q:units)";
    _queryMap["getUlsByCallsign"] = "CALL getUlsByCallsign(%0q:callsign)";
    _queryMap["getUlsByUsi"] = "CALL getUlsByUsi(%0q:usi)";
    _queryMap["getViewByHash"] = "CALL getViewByHash(%0q:hash)";
    _queryMap["getLastsequenceFromWebChat"] = "SELECT MAX(sequence) AS seq FROM web_chat";
    _queryMap["getCoordinatesBySource"] = "SELECT t.latitude, t.longitude FROM callsign c INNER JOIN lastposition t ON c.id=t.callsign_id WHERE c.source=%q:source";

    // web chat //
    _queryMap["setChat"] = "CALL setChat(%0q:ip, %1q:nick, %2q:message, %3q:sequence)";
    _queryMap["setWebWho"] = "CALL setWebWho(%0q:hash, %1q:server, %2q:ip, %3q:nick, %4q:timezone, %5q:offset, %6q:idle)";
    _queryMap["getWhoByTime"] = "CALL getWhoByTime(%0q:time)";
    _queryMap["getNextChatSeq"] = "CALL getNextChatSeq()";
    _queryMap["getChatBySequence"] = "CALL getChatBySequence(%0:seq)";


    _queryMap["setTimezone"] = "\
      SET time_zone=%0q:zonename";

    _queryMap["getCoordinatesBySourceList"] = "\
  SELECT c.source, \
         t.latitude, \
         t.longitude \
    FROM callsign c \
         INNER JOIN lastposition t ON c.id=t.callsign_id \
   WHERE c.source IN (%0:sourceList)";

    _queryMap["getPositionsBySourceList"] = "\
  SELECT t.latitude, \
         t.longitude, \
         pa.body AS path, \
         t.create_ts \
  FROM callsign c \
       LEFT JOIN position t ON c.id=t.callsign_id \
       LEFT JOIN path pa ON pa.id=t.path_id \
  WHERE c.source IN (%0:sourceList) \
    AND t.packet_date >= NOW() - INTERVAL - %1:age";

    _queryMap["getMemPositionsBySourceList"] = "\
  SELECT c.source, \
         t.latitude, \
         t.longitude, \
         UNIX_TIMESTAMP(t.packet_date) AS create_ts \
    FROM callsign c \
         LEFT JOIN mem_position t ON c.id=t.callsign_id \
   WHERE c.source IN (%0:sourceList) \
     AND t.packet_date >= NOW() - INTERVAL %1:age \
   ORDER BY source, packet_date";

    try {
      _memcached = new openframe::MemcachedController("localhost");
      _memcached->expire(86400);
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch

  } // DBI_Web::DBI_Web

  DBI_Web::~DBI_Web() {
    _queryMap.clear();
  } // DBI_Web::~DBI_Web

  const DBI_Web::resultSizeType DBI_Web::getLastpositionsByCoordinates(const BoundingBox &bb, const std::string &age, const int limit, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getMemLastpositionsByCoordinates"]);
    query.parse();

    try {
      res = query.store(bb.begin().lat(), bb.begin().lng(), bb.end().lat(), bb.end().lng(), age, limit);
      _debug(query.str(bb.begin().lat(), bb.begin().lng(), bb.end().lat(), bb.end().lng(), age, limit));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(bb.begin().lat(), bb.begin().lng(), bb.end().lat(), bb.end().lng(), age, limit));
      _errorf("DBI_Web::getLastpositionsByCoordinates #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(bb.begin().lat(), bb.begin().lng(), bb.end().lat(), bb.end().lng(), age, limit));
      _errorf("DBI_Web::getLastpositionsByCoordinates %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getLastpositionsByCoordinates

  const DBI_Web::resultSizeType DBI_Web::getSavedLastpositionsByCoordinates(const int save_id, const BoundingBox &bb, const std::string &age, const int limit, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getSavedLastpositionsByCoordinates"]);
    query.parse();

    try {
      res = query.store(save_id, bb.begin().lat(), bb.begin().lng(), bb.end().lat(), bb.end().lng(), age, limit);
      _debug(query.str(save_id, bb.begin().lat(), bb.begin().lng(), bb.end().lat(), bb.end().lng(), age, limit));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(save_id, bb.begin().lat(), bb.begin().lng(), bb.end().lat(), bb.end().lng(), age, limit));
      _errorf("DBI_Web::getSavedLastpositionsByCoordinates #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(save_id, bb.begin().lat(), bb.begin().lng(), bb.end().lat(), bb.end().lng(), age, limit));
      _errorf("DBI_Web::getSavedLastpositionsByCoordinates %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getSavedLastpositionsByCoordinates

  const DBI_Web::resultSizeType DBI_Web::getPositionsBySource(const std::string &source, const std::string &age, const int limit, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getPositionsBySource"]);
    query.parse();

    try {
      res = query.store(source, age, limit);
//      _debug(query.str(source, age, limit));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(source, age, limit));
      _errorf("DBI_Web::getPositionsBySource #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(source, age, limit));
      _errorf("DBI_Web::getPositionsBySource %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getPositionsBySource

  const DBI_Web::resultSizeType DBI_Web::getMemPositionsBySource(const std::string &source, const std::string &age, const int limit, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getMemPositionsBySource"]);
    query.parse();

    try {
      res = query.store(source, age, limit);
//      _debug(query.str(source, age, limit));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(source, age, limit));
      _errorf("DBI_Web::getPositionsBySource #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(source, age, limit));
      _errorf("DBI_Web::getPositionsBySource %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getMemPositionsBySource

  const DBI_Web::resultSizeType DBI_Web::getPositionsBySourceList(const std::string &sourceList, const std::string &age, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getMemPositionsBySourceList"]);
    query.parse();

    try {
      res = query.store(sourceList, age);
//      _debug(query.str(sourceList, age));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(sourceList, age));
      _errorf("DBI_Web::getPositionsBySourceList #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(sourceList, age));
      _errorf("DBI_Web::getPositionsBySourceList %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getPositionsBySourceList

  const DBI_Web::resultSizeType DBI_Web::getSavedPositionsBySource(const int save_id, const std::string &source, const std::string &age, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getSavedPositionsBySource"]);
    query.parse();

    try {
      res = query.store(save_id, source, age);
//      _debug(query.str(save_id, source, age));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(save_id, source, age));
      _errorf("DBI_Web::getSavedPositionsBySource #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(save_id, source, age));
      _errorf("DBI_Web::getSavedPositionsBySource %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getSavedPositionsBySource

  const DBI_Web::resultSizeType DBI_Web::getLastpositionBySource(const std::string &source, const std::string &age, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getLastpositionBySource"]);
    query.parse();

    try {
      res = query.store(source);
//      _debug(query.str(source, past, limit));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(source));
      _errorf("DBI_Web::getLastpositionBySource #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(source));
      _errorf("DBI_Web::getLastpositionBySource %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getLastpositionBySource

  const DBI_Web::resultSizeType DBI_Web::getLastpositionByName(const std::string &name, const std::string &age, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getLastpositionByName"]);
    query.parse();

    try {
      res = query.store(name);
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(name));
      _errorf("DBI_Web::getLastpositionByName #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(name));
      _errorf("DBI_Web::getLastpositionByName %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getLastpositionByName

  const DBI_Web::resultSizeType DBI_Web::getSavedLastpositionBySource(const int save_id, const std::string &source, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getSavedLastpositionBySource"]);
    query.parse();

    try {
      res = query.store(save_id, source);
      _debug(query.str(save_id, source));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(save_id, source));
      _errorf("DBI_Web::getSavedLastpositionBySource #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(save_id, source));
      _errorf("DBI_Web::getSavedLastpositionBySource %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getSavedLastpositionBySource

  const DBI_Web::resultSizeType DBI_Web::getPositionBySourceAndTime(const std::string &source, const time_t seconds, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getPositionBySourceAndTime"]);
    query.parse();

    try {
      res = query.store(source, seconds);
//      _debug(query.str(source, seconds));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(source, seconds));
      _errorf("DBI_Web::getPositionBySourceAndTime #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(source, seconds));
      _errorf("DBI_Web::getPositionBySourceAndTime %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getPositionBySourceAndTime

  const DBI_Web::resultSizeType DBI_Web::getSavedPositionBySourceAndTime(const int save_id, const std::string &source, const time_t seconds, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getSavedPositionBySourceAndTime"]);
    query.parse();

    try {
      res = query.store(save_id, source, seconds);
//      _debug(query.str(save_id, source, seconds));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(save_id, source, seconds));
      _errorf("DBI_Web::getSavedPositionBySourceAndTime #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(save_id, source, seconds));
      _errorf("DBI_Web::getSavedPositionBySourceAndTime %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getSavedPositionBySourceAndTime

  const DBI_Web::resultSizeType DBI_Web::getWeatherStringBySource(const std::string &source, const std::string &units, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getWeatherStringBySource"]);
    query.parse();

    try {
      res = query.store(source, units);
//      _debug(query.str(source, units));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(source, units));
      _errorf("DBI_Web::getWeatherStringBySource #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(source, units));
      _errorf("DBI_Web::getWeatherStringBySource %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getWeatherStringBySource

  const DBI_Web::resultSizeType DBI_Web::getCoordinatesByPath(std::string path, resultType &res) {
    mysqlpp::Query query = _sqlpp->query("SELECT t.latitude, t.longitude FROM callsign c INNER JOIN lastposition t ON c.id=t.callsign_id WHERE c.source IN (");
    digilistType digilist;

    if (getDigilistByPath(path, digilist) < 1)
      return 0;

    size_t numAdded = 0;
    for(size_t i=0; i < digilist.size(); i++) {
      if (numAdded > 0)
        query << ",";

      query << mysqlpp::quote << digilist[i];
      numAdded++;
    } // for
    query << ")";

    if (numAdded < 1)
      return 0;

    query.parse();

    try {
      res = query.store();
//      _debug(query.str());
    } // try
    catch(mysqlpp::BadQuery e) {
      // _error(query.str());
      _errorf("DBI_Web::getCoordinatesByPath %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      // _error(query.str());
      _errorf("DBI_Web::getCoordinatesByPath %s", e.what());
      return 0;
    } // catch

    return res.num_rows();
  } // DBI_Web::getCoordinatesByPath

  const DBI_Web::resultSizeType DBI_Web::getCoordinatesBySourceList(const std::string &sourceList, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getCoordinatesBySourceList"]);
    query.parse();

    try {
      res = query.store(sourceList);
//      _debug(query.str(sourceList));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(sourceList));
      _errorf("DBI_Web::getCoordinatesBySourceList #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(sourceList));
      _errorf("DBI_Web::getCoordinatesBySourceList %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getCoordinatesBySourceList

  const DBI_Web::resultSizeType DBI_Web::getCoordinatesBySource(const std::string &source, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getCoordinatesBySource"]);

    query.parse();

    try {
      res = query.store(source);
//      _debug(query.str(source));
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(source));
      _errorf("DBI_Web::getCoordinatesBySource %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      // _error(query.str());
      _errorf("DBI_Web::getCoordinatesBySource %s", e.what());
      return 0;
    } // catch

    return res.num_rows();
  } // DBI_Web::getCoordinatesBySource

  const DBI_Web::digilistType::size_type DBI_Web::getDigilistByPath(std::string path, digilistType &digilist) {
    openframe::StringToken st;

    digilist.clear();

    openframe::StringTool::replace("*", "", path);
    st.setDelimiter(',');
    st = path;
    if (st.size() < 1)
      return 0;

    openframe::StringTool::regexMatchListType rl;
    for(size_t i=0; i < st.size(); i++) {
      if (openframe::StringTool::ereg("^(WIDE.*|qA.*|APRS.*|TCPIP.*|TCPXX|CORE.*|CWOP.*|FIRST|SECOND|THIRD|FOURTH|BEACON|AP.*)", st[i], rl))
        continue;

      digilist.push_back(st[i]);
    } // for

    return digilist.size();
  } // DBI_Web::getDigilistByPath

  const bool DBI_Web::setDigiCoordinatesInMemcached(const std::string &source, const std::string &lat, const std::string &lng) {
    std::string key = openframe::StringTool::toUpper(source);
    bool isOK = true;

    openframe::Serialize s;
    s.add(lat);
    s.add(lng);

    try {
      _memcached->put("digicoords", key, s.compile());
    } // try
    catch(openframe::MemcachedController_Exception e) {
      _error(e.message());
      isOK = false;
    } // catch

    return isOK;
  } // DBI_Web::setDigiCoordinatesInMemcached

  const bool DBI_Web::setDigiCoordinatesInMemcached(const std::string &source) {
    std::string key = openframe::StringTool::toUpper(source);
    bool isOK = true;

    openframe::Serialize s;
    s.add("1");

    try {
      _memcached->put("digicoords", key, s.compile());
    } // try
    catch(openframe::MemcachedController_Exception e) {
      _error(e.message());
      isOK = false;
    } // catch

    return isOK;
  } // DBI_Web::setDigiCoordinatesInMemcached

  const bool DBI_Web::getDigiCoordinatesFromMemcached(const std::string &source, std::string &lat, std::string &lng, bool &fixed) {
    openframe::MemcachedController::memcachedReturnEnum mcr;
    std::string buf;
    std::string key;

    try {
      mcr = _memcached->get("digicoords", openframe::StringTool::toUpper(source), buf);
    } // try
    catch(openframe::MemcachedController_Exception e) {
      _error(e.message());
    } // catch

    if (mcr != openframe::MemcachedController::MEMCACHED_CONTROLLER_SUCCESS)
      return false;

    openframe::Serialize s = buf;

    if (s.size() == 1) {
      fixed = false;
      return true;
    } // if

    if (s.size() != 2)
      return false;

    lat = s[0];
    lng = s[1];
    fixed = true;

    return true;
  } // DBI_Web::getDigiCoordinatesFromMemcached

  const DBI_Web::resultSizeType DBI_Web::getUlsByCallsign(const std::string &callsign, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getUlsByCallsign"]);
    query.parse();

    try {
      res = query.store(callsign);
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(callsign));
      _errorf("DBI_Web::getUlsByCallsign #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(callsign));
      _errorf("DBI_Web::getUlsByCallsign %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getUlsByCallsign


  const DBI_Web::resultSizeType DBI_Web::getUlsByUsi(const std::string &usi, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getUlsByUsi"]);
    query.parse();

    try {
      res = query.store(usi);
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(usi));
      _errorf("DBI_Web::getUlsByUsi #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(usi));
      _errorf("DBI_Web::getUlsByUsi %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getUlsByUsi

  const DBI_Web::resultSizeType DBI_Web::getViewByHash(const std::string &hash, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getViewByHash"]);
    query.parse();

    try {
      res = query.store(hash);
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(hash));
      _errorf("DBI_Web::getViewByHash #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      _error(query.str(hash));
      _errorf("DBI_Web::getViewByHash %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getViewByHash

  const DBI_Web::resultSizeType DBI_Web::getLastsequenceFromWebChat(resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getLastsequenceFromWebChat"]);
    query.parse();

    try {
      res = query.store();
//      _debug(query.str());
    } // try
    catch(mysqlpp::BadQuery e) {
      // _error(query.str());
      _errorf("DBI_Web::getLastsequenceFromWebChat #%d %s", e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      // _error(query.str());
      _errorf("DBI_Web::getLastsequenceFromWebChat %s", e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::getLastsequenceFromWebChat

  const DBI_Web::simpleResultSizeType DBI_Web::setWebWho(const std::string &hash, const std::string &server, const std::string &ip,
                                                         const std::string &nick, const std::string &timezone, const std::string &offset_ts,
                                                         const std::string &idle) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["setWebWho"]);
    simpleResultType res;
    simpleResultSizeType num = 0;
    query.parse();

    try {
      res = query.execute();
      num = res.rows();
    } // try
    catch(mysqlpp::BadQuery e) {
      // _error(query.str());
      _errorf("*** SQL Error: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return num;
  } // DBI_Web::setWebWho

  const DBI_Web::simpleResultSizeType DBI_Web::setTimezone(const std::string &zone) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["setTimezone"]);
    simpleResultType res;
    simpleResultSizeType num = 0;
    query.parse();

    try {
      res = query.execute(zone);
      num = res.rows();
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(zone));
      _errorf("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::setTimezone

  const DBI_Web::simpleResultSizeType DBI_Web::set(const std::string &q,
                                                   const std::string &str0,
                                                   const std::string &str1,
                                                   const std::string &str2,
                                                   const std::string &str3,
                                                   const std::string &str4,
                                                   const std::string &str5,
                                                   const std::string &str6,
                                                   const std::string &str7,
                                                   const std::string &str8,
                                                   const std::string &str9
                                                  ) {
    mysqlpp::Query query = _sqlpp->query(_queryMap[q]);
    simpleResultType res;
    simpleResultSizeType num = 0;
    query.parse();

    try {
      res = query.execute(str0, str1, str2, str3, str4, str5, str6, str7, str8, str9);
      num = res.rows();
    } // try
    catch(mysqlpp::BadQuery e) {
      _error(query.str(str0, str1, str2, str3, str4, str5, str6, str7, str8, str9));
      _errorf("*** SQL Error: %s #%d %s", q.c_str(), e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::set

  const DBI_Web::resultSizeType DBI_Web::get(const std::string q,
                                             resultType &res,
                                             const std::string &str0,
                                             const std::string &str1,
                                             const std::string &str2,
                                             const std::string &str3,
                                             const std::string &str4,
                                             const std::string &str5,
                                             const std::string &str6,
                                             const std::string &str7,
                                             const std::string &str8,
                                             const std::string &str9
                                            ) {
    mysqlpp::Query query = _sqlpp->query(_queryMap[q]);
    query.parse();

    try {
      res = query.store(str0, str1, str2, str3, str4, str5, str6, str7, str8, str9);
    } // try
    catch(mysqlpp::BadQuery e) {
      // _error(query.str());
      _errorf("DBI_Web::get %s #%d %s", q.c_str(), e.errnum(), e.what());
      return 0;
    } // catch
    catch(mysqlpp::BadConversion e) {
      // _error(query.str());
      _errorf("DBI_Web::get %s %s", q.c_str(), e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Web::get

/*
  const DBI_Web::resultSizeType DBI_Web::getReporterById(const std::string &table, const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["reporterById"]);
    query.parse();

    try {
      res = query.store(table, id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getRepoterById

  const DBI_Web::resultSizeType DBI_Web::getClusterById(const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["clusterById"]);
    query.parse();

    try {
      res = query.store(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getClusterById

  const DBI_Web::resultSizeType DBI_Web::getSigsForClusterById(const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["sigsForClusterById"]);
    query.parse();

    try {
      res = query.store(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getSigsForClusterById

  const DBI_Web::resultSizeType DBI_Web::getMnosForClusterById(const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["mnosForClusterById"]);
    query.parse();

    try {
      res = query.store(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getMnosForClusterById

  const DBI_Web::resultSizeType DBI_Web::getReportsForClusterById(const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["reportsForClusterById"]);
    query.parse();

    try {
      res = query.store(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getReportsForClusterById

  const DBI_Web::resultSizeType DBI_Web::getReportersForClusterById(const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["reportersForClusterById"]);
    query.parse();

    try {
      res = query.store(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getReportersForClusterById

  const DBI_Web::resultSizeType DBI_Web::getSigsForSpamByMessageMetaId(const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["sigsForSpamByMessageMetaId"]);
    query.parse();

    try {
      res = query.store(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getSigsForSpamByMessageMetaId

  const DBI_Web::resultSizeType DBI_Web::getSigById(const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["sigById"]);
    query.parse();

    try {
      res = query.store(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getSigById

  const DBI_Web::simpleResultSizeType DBI_Web::updateSignatureMessageActiveById(const std::string &id, const int active) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["updateSignatureMessageActiveById"]);
    simpleResultType res;
    query.parse();

    try {
      res = query.execute(id, active);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.rows();
  } // DBI_Web::getSigById

  const DBI_Web::resultSizeType DBI_Web::getClassByName(const std::string &name, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getClassByName"]);
    query.parse();

    try {
      res = query.store(name);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getClassByName

  const DBI_Web::resultSizeType DBI_Web::getActiveMnoById(const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getActiveMnoById"]);
    query.parse();

    try {
      res = query.store(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getActiveMnoById

  const DBI_Web::resultSizeType DBI_Web::getActiveMnoByRand(resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getActiveMnoByRand"]);
    query.parse();

    try {
      res = query.store();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getActiveMnoById

  const DBI_Web::resultSizeType DBI_Web::getMessageForMnoById(const std::string &id, const std::string &where, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getMessageForMnoById"]);
    query.parse();

    try {
      res = query.store(id, where);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getMessageForMnoByMnoRand

  const DBI_Web::resultSizeType DBI_Web::getSignatureByRand(resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getSignatureByRand"]);
    query.parse();

    try {
      res = query.store();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getSignatureByRand

  const DBI_Web::resultSizeType DBI_Web::getClusters(resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getClusters"]);
    query.parse();

    try {
      res = query.store();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getClusters

  const DBI_Web::resultSizeType DBI_Web::getReportsForReporterById(const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["reportsForReporterById"]);
    query.parse();

    try {
      res = query.store(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getReportsForReporterById

  const DBI_Web::resultSizeType DBI_Web::getMessageById(const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getMessageById"]);
    query.parse();

    try {
      res = query.store(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getMessageById

  const DBI_Web::resultSizeType DBI_Web::getMessageMetaByMessageId(const std::string &id, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getMessageMetaByMessageId"]);
    query.parse();

    try {
      res = query.store(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getMessageMetaByMessageId

  const DBI_Web::resultSizeType DBI_Web::getBodyByHash(const std::string &hash, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(_queryMap["getBodyByHash"]);
    query.parse();

    try {
      res = query.store(hash);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI_Web::getBodyByHash

  const DBI_Web::resultSizeType DBI_Web::clearClusters() {
    mysqlpp::Query query = _sqlpp->query(_queryMap["clearClusters"]);
    mysqlpp::Query query_c = _sqlpp->query(_queryMap["countClusters"]);
    resultType res;
    resultSizeType num = 0;
    query.parse();

    try {
      res = query_c.store();
      if (res.size())
        num = atoi(res[0][0].c_str());
      query.execute();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::clearClusters

  const DBI_Web::resultSizeType DBI_Web::clearSignatureClusters() {
    mysqlpp::Query query = _sqlpp->query(_queryMap["clearSignatureClusters"]);
    mysqlpp::Query query_c = _sqlpp->query(_queryMap["countSignatureClusters"]);
    resultType res;
    resultSizeType num = 0;
    query.parse();

    try {
      res = query_c.store();
      if (res.num_rows())
        num = atoi(res[0][0].c_str());
      query.execute();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::clearSignatureClusters

  const DBI_Web::resultSizeType DBI_Web::clearSignatures() {
    mysqlpp::Query query = _sqlpp->query(_queryMap["clearSignatures"]);
    mysqlpp::Query query_c = _sqlpp->query(_queryMap["countSignatures"]);
    resultType res;
    resultSizeType num = 0;
    query.parse();

    try {
      res = query_c.store();
      if (res.size())
        num = atoi(res[0][0].c_str());
      query.execute();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::clearClusters

  const DBI_Web::resultSizeType DBI_Web::clearSignatureMessages() {
    mysqlpp::Query query = _sqlpp->query(_queryMap["clearSignatureMessages"]);
    mysqlpp::Query query_c = _sqlpp->query(_queryMap["countSignatureMessages"]);
    resultType res;
    resultSizeType num = 0;
    query.parse();

    try {
      res = query_c.store();
      if (res.num_rows())
        num = atoi(res[0][0].c_str());
      query.execute();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::clearSignatureMessages

  const DBI_Web::resultSizeType DBI_Web::clearMessageMeta() {
    mysqlpp::Query query = _sqlpp->query(_queryMap["clearMessageMeta"]);
    mysqlpp::Query query_c = _sqlpp->query(_queryMap["countMessageMeta"]);
    resultType res;
    resultSizeType num = 0;
    query.parse();

    try {
      res = query_c.store();
      if (res.num_rows())
        num = atoi(res[0][0].c_str());
      query.execute();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::clearMessageMeta

  const DBI_Web::resultSizeType DBI_Web::clearMessages() {
    mysqlpp::Query query = _sqlpp->query(_queryMap["clearMessages"]);
    mysqlpp::Query query_c = _sqlpp->query(_queryMap["countMessages"]);
    resultType res;
    resultSizeType num = 0;
    query.parse();

    try {
      res = query_c.store();
      if (res.num_rows())
        num = atoi(res[0][0].c_str());
      query.execute();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::clearMessageMeta

  const DBI_Web::resultSizeType DBI_Web::clearReports() {
    mysqlpp::Query query = _sqlpp->query(_queryMap["clearReports"]);
    mysqlpp::Query query_c = _sqlpp->query(_queryMap["countReports"]);
    resultType res;
    resultSizeType num = 0;
    query.parse();

    try {
      res = query_c.store();
      if (res.num_rows())
        num = atoi(res[0][0].c_str());
      query.execute();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::clearReports

  const DBI_Web::resultSizeType DBI_Web::clearReporters() {
    mysqlpp::Query query = _sqlpp->query(_queryMap["clearReporters"]);
    mysqlpp::Query query_c = _sqlpp->query(_queryMap["countReporters"]);
    resultType res;
    resultSizeType num = 0;
    query.parse();

    try {
      res = query_c.store();
      if (res.num_rows())
        num = atoi(res[0][0].c_str());
      query.execute();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::clearReporters

  const DBI_Web::resultSizeType DBI_Web::clearSenders() {
    mysqlpp::Query query = _sqlpp->query(_queryMap["clearSenders"]);
    mysqlpp::Query query_c = _sqlpp->query(_queryMap["countSenders"]);
    resultType res;
    resultSizeType num = 0;
    query.parse();

    try {
      res = query_c.store();
      if (res.num_rows())
        num = atoi(res[0][0].c_str());
      query.execute();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::clearSenders

  const DBI_Web::resultSizeType DBI_Web::clearBodies() {
    mysqlpp::Query query = _sqlpp->query(_queryMap["clearBodies"]);
    mysqlpp::Query query_c = _sqlpp->query(_queryMap["countBodies"]);
    resultType res;
    resultSizeType num = 0;
    query.parse();

    try {
      res = query_c.store();
      if (res.num_rows())
        num = atoi(res[0][0].c_str());
      query.execute();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return num;
  } // DBI_Web::clearBodies
*/
} // namespace web
