一个基于B树的key-value库
本目录下代码提供了一个单线程单表db, 并且不依赖其他其他非库头文件
如要简单使用, 仅需了解和包含db.h即可, 具体使用方式可参考../db_bench目录
db文件不支持大小端机器之间移植

TODO
能否优化遍历方式
添加bin log, 防止数据丢失
在DB::checkpoint中, 可以将prepare和commit放到另外线程来做
checkpoint线程自动估算下次醒来的时间
支持同时打开多个db, 但使用共同的checkpoint线程
