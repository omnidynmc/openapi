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
 **************************************************************************
 $Id: Vars.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
 **************************************************************************/

#include <list>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <new>
#include <iostream>
#include <string>
#include <exception>
#include <ctime>
#include <sstream>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <math.h>

#include "Tag.h"

namespace openapi {

using namespace std;

  /**************************************************************************
   ** Vars Class                                                           **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const char Tag::DEFAULT_DELIMITER 		= '|';
  const char Tag::DEFAULT_FIELD_DELIMITER	= ':';
  const bool Tag::DEFAULT_FIELD_UPPERCASE	= true;
  const bool Tag::DEFAULT_ENCODE_SPACES	= false;

  Tag::Tag(const string &name) : _name(name), _innerHtml("") {
    if (!name.length())
      assert(false);
  } // Tag::Tag

  Tag::Tag() : _name(""), _innerHtml("") {
  } // Tag::Tag

  Tag::~Tag() {
  } // Tag::~Tag

  const string Tag::render() {
    attribMapType::iterator ptr;
    stringstream a;
    stringstream s;
    string ret;

    if (_name.length() < 1)
      return _innerHtml;

    for(ptr = _attribs.begin(); ptr != _attribs.end(); ptr++) {
      if (ptr != _attribs.begin())
        a << " ";
      a << ptr->first << "=\"" << ptr->second << "\"";
    } // for

    s << "<" << _name << " " << a.str();
    if (_innerHtml.length() || size()) {
      s << ">";

      for(Tag::size_type i=0; i < size(); i++)
        s << at(i).render();

      if (_innerHtml.length())
        s << _innerHtml;

      s << "</" << _name << ">";
    } // if
    else
      s << " />";

    return s.str();
  } // Tag::render

  void Tag::attrib(const string &name, const string &value) {
    attribMapType::iterator ptr;

    ptr = _attribs.find(name);
    if (ptr != _attribs.end()) {
      _attribs[name] = value;
      return;
    } // if

    _attribs[name] = value;

    return;
  } // Tag::attrib

  Tag &Tag::operator=(const string &innerHtml) {
    _innerHtml = innerHtml;
    return *this;
  } // Tag::operator=

} // namespace openapi
