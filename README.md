# filter_plugin
Introduction
============

FILTER SQL clause support for MySQL

Compling
========

Place plugin directory into plugin subdirectory of MySQL source directory, then cd to MySQL source directory and compile server. Plugin will be compiled automatically. You need to use GCC 4.9 or newer  or any other compiler which supports [std::regex] () to compile plugin.

Installation
============

Copy filter_plugin.so into plugin directory of your MySQL server, then login and type:

    INSTALL PLUGIN filter_plugin SONAME 'filter_plugin.so';

Uninstallation
==============

Connect to MySQL server and run:

    UNINSTALL PLUGIN filter_plugin;
    
Usage examples
==============

    SELECT COUNT(*) FILTER(WHERE host='127.0.0.1') FROM user;
    SELECT COUNT(user) FILTER(WHERE host='127.0.0.1'), COUNT(*) FILTER(WHERE host like '%host') FROM user;
    SELECT COUNT(DISTINCT user) FILTER(WHERE host not like '%host') FROM user;
    SELECT COUNT(ALL user) FILTER(WHERE host not like '%host') FROM user;

