@echo

REM 初始化
mysqld --initialize --console;

REM 登录
mysql -u root -pSystem123@;

REM 展示
show databases;

REM 修改密码
alter user 'root'@'host' identified by 'System123@';
flish privileges;

REM 远程连接
use mysql;
update user set host="%" where user="root";