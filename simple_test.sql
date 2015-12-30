\W
uninstall plugin filter_plugin;
install plugin filter_plugin soname 'filter_plugin.so'; 
SELECT 1;
SELECT COUNT(user) FROM mysql.user;
SELECT COUNT(user) FILTER(WHERE host='127.0.0.1') FROM mysql.user;
SELECT COUNT(*) FILTER(WHERE host='127.0.0.1') FROM mysql.user;
SELECT COUNT(user) FILTER(WHERE host='127.0.0.1'), COUNT(*) FILTER(WHERE host like '%host') FROM mysql.user;
SELECT COUNT(DISTINCT user) FILTER(WHERE host not like '%host') FROM mysql.user;
SELECT COUNT(ALL user) FILTER(WHERE host not like '%host') FROM mysql.user;
select 1;
select count(user) from mysql.user;
select count(user) filter(where host='127.0.0.1') from mysql.user;
select count(*) filter(where host='127.0.0.1') from mysql.user;
select count(user) filter(where host='127.0.0.1'), count(*) filter(where host like '%host') from mysql.user;
select count(distinct user) filter(where host not like '%host') from mysql.user;
select count(all user) filter(where host not like '%host') from mysql.user;

