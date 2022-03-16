/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
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
 $Id: EarthTools.cpp,v 1.2 2005/12/13 21:07:03 omni Exp $
 **************************************************************************/

#include <list>
#include <new>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include <openframe/openframe.h>

#include "EarthTools.h"

#include "config.h"
#include "openapi.h"

#define pi 3.14159265358979323846

namespace openapi {

/**
 * Distance Units
 *
 * Metres, Millimetres, Centimetres, Kilometres, Inches, Feet, Yards, Fathoms
 * 0       1            2            3           4       5     6      7
 * Statute Miles, Nautiecal Miles, Light Years (9460000000000000)
 * 8              9                10
 */

static double distunits[] = { 1, 0.001, 0.01, 1000, 0.0254, 0.3048, 0.9144,
                              1.8288, 1609, 1852, -1};

/**
 * Speed Units
 *
 * Metres/s, Kilometre/hr, Feet/s, Feet/m, Yard/m, Statute Mile/hr, Knot
 * 0         1             2       3       4       5                6
 */

static double speedunits[] = { 1, 0.277777777777777777777777777777777777,
                              0.3048, 0.00508, 0.01524, 0.44704, 0.514444444444, -1};


/**************************************************************************
 ** EarthTools Class                                                     **
 **************************************************************************/

  EarthTools::EarthTools() {
    // set defaults
  } // EarthTools::EarthTools

  EarthTools::~EarthTools() {
    // cleanup
  } // EarthTools::~EarthTools

  const double EarthTools::conversion(const conversionEnumType cnv) {
    double ret = -1.0;

    switch(cnv) {
      case CONVERSION_KNOTSTOKPH:
        ret = 1.852;
        break;
      case CONVERSION_KPHTOKNOTS:
        ret = 0.539956803;
        break;
      case CONVERSION_KPHTOMPH:
        ret = 0.621371192;
        break;
      case CONVERSION_MPHTOKPH:
        ret = 1.609344;
        break;
      case CONVERSION_KPHTOMS:
        ret = (10 / 36);
        break;
      case CONVERSION_MPHTOMS:
        ret = (1.609344 * 10 / 36);
        break;
      case CONVERSION_INCHTOMM:
        ret = 0.254;
        break;
      case CONVERSION_MMTOINCH:
        ret = 0.0393700787;
        break;
      case CONVERSION_METERTOFOOT:
        ret = 3.2808399;
        break;
      case CONVERSION_FOOTTOMETER:
        ret = 0.3048;
        break;
      case CONVERSION_NONE:
        ret = 1;
        break;
      default:
        assert(false);		// bug
        break;
    } // switch

    return ret;
  } // EarthTools::conversion

  const double EarthTools::convert(const double num, const conversionEnumType cnv, const unsigned int precision) {
    double ret;

    ret = num * conversion(cnv);

    if (precision > 0)
      ret = round(ret, precision);

    return ret;
  } // EarthTools::convert

  const EarthTools::systemEnumType EarthTools::findSystemType(const std::string &system) {
    if (system == "metric")
      return SYSTEM_METRIC;
    else if (system == "imperial")
      return SYSTEM_IMPERIAL;
    else if (system == "nautical")
      return SYSTEM_NAUTICAL;

    return SYSTEM_UNKNOWN;
  } // EarthTools::findSystemType

  const EarthTools::unitEnumType EarthTools::findUnitType(const std::string &unit) {
    if (unit == "speed")
      return UNIT_SPEED;
    else if (unit == "vol")
      return UNIT_VOLUME;
    else if (unit == "dist")
      return UNIT_DISTANCE;
    else if (unit == "ruler")
      return UNIT_RULER;

    return UNIT_UNKNOWN;
  } // EarthTools::findUnitType

  const std::string EarthTools::convertBySystem(const double num, const std::string &type, const std::string &system,
                                           const unsigned int precision) {
    double calc;
    std::string ret = "";
    std::string suffix = "";

    systemEnumType systype = EarthTools::findSystemType(system);
    unitEnumType unitype = EarthTools::findUnitType(type);
    conversionEnumType cnv = CONVERSION_NONE;

    if (systype == SYSTEM_UNKNOWN || unitype == UNIT_UNKNOWN)
      return "unknown";

    switch(unitype) {
      case UNIT_SPEED:
        if (systype == SYSTEM_METRIC) {
          suffix = "KPH";
          cnv = CONVERSION_NONE;
        } // if
        else if (systype == SYSTEM_IMPERIAL) {
          suffix = "MPH";
          cnv = CONVERSION_KPHTOMPH;
        } // if
        else if (systype == SYSTEM_NAUTICAL) {
          suffix = "KNOTS";
          cnv = CONVERSION_KPHTOKNOTS;
        } // if
        else
          assert(false);	// bug

        break;
      case UNIT_VOLUME:
        if (systype == SYSTEM_METRIC) {
          suffix = "mm";
          cnv = CONVERSION_NONE;
        } // if
        else if (systype == SYSTEM_IMPERIAL) {
          suffix = "in";
          cnv = CONVERSION_MMTOINCH;
        } // if
        else if (systype == SYSTEM_NAUTICAL) {
          suffix = "in";
          cnv = CONVERSION_MMTOINCH;
        } // if
        else
          assert(false);	// bug

        break;
      case UNIT_DISTANCE:
        if (systype == SYSTEM_METRIC) {
          suffix = "km";
          cnv = CONVERSION_NONE;
        } // if
        else if (systype == SYSTEM_IMPERIAL) {
          suffix = "mi";
          cnv = CONVERSION_KPHTOMPH;
        } // if
        else if (systype == SYSTEM_NAUTICAL) {
          suffix = "km";
          cnv = CONVERSION_KPHTOKNOTS;
        } // if
        else
          assert(false);	// bug

        break;
      case UNIT_RULER:
        if (systype == SYSTEM_METRIC) {
          suffix = "m";
          cnv = CONVERSION_NONE;
        } // if
        else if (systype == SYSTEM_IMPERIAL) {
          suffix = "ft";
          cnv = CONVERSION_METERTOFOOT;
        } // if
        else if (systype == SYSTEM_NAUTICAL) {
          suffix = "m";
          cnv = CONVERSION_NONE;
        } // if
        else
          assert(false);	// bug

        break;
      case UNIT_UNKNOWN:
        default:
        assert(false);		// bug
        break;
    } // switch

    calc = convert(num, cnv, precision);

    ret = openframe::stringify<double>(calc, precision) + " " + suffix;

    return ret;
  } // EarthTools::convertBySystem

  const double EarthTools::round(const double r, const unsigned int precision) {
    double off = pow(10, precision);
    return int(r*off)/off;
  } // EarthTools::round

  const double EarthTools::distance(double lat1, double lon1, double lat2, double lon2, char unit) {
    double theta;
    double distance;


    if (lat1 > 90 || lat1 < -90 ||
        lat2 > 90 || lat2 < -90)
      return -1;

    if (lon1 > 180 || lon1 < -180 ||
        lon2 > 180 || lon2 < -180)
      return -1;

    theta = lon1 - lon2;

    distance = (sin(deg2rad(lat1)) * sin(deg2rad(lat2))) + (cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta)));
    distance = acos(distance);
    distance = rad2deg(distance);
    distance = distance * 60 * 1.1515;

    switch(unit) {
      case 'M':
        break;
      case 'K':
        distance = distance * 1.609344;
        break;
      case 'N':
        distance = distance * 0.8684;
        break;
    } // switch

    return distance;
  } // EarthTools::Distance

  double EarthTools::deg2rad(double deg) {
    return (deg * pi / 180);
  } // EarthTools::deg2rad

  double EarthTools::rad2deg(double rad) {
    return (rad * 180 / pi);
  } // EarthTools::rad2deg

  const bool EarthTools::isValidLatLong(const double lat, const double lon) {
    /**
     * Invalid Positions
     *
     * Don't store invalid positions.
     *
     * Latitude > 90 || Latitude < -90 [INVALID]
     * Longitude > 180 || Latitude < -180 [INVALID]
     * Latitude == 0 || Longitude == 0 [INVALID]
     */
    if (lat == 0 || lon == 0 || lon > 180 || lon < -180 ||
        lat > 90 || lat < -90)
      return false;

    return true;
  } // EarthTools::isValidLatLong

  const double EarthTools::speed(const double dist, const time_t seconds, 
                                 const int distUnits, const int speedUnits) {
    double speed;
    time_t secs;

    if (dist < 1)
      return -1;

    // minimum of one second.
    if (seconds < 1)
      secs = 1;
    else
      secs = seconds;

    speed = ((dist * distunits[distUnits])  / (secs * speedunits[speedUnits]));

    return speed;
  } // EarthTools::Speed

  const std::string EarthTools::dec2nmea(const double n, const bool isLat) {
    double d, min;
    int deg;
    char y[10];

    d = fabs(n);

    deg = (int) floor(d);
    min = (d-deg)*60;

    if (isLat)
      snprintf(y, sizeof(y), "%02d%05.2f", deg, min);
    else
      snprintf(y, sizeof(y), "%03d%05.2f", deg, min);

    return std::string(y);
  } // EarthTools::dec2nmea

} // namespace openapi
