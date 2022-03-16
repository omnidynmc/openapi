#ifndef OPENAPI_OPENAPI_LOG_H
#define OPENAPI_OPENAPI_LOG_H

#include <iostream>
#include <iomanip>
#include <string>

#include <openframe/OpenFrame_Log.h>

namespace openapi {

  class OpenAPI_Log : public openframe::OpenFrame_Log {
    public:
      OpenAPI_Log(const std::string &ident) : OpenFrame_Log(ident) { }
      virtual ~OpenAPI_Log() { }

      virtual void fcgi(const std::string &message) {
        std::cout << "Abstract FCGI: " << std::endl;
      } // log

    protected:
    private:

  }; // OpenAPI_Log

} // openapi

#endif
