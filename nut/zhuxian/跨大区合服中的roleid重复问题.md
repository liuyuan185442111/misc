跨唯一名合服过程中roleid重复导致的与寻宝相关的问题

以下设定合服过程中，角色A和角色B发生roleid重复，并修改了角色B的roleid

dbutil gamedbd.conf scanconsignorder
列出consign表中的所有记录

如果寻宝的数据与服务器的数据不符，以服务器的数据为准，并给寻宝相应返回码，if(userid_mismatch) retcode = 7; if(roleid_mismatch) retcode = 8;

##寄售角色
###主动取消寄售
如果合服前B寄售了自己的角色，合服后在寻宝网上主动取消寄售，consign表中对应记录的seller_roleid是A的roleid，最终会将A取消寄售，而本身还是处于寄售状态。
dbconsignrolefail.hrp:75
###超时取消寄售
应该与主动取消寄售类似
###交易成功
这是不可能的，交易时会判断A是不是处于寄售状态，seller_roleid和seller_userid是不是相符。
dbconsignsoldrole.hrp:260

##寄售商品
###主动取消寄售
如果合服前B寄售了商品，合服后A能在游戏中看到该寄售信息，并且可进行取消寄售操作，但物品退换给了B账号下的第一个角色。
dbconsignmail.hrp:125，判断seller_roleid和seller_userid不符合，则以seller_userid为准。
如果手动修改consign表中某寄售商品的记录，使seller_roleid和seller_userid都变为另外一个角色，该寄售记录和商品则完全属于另外的角色了。
###超时取消寄售
应该与主动取消寄售类似
###交易成功
应该可以成功。在gamedbd/dbconsignsold.hrp中将consign表中对应记录记为了DSTATE_SOLD，但在gdeliveryd/dbconsignsold.hrp中如果userid_mismatch或roleid_mismatch为真，会告知寻宝。同时会给db发dbconsignmail.hrp的协议，这里将会发现seller_roleid和seller_userid不相符，于是将seller_roleid重置为B的第一个角色，给买家发物品，给B的第一个角色发交易成功的邮件。


解决方案推荐在合服过程中修正consign表的相应记录。

2018年3月7日