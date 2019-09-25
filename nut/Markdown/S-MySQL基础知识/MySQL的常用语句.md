数据库
=
```sql
创建数据库
CREATE DATABASE db_name [DEFAULT CHARACTER SET character_set];

删除数据库
DROP DATABASE db_name;

修改数据库默认字符集
ALTER DATABASE db_name DEFAULT CHARACTER SET = utf8;
```
表
=
```sql
创建表
CREATE [TEMPORARY] TABLE [IF NOT EXISTS] tbl_name (列的定义)
[ENGINE=存储引擎 DEFAULT CHARSET=字符集 AUTO-INCREMENT=自增开始数字];

其中"列的定义"格式为:
列名 类型 [NOT NULL|NULL] [DEFAULT 默认值] [AUTO_INCREMENT] [CHARACTER SET=字符集]

主键被当做一列, 所以出现在"列的定义"部分:
PRIMARY KEY(列名)
```
```sql
删除表
DROP TABLE tbl_name;

添加列
ALTER TABLE tbl_name ADD 列名 列类型 [AFTER 某列|FIRST];

删除列
ALTER TABLE tbl_name DROP 列名;

修改列类型
ALTER TABLE tbl_name MODIFY 列名 新类型 新参数;

修改列名和列类型
ALTER TABLE tbl_name CHANGE 旧列名 新列名 新类型 新参数;

更改主键
主键被当成一列, 先删除旧主键, 再添加新主键
ALTER TABLE 表名 DROP PRIMARY KEY;
ALTER TABLE 表名 ADD PRIMARY KEY(列名);

重命名表
RENAME TABLE 表名 TO 新表名;
或者
ALTER TABLE 表名 RENAME [AS] 新表名;

描述表
SHOW [FULL] COLUMNS FROM 表名;
DESCRIBE 表名;
DESCRIBE 表名 列名;

排序表
ALTER TABLE tbl_name ORDER BY col_name;

复制表
CREATE TABLE tbl_name LIKE old_tbl_name;
或
CREATE TABLE tbl_name AS (SELECT 语句);
前者复制表结构, 但不复制数据; 后者复制数据, 但表结构可能不同.
可用SHOW CREATE TABLE tbl_name;或DESC tbl_name;来比较表结构的异同.

修改自增初值
ALTER TABLE tbl_name AUTO_INCREMENT = 起始数字;

清空表
TRUNCATE TABLE tbl_name;
或者
DELETE tbl_name;
前者会重置自增初值.

清空第10条以后数据
DELETE FROM tbl_name WHERE id NOT IN (SELECT TOP 10 id FROM tbl_name ORDER BY id);

改变表的默认字符集, 并且将所有列的字符集进行转换
ALTER TABLE tbl_name CONVERT TO CHARACTER SET charset_name;
```
记录
=
```sql
INSERT
INSERT INTO 表名[(列名)] VALUES(常量);

UPDATE
UPDATE 表名 SET 列名=表达式 [WHERE 条件];

SELECT
SELECT [ALL|DISTINCT] <目标列表达式>
FROM <表名或视图名>
[WHERE <条件表达式>]
[GROUP BY <列名> [HAVING <条件表达式>]
[ORDER BY<列名2> [ASC|DESC]];
DISTINCT用于去掉结果中的重复行, 没有DISTINCT, 即默认ALL.

REPLACE
REPLACE INTO 表名 VALUES(数据);
如果新插入数据在表中没有重复, 则执行插入; 如果主键重复, 则将表中重复数据替换为新数据.

随机选择指定数目的数据
用 order by rand() limit 5 子句.
```
功能
=
```sql
修改用户名
RENAME USER old_user TO new_user;

修改密码
SET PASSWORD [FOR user] = PASSWORD(‘newpassword’);
如果不加for user, 表示修改当前用户的密码. user的格式为’name’@’host’.

整理碎片
OPTIMIZE TABLE tbl_name; 
```
```sql
创建用户thor, 密码为word
insert into mysql.user(host,user,password) values("%","thor",password("word"));
flush privileges;
实际工作是在mysql.user表中添加相应记录.

授予权限, 将数据库thunder中左右表的权限授予thor
grant all privileges on thunder.* to thor@"%";
flush privileges;

或者将创建用户和授予权限放在一起
grant all privileges on thunder.* to thor@"%" identified by ‘password’;
flush privileges;
```
