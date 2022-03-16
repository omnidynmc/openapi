/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Daniel Robert Karrels                             **
 **                    Dynamic Networking Solutions                      **
 **                                                                      **
 ** This program is free software; you can redistribute it and/or modify **
 ** it under the terms of the GNU General Public License as published by **
 ** the Free Software Foundation; either version 1, or (at your option)  **
 ** any later version.                                                   **
 **                                                                      **
 ** This program is distributed in the hope that it will be useful,      **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of       **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with this program; if not, write to the Free Software          **
 ** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            **
 **************************************************************************
 $Id: EarthTools.h,v 1.1 2005/11/21 18:16:02 omni Exp $
 **************************************************************************/

#ifndef __OPENAPI_EARTHTOOLS_H
#define __OPENAPI_EARTHTOOLS_H

#include <string>
#include <map>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "openapi.h"

namespace openapi {
/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class EarthTools {
    public:
      EarthTools();				// constructor
      virtual ~EarthTools();			// destructor

      // ### Type Definitions ###
      enum conversionEnumType {
        CONVERSION_KNOTSTOKPH,
        CONVERSION_KPHTOKNOTS,
        CONVERSION_KPHTOMPH,
        CONVERSION_MPHTOKPH,
        CONVERSION_KPHTOMS,
        CONVERSION_MPHTOMS,
        CONVERSION_INCHTOMM,
        CONVERSION_MMTOINCH,
        CONVERSION_METERTOFOOT,
        CONVERSION_FOOTTOMETER,
        CONVERSION_NONE
      }; // conversionEnumType

      enum systemEnumType {
        SYSTEM_METRIC,
        SYSTEM_IMPERIAL,
        SYSTEM_NAUTICAL,
        SYSTEM_UNKNOWN
      }; // systemEnumType

      enum unitEnumType {
        UNIT_SPEED,
        UNIT_VOLUME,
        UNIT_DISTANCE,
        UNIT_RULER,
        UNIT_UNKNOWN
      }; // unitEnumType

      static const double distance(double, double, double, double, char);
      static double deg2rad(double);
      static double rad2deg(double);
      static const bool isValidLatLong(const double, const double);
      static const double speed(const double, const time_t, const int, const int);
      static const std::string dec2nmea(const double, const bool);
      static const double conversion(const conversionEnumType);
      static const double round(const double, const unsigned int);
      static const double convert(const double, const conversionEnumType, const unsigned int);
      static const systemEnumType findSystemType(const std::string &);
      static const unitEnumType findUnitType(const std::string &);
      static const std::string convertBySystem(const double, const std::string &, const std::string &,
                                          const unsigned int);

    public:
    protected:
    private:
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

#if 0
  #define KnotToKmh(x)			(x * 1.852);
  #define	MphToKmh(x)			(x * 1.609344);
  #define KmhToMs(x)			(x * 10 / 36);
  #define MphToMs(x)			(x * 1.609344 * 10 / 36);
  #define InchToMm(x)			(x * 0.254);
#endif

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

int axtoi(const char *);

}
#endif
