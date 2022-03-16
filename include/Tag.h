/**************************************************************************
 ** Copyright (C) 1999 Gregory A. Carter                                 **
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
 **************************************************************************/

#ifndef __TAG_H
#define __TAG_H

#include <map>
#include <string>
#include <list>
#include <ctime>
#include <set>

#include "OpenAPI_Abstract.h"

#include "noCaseCompare.h"

namespace openapi {

using std::string;
using std::map;
using std::list;
using std::pair;
using std::set;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class Tag_Exception : public OpenAPI_Exception {
  public:
    Tag_Exception(const string message) throw() : OpenAPI_Exception(message) { }

  private:
}; // class Tag_Exception

class Tag : public vector<Tag> {
  public:
    Tag();
    Tag(const string &);
    ~Tag();

    static const char DEFAULT_DELIMITER;
    static const char DEFAULT_FIELD_DELIMITER;
    static const bool DEFAULT_FIELD_UPPERCASE;
    static const bool DEFAULT_ENCODE_SPACES;

    typedef map<string, string, noCaseCompare> attribMapType;

    void attrib(const string &, const string &);
    void innerHtml(const string &innerHtml) { _innerHtml = innerHtml; }
    const string innerHtml();
    const string render();
    Tag &operator=(const string &);

  private:
    string _name;
    string _innerHtml;
    attribMapType _attribs;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif
