##基础知识
Web服务器接收到来自客户端的web请求，在交给Servlet处理之前，会先经过过滤器的过滤。Servlet处理完请求将结果返回给客户端的时候，也需要先经过过滤器的过滤。多个过滤器可以组成过滤器链，按照在web.xml的注册顺序依次调用，对返回结果的过滤顺序正好与此相反。

过滤器必须实现javax.servlet.Filter接口，这一接口声明了init、doFilter、destroy三个方法。

	public void init(FilterConfig);
在容器实例化过滤器时被调用，对过滤器进行初始化，调用时会传递一个包含过滤器的配置和运行环境的FilterConfig对象。利用FilterConfig对象可以得到ServletContext对象，以及在web.xml文件中指定的过滤器初始化参数。

	public void doFilter(ServletRequest, ServletResponse, FilterChain);
过滤器的自定义行为主要在这里完成，过滤器执行doFilter方法时，会自动获得过滤器链对象，使用该对象的doFilter方法可继续调用下一级过滤器。
	
	public void destory();
在停止使用过滤器前，由容器调用过滤器的这个方法，完成必要的清除和释放资源的工作。

Filter可能的实现过程是（以过滤web请求为例）：从web.xml中找到第一个	`<filter>`，得到过滤器类路径、过滤器参数、下一个过滤器，将参数封装到FilterConfig对象中传递给过滤器对象的init方法，将ServletRequest，ServletResponse，FilterChain（下一个过滤器对象）作为参数传递给滤器对象的doFilter方法。用户在doFilter方法中调用FilterChain.doFilter方法，这样会调用下一个过滤器，过程类似，如果没有调用FilterChain.doFilter方法，web请求就不会提交给Servlet，也就是说web请求在此消失。

如果在Servlet里先调用`out.flush();out.close();`，再执行`Thread.sleep(3000);`，过滤器这样写：

	System.out.println("before");
	filterChain.doFilter(request, response);
	System.out.println("after");
结果是客户端先得到网页，终端里等待3s后再打印出after。应该是执行完`out.close();`之后，web服务器结束输出，将结果返回给客户端。如果没有out.close();语句，则在3s后终端打印完after，客户端才得到网页内容，可能是调用完过滤器后web服务器自动调用了out.close();。
##一个简单的例子
只有指定的IP地址才能访问/abc/*。
```java
package advance;
import java.io.IOException;
import javax.servlet.Filter;
import javax.servlet.FilterChain;
import javax.servlet.FilterConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class FilterIP implements Filter
{
	private FilterConfig fiterconfig;
	private String ip;
	public void init(FilterConfig config) throws ServletException
	{
		this.fiterconfig = config;
		ip = fiterconfig.getInitParameter("allowedIP");
	}
	public void destroy(){}
	public void doFilter(ServletRequest req, ServletResponse res, FilterChain chain) throws IOException, ServletException
	{
		HttpServletRequest request = (HttpServletRequest)req;
		HttpServletResponse response = (HttpServletResponse)res;
		String reqIP = request.getRemoteHost();
		if (reqIP.equals(ip))
		{
			chain.doFilter(req, res);
		}
		else
		{
			response.sendRedirect("error.jsp");
		}
	}
}
```
web.xml配置
```xml
	<filter>
		<filter-name>fIP</filter-name>
		<filter-class>advance.FilterIP</filter-class>
		<init-param>
			<param-name>allowedIP</param-name>
			<param-value>10.108.36.227</param-value>
		</init-param>
	</filter>
	<filter-mapping>
		<filter-name>fIP</filter-name>
		<url-pattern>/abc/*</url-pattern>
	</filter-mapping>
```
##两个Filter组成FilterChain
filter1
```java
	System.out.println("before1");
	filterChain.doFilter(request, response);
	System.out.println("after1");
```
fiter2
```java
	System.out.println("before2");
	filterChain.doFilter(request, response);
	System.out.println("after2");
```
web.xml配置
```xml
     <filter>
        <filter-name>f1</filter-name>
        <filter-class>advance.Filter1</filter-class>
      </filter>
      <filter-mapping>
        <filter-name>f1</filter-name>
        <url-pattern>/*</url-pattern>
      </filter-mapping>

      <filter>
        <filter-name>f2</filter-name>
        <filter-class>advance.Filter2</filter-class>
      </filter>
      <filter-mapping>
        <filter-name>f2</filter-name>
        <url-pattern>/*</url-pattern>
      </filter-mapping>
```
结果：
before1
before2
after2
after1
![result](http://img.blog.csdn.net/20150418173811568)
##sendRedirect和forward的区别
HttpServletResponse.sendRedirect()是通过向客户浏览器发送命令来完成跳转操作，是重新定向，前后页面不是一个request，在所有的语句都执行完之后才完成跳转操作。
HttpServletRequest.getRequestDispatcher()返回的是一个RequestDispather对象。RequestDispatcher.forward()是在服务器端运行，是请求转发，前后页面共享一个request，所以RequestDispatcher.forward()对于浏览器来说是“透明的”，浏览器的地址栏都不会变，此语句后面的语句将不再执行。
`<jsp:forward>`是服务器跳转，跳转语句后面的语句将不再执行。实际是调用了pageContext.forward()函数。

ServletContext.getRequestDispatcher(String url)中的url只能使用绝对路径；
ServletRequest.getRequestDispatcher(String url)中的url可以使用相对路径。
因为ServletRequest具有相对路径的概念；而ServletContext无此概念。

在开发中如果项目中有一些敏感web资源不想被外界直接访问，那么可以考虑将这些敏感的web资源放到WEB-INF目录下，这样就可以禁止外界直接通过URL来访问了，但可以通过request.getRequestDispatcher("/WEB-INF/some.jsp").forward(request, response)来访问。
##其他
###FilterChain的获得
在web.xml中从头到尾依次查找`<filter-mapping>`标签里的`<url-pattern>`，如果请求页面的url符合则得到`<filter-name>`，再根据`<filter-name>`查找其`<filter-class>`。所以FilterChain的顺序只与`<filter-mapping>`标签的顺序有关。
获得FilterChain中下一个过滤器应该是动态查找确定的，因为可能在某个Filter中进行了forward动作，查找下一个Filter的时候只能找下面可以过滤forward的过滤器。
###跳过后续过滤器
假如应用中存在多个过滤器，如何在某个过滤器处理后跳过后续过滤器处理？这里提供了两种方式来解决这个问题。
方式一：
在后续过滤器中根据前面过滤器处理后设置的标识手动跳过。标识可以放在request Attributes里，比如`request.setAttribute("jumpFilter", "true")`。
方式二：
过滤器处理时，不进行后续过滤器链处理（FilterChain#doFilter()），而是直接转发请求给Servlet或URL（RequestDispatcher#forward()），比如request.getRequestDispatcher(httpServletRequest.getServletPath() + httpServletRequest.getPathInfo()).forward(request, response)。
这样会跳过后续的所有过滤器处理，如果有某个过滤器仍然需要进行处理，那可以在web.xml中配置该过滤器`<filter-mapping>：<dispatcher>FORWARD</dispatcher>`表示仍会处理forward的请求。

2.4版本的servlet规范在部属描述符中新增加了一个`<dispatcher>`元素，这个元素有四个可能的值，即REQUEST,FORWARD,INCLUDE和ERROR，可以在一个`<filter-mapping>`元素中加入任意数目的`<dispatcher>`，使得filter将会作用于直接从客户端过来的request，通过forward过来的request，通过include过来的request和通过`<error-page>`过来的request。如果没有指定任何`<dispatcher>`元素，默认值是REQUEST。
###过滤器可以针对某个servlet过滤
需要更改的是把web.xml里相应的`<url-pattern>`改成` <servlet-name>`。
```xml
<filter-mapping>
    <filter-name>MyFilter</filter-name>
    <servlet-name>MyServlet</servlet-name>
</filter-mapping>
```
###Spring Security
Spring Security的web架构是完全基于标准的servlet过滤器的。它没有在内部使用servlet或任何其他基于servlet的框架（比如spring mvc），所以它没有与任何特定的web技术强行关联。它只管处理HttpServletRequest和HttpServletResponse，不关心请求是来自浏览器，web服务客户端，HttpInvoker，还是一个AJAX应用。
##参考
关于Filter中ServletRequest和ServletResponse强转HttpServletRequest和HttpServletResponse安全问题
http://blog.csdn.net/huan_mie/article/details/6114427
getRequestDispatcher()与sendRedirect()的区别
http://www.cnblogs.com/phpzxh/archive/2010/02/01/1661137.html
理解Servlet过滤器(javax.servlet.Filter)
http://blog.csdn.net/microtong/article/details/5007170
如何跳过过滤器处理？
http://88250.b3log.org/how-to-skip-filters-in-java
web.xml里`<filter-mapping>`中的`<dispatcher>`作用
http://hintcnuie.iteye.com/blog/226251/
安全过滤器链
http://www.mossle.com/docs/springsecurity3/html/web-infrastructure.html