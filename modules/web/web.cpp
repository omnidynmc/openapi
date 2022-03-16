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
 $Id$
 **************************************************************************/

#include <fstream>
#include <string>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <iostream>

#include "App.h"
#include "App_Log.h"
#include "Web.h"

#include "web.h"
#include "openapi.h"
extern "C" {
  /**********************
   ** Global Variables **
   **********************/

  modweb::Web *web;

  /******************
   ** Log Commands **
   ******************/

  /********************
   ** Initialization **
   ********************/

  /*****************
   ** Constructor **
   *****************/
  void module_constructor() {
    try {
      web = new modweb::Web();
      web->logger(new openapi::App_Log(""));
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch

    web->initializeSystem();

    return;
  } // module_constructor

  /****************
   ** Destructor **
   ****************/

  void module_destructor() {
    app->cfg->pop("module.web");

    // close listening sockets
    web->deinitializeSystem();
    delete web;
    return;
  } // module_destructor
} // extern "C"
