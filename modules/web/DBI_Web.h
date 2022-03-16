#ifndef MODULE_WEB_DBI_WEB_H
#define MODULE_WEB_DBI_WEB_H

#include <map>
#include <string>

#include <mysql++.h>

#include <openframe/MemcachedController.h>

#include "App_Log.h"
#include "DBI.h"

namespace modweb {
/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  class Coordinates {
    public:
      Coordinates(const double lat, const double lng) : m_lat(lat), m_lng(lng) { }
      ~Coordinates() { }

      const double lat() const { return m_lat; }
      const double lng() const { return m_lng; }

    protected:
    private:
      double m_lat;
      double m_lng;
  }; // class Coordinates

  class BoundingBox {
    public:
      BoundingBox(const Coordinates &begin, const Coordinates &end) : m_begin(begin), m_end(end) { }
      ~BoundingBox() { }

      const Coordinates begin() const { return m_begin; }
      const Coordinates end() const { return m_end; }

    protected:
    private:
      Coordinates m_begin;
      Coordinates m_end;
  }; // class Coordinates

  class DBI_Web : public openapi::DBI {
    public:
      DBI_Web(openapi::App_Log *, const std::string &, const std::string &, const std::string &, const std::string &);
      virtual ~DBI_Web();

      // ### Type Definitions ###
      typedef std::vector<std::string> digilistType;

      // ### Public Members ###
      const resultSizeType getLastpositionsByCoordinates(const BoundingBox &, const std::string &age, const int, resultType &);
      const resultSizeType getSavedLastpositionsByCoordinates(const int, const BoundingBox &, const std::string &age, const int, resultType &);
      const resultSizeType getLastpositionBySource(const std::string &, const std::string &, resultType &);
      const resultSizeType getLastpositionByName(const std::string &, const std::string &, resultType &);
      const resultSizeType getSavedLastpositionBySource(const int, const std::string &, resultType &);
      const resultSizeType getPositionBySourceAndTime(const std::string &, const time_t, resultType &);
      const resultSizeType getSavedPositionBySourceAndTime(const int, const std::string &, const time_t, resultType &);
      const resultSizeType getPositionsBySource(const std::string &, const std::string &, const int, resultType &);
      const resultSizeType getMemPositionsBySource(const std::string &, const std::string &, const int, resultType &);
      const resultSizeType getPositionsBySourceList(const std::string &, const std::string &, resultType &);
      const resultSizeType getSavedPositionsBySource(const int, const std::string &, const std::string &, resultType &);
      const resultSizeType getWeatherStringBySource(const std::string &, const std::string &, resultType &);
      const resultSizeType getCoordinatesByPath(std::string, resultType &);
      const resultSizeType getCoordinatesBySource(const std::string &, resultType &);
      const resultSizeType getCoordinatesBySourceList(const std::string &, resultType &);
      const resultSizeType getUlsByCallsign(const std::string &, resultType &);
      const resultSizeType getUlsByUsi(const std::string &, resultType &);
      const resultSizeType getViewByHash(const std::string &, resultType &);
      const resultSizeType getLastsequenceFromWebChat(resultType &);
      const simpleResultSizeType setWebWho(const std::string &, const std::string &, const std::string &,
                                           const std::string &, const std::string &, const std::string &,
                                           const std::string &);
      const bool setDigiCoordinatesInMemcached(const std::string &source);
      const bool setDigiCoordinatesInMemcached(const std::string &source, const std::string &lat, const std::string &lng);
      const bool getDigiCoordinatesFromMemcached(const std::string &source, std::string &lat, std::string &lng, bool &);
      const digilistType::size_type getDigilistByPath(std::string, digilistType &);
      const simpleResultSizeType setTimezone(const std::string &zone);

      const simpleResultSizeType set(const std::string &q,
                                     const std::string &str0="",
                                     const std::string &str1="",
                                     const std::string &str2="",
                                     const std::string &str3="",
                                     const std::string &str4="",
                                     const std::string &str5="",
                                     const std::string &str6="",
                                     const std::string &str7="",
                                     const std::string &str8="",
                                     const std::string &str9=""
                                    );

      const resultSizeType get(const std::string q,
                               resultType &res,
                               const std::string &str0="",
                               const std::string &str1="",
                               const std::string &str2="",
                               const std::string &str3="",
                               const std::string &str4="",
                               const std::string &str5="",
                               const std::string &str6="",
                               const std::string &str7="",
                               const std::string &str8="",
                               const std::string &str9=""
                              );

    protected:

    private:
      openframe::MemcachedController *_memcached;
      queryMapType _queryMap;
  }; // DBI_Web

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace modweb

#endif
