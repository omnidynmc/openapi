#ifndef __MODULE_WEB_WEBCOMMANDS_H
#define __MODULE_WEB_WEBCOMMANDS_H

#include <list>

#include <time.h>
#include <sys/time.h>

#include <openframe/openframe.h>

namespace modweb {
  using namespace openframe;
/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

CREATE_COMMAND(fcgiBALLOON);
CREATE_COMMAND(fcgiCHAT);
CREATE_COMMAND(fcgiHNYBALLOON);
CREATE_COMMAND(fcgiLASTPOSITION);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace modweb
#endif
