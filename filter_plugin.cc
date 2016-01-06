/*
* Copyright (c) 2015, Sveta Smirnova. All rights reserved.
* 
* This file is part of filter_plugin.
*
* filter_plugin is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* filter_plugin is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with filter_plugin.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef MYSQL_SERVER
#define MYSQL_SERVER
#endif

#include <ctype.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <regex>

#include "sys_vars.h"

#include <mysql/plugin.h>
#include <mysql/plugin_audit.h>
#include <mysql/service_mysql_alloc.h>
#include <my_thread.h> // my_thread_handle needed by mysql_memory.h
#include <mysql/psi/mysql_memory.h>
#include <typelib.h>

using namespace std;
using namespace std::regex_constants;
static MYSQL_PLUGIN plugin_info_ptr;

/* instrument the memory allocation */
#ifdef HAVE_PSI_INTERFACE
static PSI_memory_key key_memory_filter;

static PSI_memory_info all_rewrite_memory[]=
{
    { &key_memory_filter, "filter", 0 } 
};

static int filter_plugin_init(MYSQL_PLUGIN plugin_ref)
{
  plugin_info_ptr= plugin_ref;
  const char* category= "sql";
  int count;

  count= array_elements(all_rewrite_memory);
  mysql_memory_register(category, all_rewrite_memory, count);
  return 0; /* success */
}

#else

#define plugin_init NULL
#define key_memory_filter PSI_NOT_INSTRUMENTED

#endif /* HAVE_PSI_INTERFACE */

/* Declarations */
void _rewrite_query(const void *event, const struct mysql_event_parse *event_parse, char const* new_query);


/* The job */
/*
* <filter clause> ::=
*   FILTER <left paren> WHERE <search condition> <right paren>
* (10.9 <aggregate function>, 5WD-02-Foundation-2003-09.pdf, p.505)
* Only for aggregate functions:
* <computational operation> ::=
* AVG
* | MAX
* | MIN
* | SUM
* | EVERY
* | ANY
* | SOME
* | COUNT
* | STDDEV_POP
* | STDDEV_SAMP
* | VAR_SAMP
* | VAR_POP
* | COLLECT
* | FUSION
* | INTERSECTION
* <set quantifier> ::=
* DISTINCT
* | ALL
*
* MySQL only supports COUNT|AVG|SUM|MAX|MIN|STDDEV_POP|STDDEV_SAMP|VAR_SAMP|VAR_POP
*
*/
static int filter(MYSQL_THD thd, mysql_event_class_t event_class,
                          const void *event)
{
  const struct mysql_event_parse *event_parse=
    static_cast<const struct mysql_event_parse *>(event);

  if (event_parse->event_subclass != MYSQL_AUDIT_PARSE_PREPARSE)
    return 0;
  
  string subject= event_parse->query.str;
  string rewritten_query;
  
  regex filter_clause_star("(COUNT)\\((\\s*\\*\\s*)\\)\\s+FILTER\\s*\\(\\s*WHERE\\s+([^\\)]+)\\s*\\)", ECMAScript | icase);
  rewritten_query= regex_replace(subject, filter_clause_star, "$1(CASE WHEN $3 THEN 1 ELSE NULL END)");
  subject= rewritten_query;
  
  regex filter_clause_distinct("(COUNT|AVG|SUM|MAX|MIN|STDDEV_POP|STDDEV_SAMP|VAR_SAMP|VAR_POP)\\(\\s*DISTINCT\\s([^\\)]*)\\)\\s+FILTER\\s*\\(\\s*WHERE\\s+([^\\)]+)\\s*\\)", ECMAScript | icase);
  rewritten_query= regex_replace(subject, filter_clause_distinct, "$1(DISTINCT CASE WHEN $3 THEN $2 ELSE NULL END)");   
  subject= rewritten_query;
  
  regex filter_clause_all("(COUNT|AVG|SUM|MAX|MIN|STDDEV_POP|STDDEV_SAMP|VAR_SAMP|VAR_POP)\\(\\s*ALL\\s([^\\)]*)\\)\\s+FILTER\\s*\\(\\s*WHERE\\s+([^\\)]+)\\s*\\)", ECMAScript | icase);
  rewritten_query= regex_replace(subject, filter_clause_all, "$1(ALL CASE WHEN $3 THEN $2 ELSE NULL END)");   
  subject= rewritten_query;
  
  regex filter_clause("(COUNT|AVG|SUM|MAX|MIN|STDDEV_POP|STDDEV_SAMP|VAR_SAMP|VAR_POP)\\(([^\\)]*)\\)\\s+FILTER\\s*\\(\\s*WHERE\\s+([^\\)]+)\\s*\\)", ECMAScript | icase);
  rewritten_query= regex_replace(subject, filter_clause, "$1(CASE WHEN $3 THEN $2 ELSE NULL END)");
  
  if (0 != rewritten_query.compare(event_parse->query.str))
    _rewrite_query(event, event_parse, rewritten_query.c_str());
   
  
  return 0;
}

static st_mysql_audit filter_plugin_descriptor= {
  MYSQL_AUDIT_INTERFACE_VERSION,  /* interface version */
  NULL,
  filter,                         /* implements FILTER */
  { 0, 0, (unsigned long) MYSQL_AUDIT_PARSE_ALL,}
};

mysql_declare_plugin(filter_plugin)
{
  MYSQL_AUDIT_PLUGIN,
  &filter_plugin_descriptor,
  "filter_plugin",
  "Sveta Smirnova",
  "FILTER SQL:2003 support for MySQL",
  PLUGIN_LICENSE_GPL,
  filter_plugin_init,
  NULL,               /* filter_plugin_deinit - TODO */
  0x0001,             /* version 0.0.1      */
  NULL,               /* status variables   */
  NULL,               /* system variables   */
  NULL,               /* config options     */
  0,                  /* flags              */
}
mysql_declare_plugin_end;


// private functions

void _rewrite_query(const void *event, const struct mysql_event_parse *event_parse, char const* new_query)
{
  char *rewritten_query= static_cast<char *>(my_malloc(key_memory_filter, strlen(new_query) + 1, MYF(0)));
  strncpy(rewritten_query, new_query, strlen(new_query));
  rewritten_query[strlen(new_query)]= '\0';
  event_parse->rewritten_query->str= rewritten_query;
  event_parse->rewritten_query->length=strlen(new_query);
  *((int *)event_parse->flags)|= (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
}


/* vim: set tabstop=2 shiftwidth=2 softtabstop=2: */
/* 
* :tabSize=2:indentSize=2:noTabs=true:
* :folding=custom:collapseFolds=1: 
*/
