本来我是不想学Java和Jave Web的，因为我主要学习的是C和C++，Java也是一门比较复杂的语言，我怕精力不够，而且Java和C++都是面向对象的，语法比较类似，我怕会记混。但是没有办法，实验室项目就是Java Web的，总不能什么都不干吧，于是我稍微学了一点Java Web，也就是个入门。
我是大概一个月之前开始学习的，期间去公司实习了16天，为赶毕设进度7天花在了Hadoop上，还在阿里、腾讯、搜狐分别被虐，大概总共花在学Java Web上的时间有两周吧。之前毫无经验，Java只是扫了下语法，到现在Java Web的基础内容大概有个初步了解了，包括Jsp，Servlet，JavaBean，Filter，Listener，并做了一些小例子。主要学习资料就是一本普通的Java Web教材和网络。下面对这些天的学习成果和笔记进行一个整理和总结。
<p align="right">东方肖遥</p>
<p align="right">2015年5月2日</p>
<p align="right"><a href="http://blog.csdn.net/liuyuan185442111" target="_blank">我的csdn博客</a></p>
##Java Web基础知识
[Java Web入门之我的理解](http://blog.csdn.net/liuyuan185442111/article/details/44998951)
我看完书本后的理解，后来又对其进行了一些修正

[Java Web入门之jsp要点](http://blog.csdn.net/liuyuan185442111/article/details/45041435)
第一次请求JSP页面的执行过程
jsp标签的说明
jsp源文件翻译成java代码的过程
out.print()和out.write()的区别
粗略介绍了编译指令和动作指令
jsp的编码，乱码出现的原因和解决办法
jsp页面的9个隐含对象
Cookie的使用方法

[Java Web入门之JavaBean要点](http://blog.csdn.net/liuyuan185442111/article/details/45073511)
JavaBean的特点
如何使用JavaBean
与JavaBean相关的jsp标签

[Java Web入门之Servlet要点](http://blog.csdn.net/liuyuan185442111/article/details/45073913)
一个简单的Servlet例子

[Java Web入门之Filter要点](http://blog.csdn.net/liuyuan185442111/article/details/45084461)
Filter的原理和使用方法
sendRedirect和forward的区别
将文件放到/WEB-INF/下可保证安全
FilterChain的获得过程
如何跳过某个过滤器后面的过滤器
针对某个servlet的过滤器

[Java Web入门之Listener要点](http://blog.csdn.net/liuyuan185442111/article/details/45440783)
Listener的分类和简单用法

[Java Web入门之EL、JSTL、自定义标签要点](http://blog.csdn.net/liuyuan185442111/article/details/45130097)
EL、JSTL、自定义标签的一些知识
##JDBC和数据库
[Java Web入门之JDBC](http://blog.csdn.net/liuyuan185442111/article/details/43706115)
一个静态查询MySQL的例子
eclipse安装支持MySQL的JDBC驱动

[Java Web入门之tomcat数据库连接池](http://blog.csdn.net/liuyuan185442111/article/details/45221773)
tomcat数据库连接池的使用方法
##前端知识
[Java Web入门之css页面布局基础知识](http://blog.csdn.net/liuyuan185442111/article/details/43762639)
盒子模型和几个例子

[Java Web入门之一些小例子](http://blog.csdn.net/liuyuan185442111/article/details/45426735)
获取页面的basePath
error页面
图片部分超链接
登陆界面JavaScript控制输入框非空
时间的显示及页面的自动刷新
##实例
[Java Web入门之生成一个验证码图片](http://blog.csdn.net/liuyuan185442111/article/details/45371133)
如何生成验证码图片
 
[Java Web入门之监听session的销毁](http://blog.csdn.net/liuyuan185442111/article/details/45422779)
用Listener监听session的销毁

[Java Web入门之用Filter实现用户登录管理](http://blog.csdn.net/liuyuan185442111/article/details/45397803)
用Filter实现了用户登录的管理