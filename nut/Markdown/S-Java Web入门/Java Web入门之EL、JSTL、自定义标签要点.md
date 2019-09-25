Expression Language
##基础
目的：为了使JSP写起来更加简单。
语法结构：`${expression}`
启用/禁用：通过`<%@ page isELIgnored="true|false" %>`来设置jsp页面是否支持EL，默认是支持EL。
翻译过程：

	${expression}
↓
```java
out.write(
	(java.lang.String) org.apache.jasper.runtime.PageContextImpl.proprietaryEvaluate(
		"${expression}", java.lang.String.class, (javax.servlet.jsp.PageContext)_jspx_page_context, null, false
	)
);
```
_jspx_page_context就是jsp里的隐含对象pageContext。
也就是说将expression和pageContext传入EL引擎，然后翻译出String类型的结果。
##.与[ ]运算符
EL提供“.”和“[ ]”两种运算符来读取数据。
当要读取的属性名称中包含一些特殊字符，如 . 或 - 等并非字母或数字的符号，就一定要使用“[ ]”。例如：`${user.My-Name}`应当改为`${user["My-Name"]}`。
当访问数组的元素，此时属性名称就是数字，所以不能用“.”来读取数据，只能用“[ ]”。
在EL中，字符串既可以使用`"abc"`，也可以使用`'abc'`，所以下面三者是等价的（如果都合理）：
```jsp
${user.username}
${user["username"]}
${user['username']}
```
##作用域
使用EL时，默认会以一定的顺序搜索4个作用域，并显示最先找到的属性值。例如，对于`${username}`，会按照如下作用域顺序依次查找：
pageContext.getAttribute("username") → request.getAttribute("username") → session.getAttribute("username") → application.getAttribute("username")
如果找不到，则返回一个空字符串。
也可以在EL里显式指明作用域，下面是EL使用到的变量作用域的名称：

- page范围：pageScope
- request范围：requestScope
- session范围：sessionScope
- application范围：applicationScope

简单例子
```jsp
<%request.setAttribute("name", "lily");%>
${requestScope.name}
```
输出lily。
##JavaBean
用`<jsp:useBean id="user" class="Advance.Customer"/>`建立了JavaBean对象，${user.username}相当于user.getUsername()。
通过<% %>和<%! %>定义的对象是无法使用EL的。
##隐式对象
EL表达式含有11个隐式对象。
|EL隐式对象|说明|
|-|-|
|pageContext|JSP页面的上下文对象，它可以用于访问JSP隐式对象。|
|pageScope||
|requestScope||
|sessionScope||
|applicationScope||
|param|获取request对象参数的单个值，对应于request.getParameter()。表达式${param.loginname}相当于request.getParameter(loginname)|
|paramValues|获取request对象参数的一个数值数组而不是单个值，对应于request.getParameterValues()。|
|header|将请求头名称映射到单个字符串头值，对应于request.getHeader()|
|headerValues|将请求头名称映射到一个数值数组，对应于request.getHeaderValues()|
|cookie|将cookie名称映射到单个cookie对象，对应于request.getCookies()|
|initParam|将上下文初始化参数名称映射到单个值，对应于ServletContext.getInitParamter()|
pageScope，requestScope，sessionScope，applicationScope都是Map型变量。
initParam用于获取web.xml中初始的参数值。
##JSP Standard Tag Library
JSP标准标签库，将许多JSP应用程序通用的核心功能封装为简单的标记。
如要使用JSTL，需要几个jar包，可以从http://tomcat.apache.org/download-taglibs.cgi下载。
在JSP页面中使用JSTL标签之前，必须用taglib指令将标签库导入JSP页面，指令如下：
`<% @taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>`
prefix是自定义前缀，可以改成别的。
JSTL提供了5个主要的标签库：核心标签库、国际化与格式化标签库、XML标签库、SQL标签库、函数标签库。
一个使用核心标签库的简单例子，用来判断成绩的等级
```jsp
<%@ page language="java" pageEncoding="utf-8" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<html>
<head>
	<title>score</title>
</head>
<body>
	<c:set var="x" value="${param.score}" />
	<c:if test="${x<0.0}">成绩不能为负</c:if>
	<c:if test="${!(x<0.0)}">
	   <c:if test="${! empty x}">你输入的成绩为${x}, 等级为:
		  <c:choose>
                <c:when test="${x>=90.0}">优秀</c:when>
                <c:when test="${x>=80.0}">良好</c:when>
                <c:when test="${x>=70.0}">中等</c:when>
                <c:when test="${x>=60.0}">及格</c:when>
                <c:otherwise>不及格</c:otherwise>
           </c:choose>
        </c:if>
	</c:if>
	<form action="">
		input score:<input type="text" name="score"><br>
		<input type="submit" value="submit"/>
	</form>
</body>
</html>
```
这几个标签无非是封装了定义语句和判断语句，似乎JSTL的作用是使JSP看起来更像一门独立的编程语言。
##JSP自定义标签
JSP技术提供了一种封装其他动态类型的机制——自定义标签，取代了JSP中的java程序，方便不熟悉java编程的网页设计人员。JSP自定义标签与JSTL技术实现并无不同之处。
开发自定义JSP标签的基本步骤如下：
1. 开发标签处理程序类
2. 创建标签库的描述文件（tld文件）：记录标签处理程序类的属性和位置，JSP容器通过这个文件来得知自定义标签处理程序的信息。
3. 在web.xml文件配置元素：定义了web应用中用得到的自定义标签，以及标签库描述文件的位置。
或者省略第三步，在jsp中直接申明tld文件的路径。
自定义标签需要JavaBean组件的支持。JSTL似乎是对一些java语句的封装，自定义标签似乎是对JavaBean的封装。

一个生成随机数的自定义标签
TagRandom.java
```java
package advance;
import java.util.Random;
import java.io.IOException;
import javax.servlet.jsp.JspException;
import javax.servlet.jsp.tagext.TagSupport;

public class TagRandom extends TagSupport
{
    public int doStartTag() throws JspException
    {
        Random r = new Random();
        int n = r.nextInt(1000);
        try
        {
            pageContext.getOut().print(n);
        } catch(IOException e)
        {
            e.printStackTrace();
        }
        return EVAL_BODY_INCLUDE;
    }
}
```
/WEB-INF/tagrandom.tld
```xml
<taglib>
<tlib-version>1.0</tlib-version>
<jsp-version>2.0</jsp-version>
<tag>
	<name>tagrandom</name>
	<tag-class>advance.TagRandom</tag-class>
	<body-content>empty</body-content>
</tag>
</taglib>
```
jsp页面
```jsp
<%@ page language="java" pageEncoding="utf-8" %>
<%@ taglib uri="/WEB-INF/tagrandom.tld" prefix="trd" %>
<html>
<head>
	<title>score</title>
</head>
<body>
	<trd:tagrandom/><br>
</body>
</html>
```