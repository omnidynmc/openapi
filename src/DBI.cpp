#include <config.h>

#include <new>
#include <string>

#include <mysql++.h>

#include <openframe/openframe.h>

#include "App_Log.h"
#include "DBI.h"

namespace openapi {
  DBI::DBI(App_Log *log, const std::string &db, const std::string &host, const std::string &user, const std::string &pass) :
           OpenAPI_Abstract(log), _db(db), _host(host), _user(user), _pass(pass) {

    try {
      _sqlpp = new mysqlpp::Connection(true);
      _sqlpp->set_option(new mysqlpp::ReconnectOption(true));
      _sqlpp->set_option(new mysqlpp::MultiResultsOption(true));
      _sqlpp->set_option(new mysqlpp::SetCharsetNameOption("utf8"));
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch

    _connect();

  } // DBI

  DBI::~DBI() {
    if (_sqlpp->connected())
      _sqlpp->disconnect();

    delete _sqlpp;
  } // DBI:~DBI

  const bool DBI::_connect() {
    try {
      _sqlpp->connect(_db.c_str(), _host.c_str(), _user.c_str(), _pass.c_str(), 3306);
    } // try
    catch(mysqlpp::ConnectionFailed e) {
      _errorf("*** SQL Error: #%d %s", e.errnum(), e.what());
      return false;
    } // catch

    return true;
  } // DBI::_connect

  const DBI::resultSizeType DBI::query(const std::string &sqls, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(sqls);

    try {
      res = query.store();
    } // try
    catch(mysqlpp::BadQuery e) {
      _errorf("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    return res.num_rows();
  } // DBI::query

  void DBI::print(resultType &res) {
    map<size_t, size_t> maxFieldLen;
    resultSizeType j;
    size_t maxRowLen = 0;
    size_t fieldLen = 0;
    size_t i;
    std::string sep = "";
    std::string fieldSep = "";
    std::string field;
    std::string value;

    for(i=0; i < res.num_fields(); i++) {
      // populate map with head size
      fieldLen = res.field_name(i).length();
      maxFieldLen[i] = fieldLen;
    } // for

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        value = safe(res[j][i].c_str());
        maxFieldLen[i] = DBI_MAX(maxFieldLen[i], DBI::maxNewlineLen(value) );
      } // for
    } // for

    // create the sep
    for(i=0; i < maxFieldLen.size(); i++) {
      fieldSep = "+";
      openframe::StringTool::pad(fieldSep, "-", maxFieldLen[i]+3);
      sep += fieldSep;
      maxRowLen += maxFieldLen[i];
    } // for
    sep += "+";

    maxRowLen += (maxFieldLen.size()-1) * 3;

    std::cout << sep << std::endl;
    std::cout << "| ";
    for(i=0; i < res.num_fields(); i++) {
      field = res.field_name(i);
      openframe::StringTool::pad(field, " ", maxFieldLen[i]);
      if (i)
        std::cout << " | ";
      std::cout << field;
    } // for
    std::cout << " |" << std::endl;
    std::cout << sep << std::endl;

    // now one more loop to print out stuffs
    for(j=0; j < res.num_rows(); j++) {
      size_t newlines=0;
      for(size_t n=0; n == 0 || newlines; n++) {
        std::stringstream s;
        newlines=0;
        s << "| ";
        for(i=0; i < res.num_fields(); i++) {
          if (lineForNewlineRow(safe(res[j][i].c_str()), value, n))
            newlines++;

          openframe::StringTool::pad(value, " ", maxFieldLen[i]);
          if (i)
            s << " | ";
          s << value;
        } // for
        s << " |" << std::endl;

        if (newlines)
          std::cout << s.str();
      } // for
    } // for
    std::cout << sep << std::endl;
    std::cout << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set" << std::endl << std::endl;
  } // DBI::print

  const size_t DBI::maxNewlineLen(const std::string &buf) {
    openframe::StringToken st;
    size_t len = 0;

    st.setDelimiter('\n');
    st = buf;

    if (!st.size())
      return 0;

    for(size_t i=0; i < st.size(); i++)
      len = DBI_MAX(len, st[i].length());

    return len;
  } // DBI::maxNewlineLen

  const bool DBI::lineForNewlineRow(const std::string &buf, std::string &ret, const size_t i) {
    openframe::StringToken st;

    st.setDelimiter('\n');
    st = buf;

    if (i >= st.size()) {
      ret = "";
      return false;
    } // if

    ret = st[i];

    return true;
  } // DBI::lineForNewlineRow

  void DBI::print(const std::string &label, resultType &res) {
    std::string sep = "";
    std::string fieldSep = "";
    std::string name;
    std::string value;
    std::string l = "";
    size_t maxFieldLen = 0;
    size_t maxValueLen = 0;
    size_t i;
    resultSizeType j;

    for(i=0; i < res.num_fields(); i++)
      maxFieldLen = DBI_MAX(res.field_name(i).length(), maxFieldLen);

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        value = res[j][i].c_str();
        value = safe(value);
        maxValueLen = DBI_MAX(DBI::maxNewlineLen(value), maxValueLen);
      } // for
    } // for

    maxFieldLen += 1;
    maxValueLen += 1;

    sep = "+";
    openframe::StringTool::pad(sep, "-", maxFieldLen+maxValueLen+4);
    sep += "+";

    fieldSep = "+";
    openframe::StringTool::pad(fieldSep, "-", maxFieldLen+2);
    fieldSep += "+";
    openframe::StringTool::pad(fieldSep, "-", maxFieldLen+maxValueLen+4);
    fieldSep += "+";

    l = label;
    openframe::StringTool::pad(l, " ", maxFieldLen+maxValueLen+1);

    // loop through result
    std::cout << sep << std::endl;
    std::cout << "| " << l << " |" << std::endl;
    std::cout << fieldSep << std::endl;
    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        size_t newlines=0;
        for(size_t n=0; n == 0 || newlines; n++) {
          newlines=0;
          name = (n == 0 ? res.field_name(i) : "");
          if (DBI::lineForNewlineRow(safe(res[j][i].c_str()), value, n))
            newlines++;
          openframe::StringTool::pad(name, " ", maxFieldLen);
          openframe::StringTool::pad(value, " ", maxValueLen);
          if (newlines)
            std::cout << "| " << name << "| " << value << "|" << std::endl;
        } // for
      } // for
    } // for
    std::cout << fieldSep << std::endl;

  } // DBI::print

  const std::string DBI::safe(const std::string &buf) {
    std::stringstream s;

    s.str("");
    for(size_t i=0; i < buf.length(); i++) {
      if (((int) buf[i] < 32 || (int) buf[i] > 126) && (int) buf[i] != 10)
        s << "\\x" << openframe::StringTool::char2hex(buf[i]);
      else
        s << buf[i];
    } // for

    return s.str();
  } // DBI::safe

  void DBI::makeListFromField(const std::string &name, resultType &res, std::string &list) {
    resultSizeType j;

    list = "";

    for(j=0; j < res.num_rows(); j++) {
      if (list.length())
        list += ", ";
      list += res[j][name.c_str()].c_str();
    } // for
  } // DBI::makeListFromField
} // openapi
