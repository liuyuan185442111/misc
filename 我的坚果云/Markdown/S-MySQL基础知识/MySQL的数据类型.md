MySQL的数据主要分成三类：数字类型、字符串类型、日期和时间类型。
整形数据类型
-
数据类型|取值范围|大小（字节）
-|-|-
TINYINT|-127~127/0~255|1
BIT|-127~127/0~255|1
BOOL|-127~127/0~255|1
SMALLINT|-32768~32767/0~65535|2
MEDIUMINT|-8388608~8388607/0~16777215|3
INT|太大不写了|4
BIGINT|太大不写了|8
关键字INT是INTEGER的同义词。
关于TINYINT、BIT、BOOL为什么同时存在，和int(11)的意义，参见[《mysql int(3)与int(11)的区别》](http://blog.sina.com.cn/s/blog_610997850100wjrm.html)。
浮点型数据类型
-
FLOAT占4字节，DOUBLE占8字节。
还有DECIMAL类型。
对于DECIMAL(M,D)，如果M>D，占M+2个字节，否则占D+2个字节。例如salary DECIMAL(5,2)，5代表十进制数字的数目，2代表在小数点后的数字位数，这里，salary列可以存储的值范围是从-999.99到999.99(似乎还有另外的地方来存符号？)。如果溢出，截短为最大值或最小值。关键字DEC是DECIMAL的同义词。
常规字符串类型
-
包括char(M)和varchar(M)，参见[《MySql中varchar(10)和varchar(100)的区别==>>以及char的利弊》](http://blog.csdn.net/imzoer/article/details/8435540)。
char等价于char(1)。 
ENUM和SET类型
-
类型|最大值|说明
-|-|-
ENUM(“value1”,…)|65535|该类型的列只可以容纳所列值之一或者为NULL
SET(“value1”,…)|2^个数-1|该类型的列可以容纳一组值或者为NULL
例如：

	CREATE TABLE t(c set('a','b','c'));
a,b,c的权值分别为1,2,4，`'a,b'`就是3，该列取值范围为[0,7]，0就是NULL，其他不合理的值都自动变为NULL。`INSERT INTO t VALUES('a,b');`等价于`INSERT INTO t VALUES(3);`。

	CREATE TABLE t(c enum('a','b','c'));
a,b,c的权值分别为1,2,3，该列取值范围为[0,3]，0就是NULL，其他不合理的值自动变为NULL。`INSERT INTO t VALUES('b');`等价于`INSERT INTO t VALUES(2);`。
日期和时间类型
-
如果赋予类型一个不合法的值，将会被0代替。
类型|取值范围|说明
-|-|-
DATE|1000-01-01~9999-12-31|YYYY-MM-DD
TIME|-838:58:59~835:59:59|HH:MM:SS
DATETIME|1000-01-01 00:00:00——9999-12-31 23:59:59|YYYY-MM-DD HH:MM:SS
TIMESTAMP|1970-01-01 00:00:00——2037年的某个时间|专有的自动更新特性，格式同DATETIME类型
YEAR(M)|1901~2155|可指定两位数字和四位数字的格式