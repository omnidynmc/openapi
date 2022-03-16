#ifndef MODULE_WEB_H
#define MODULE_WEB_H

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define MODULE_WEB_SQL_WRITE_HOST		app->cfg->get_string("module.web.sql.write.host", "")
#define MODULE_WEB_SQL_WRITE_DB		app->cfg->get_string("module.web.sql.write.db", "")
#define MODULE_WEB_SQL_WRITE_USER		app->cfg->get_string("module.web.sql.write.user", "")
#define MODULE_WEB_SQL_WRITE_PASS		app->cfg->get_string("module.web.sql.write.pass", "")

#define MODULE_WEB_SQL_READ_HOST		app->cfg->get_string("module.web.sql.read.host", "")
#define MODULE_WEB_SQL_READ_DB		app->cfg->get_string("module.web.sql.read.db", "")
#define MODULE_WEB_SQL_READ_USER		app->cfg->get_string("module.web.sql.read.user", "")
#define MODULE_WEB_SQL_READ_PASS		app->cfg->get_string("module.web.sql.read.pass", "")

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
