crond是一个系统服务，用于执行定时任务。
它每分钟读取一次三个地方的配置文件，然后执行其标示的命令，这三个地方是：
/etc/crontab，这个文件负责安排由系统管理员制定的维护系统以及其他任务的crontab。
/etc/cron.d/，这个目录用来存放任何要执行的crontab文件或脚本。
/var/spool/cron/，这个目录下存放的是每个用户包括root的crontab任务，每个任务以创建者的名字命名，比如tom的crontab任务对应的文件就是/var/spool/cron/tom，一个用户最多只有一个crontab文件。

crontab命令用于设置周期性被执行的指令，实际上是在编辑上面这些配置文件。

/etc/下还有这几个目录cron.hourly/、cron.daily/、cron.weekly/、cron.monthly/，这几个目录方便管理每小时、每天、每周、每月执行的任务，如要启用它们，需在/etc/crontab或/etc/cron.d/中添加它们，否则它们就是无用的。

虽然/var/spool/cron/的权限是700，但普通用户可以通过crontab命令来提交定时任务。root通过/etc/cron/cron.allow文件来控制谁有权使用crontab命令，如果用户的名字出现在cron.allow文件中，他就有权使用crontab命令。如果cron.allow文件不存在，系统会检查/etc/cron/cron.deny文件来确定是否这个用户被拒绝存取。如果两个文件都存在，cron.allow有优先权。如果两个文件都不存在，只有root可以提交任务。如果cron.deny文件为空文件，所有的用户都可以使用crontab。

参考
《linux定时任务》
http://blog.csdn.net/liuyuan185442111/article/details/42716451
《linux下添加定时任务》
http://blog.csdn.net/hi_kevin/article/details/8983746
《Fine-grainedtask scheduling with cron.d》
http://articles.slicehost.com/2010/8/6/fine-grained-task-scheduling-with-cron-d