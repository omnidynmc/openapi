#ifndef OPENTEST_HEADER
#define OPENTEST_HEADER

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define SQL_WRITE_HOST		app->cfg->get_string("openapi.sql.write.host", "")
#define SQL_WRITE_DB		app->cfg->get_string("openapi.sql.write.db", "")
#define SQL_WRITE_USER		app->cfg->get_string("openapi.sql.write.user", "")
#define SQL_WRITE_PASS		app->cfg->get_string("openapi.sql.write.pass", "")

#define SQL_READ_HOST		app->cfg->get_string("openapi.sql.read.host", "")
#define SQL_READ_DB		app->cfg->get_string("openapi.sql.read.db", "")
#define SQL_READ_USER		app->cfg->get_string("openapi.sql.read.user", "")
#define SQL_READ_PASS		app->cfg->get_string("openapi.sql.read.pass", "")

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

#endif
