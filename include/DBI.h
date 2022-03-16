#ifndef OPENTEST_DBI_H
#define OPENTEST_DBI_H

#include <string>
#include <map>

#include <mysql++.h>

#include "App_Log.h"
#include "OpenAPI_Abstract.h"

namespace openapi {
  using std::string;
  using std::map;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define DBI_MAX(a, b)		(a > b ? a : b)

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class DBI : public OpenAPI_Abstract {
    public:
      DBI(App_Log *, const string &, const string &, const string &, const string &);
      virtual ~DBI();

      // ### Type Definitions ###
      typedef mysqlpp::StoreQueryResult::list_type::size_type resultSizeType;
      typedef mysqlpp::StoreQueryResult resultType;
      typedef unsigned long long simpleResultSizeType;
      typedef mysqlpp::SimpleResult simpleResultType;
      typedef mysqlpp::Query queryType;
      typedef map<string, string> queryMapType;

      mysqlpp::Connection *handle() { return _sqlpp; }

      static void print(resultType &res);
      static void print(const string &, resultType &res);
      static void makeListFromField(const string &, resultType &, string &);
      static const string safe(const string &);
      static const bool lineForNewlineRow(const string &, string &, const size_t);
      static const size_t maxNewlineLen(const string &);
      const resultSizeType query(const string &, resultType &);

    protected:
      // ### Protected Members ###
      const bool _connect();

      // ### Protected Variables ###
      mysqlpp::Connection *_sqlpp;
      string _db;
      string _host;
      string _user;
      string _pass;

    private:

  }; // OpenAPI_Abstract

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // openapi

#endif
