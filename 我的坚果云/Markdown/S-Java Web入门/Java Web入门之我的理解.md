Web无非是一个BS模型：
Browser-----------------Web Server

Browser请求一个url，Web Server返回一个html文档。
所谓动态网页，就是Browser来请求时，那个html文档是现生成的。最早的cgi——我就用c写过——就是用printf一行一行地把html语句打印出来。现在我发现Java Web也是类似，不过是换用了tomcat这种应用程序（当然也可以是其他程序），语言也换成了java。但本质是类似的，这就是所谓的Servlet。
但是用java写太麻烦了，于是在Servlet上封装一层，就是jsp语言，用jsp写网页就容易多了嘛。当Browser第一次访问jsp页面时，tomcat会将其翻译成java源文件，然后编译成class文件，再用jvm执行，生成html源文件，返回给Browser。Java源文件和class文件放在`"$TOMCAT/work/"`目录下。以后再访问这个页面，直接用jvm执行class文件就行了，所以首次访问会慢一点。
jsp语法类似于html语法，最大的不同是可以通过`"<% %>"`标签来插入java程序段，当然还有其他一些特性。所以jsp基本上就是html和java的混合，翻译过程基本上就是将html语句原封不动的放到java输出语句里。

到现在基本上就可以写简单的Java Web程序了，但html和java语句都混合在jsp源文件里，混乱且不好维护。改进方向有两个：一是使用JavaBean；二是使用EL、JSTL和自定义标签。
一、使用JavaBean
可以把jsp中的一些功能或/和数据拿出来，用java类封装起来，称之为JavaBean，然后通过`<jsp:useBean>`标签引入JavaBean。JavaBean就是一种特殊的java类而已，不过多了一些约束条件。
二、使用EL、JSTL和自定义标签
这个方向将java语句封装成标签，从而使jsp看起来更像一门独立的语言。

下面简要描述下其他相关内容：
$TOMCAT下有webapps这个文件夹，webapps下不同的文件夹代表不同的项目，/WEB-INF/classes/下存放JavaBean和Servlet的class文件，不同的是JavaBean类的路径在`<jsp:useBean>`标签里标识，而Servlet类的路径在/WEB-INF/web.xml里标识。

Servlet是java类，它遵循Java Servlet规范，由web服务器端的jvm执行，被用来扩展web服务器的功能，可以接收http请求，并将相应结果返回给客户端。Servlet可以调用JavaBean。Servlet接受get和post请求，要实现doGet和doPost方法，接受get请求时调用doGet方法，接受post请求时调用doPost方法。直接访问Servlet相当于对其发送get请求。Servlet的类路径和url映射路径都在web.xml里配置。

Filter也是java类，它处于Web客户端与Servlet、jsp中间。

使用jsp和JavaBean进行web开发是所谓的Model 1，使用jsp，JavaBean，Servlet进行web开发是所谓的Model 2，关于Model 1和Model 2请百度。
spring，struts，hibernate等框架基本上是对jsp，JavaBean，Servlet，Filter的封装。

EL和JSTL：EL表达式使一些表述变得简单；JSP标准标签库，我觉得它的核心标签库的流程控制语句不怎么好用，自定义标签看起来倒是不错。通过使用EL和标签库，使jsp看起来更像一门独立的语言，而不是处处充斥着`<% %>`java代码段。

当时挑了一本比较薄的书，《JSP Web技术及其应用教程》，看了几天，大概理解就是这样。