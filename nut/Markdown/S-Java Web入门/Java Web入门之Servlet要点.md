Servlet是Java Web技术的核心基础，它是遵循Java Servlet规范的Java类，由web服务器端的jvm执行，是符合“请求-响应”访问模式的应用程序。Servlet通常用于在服务器端完成访问数据库、调用JavaBean等业务性操作。
Servlet的核心方法是service()，默认service()的功能是调用doGet()和doPost()，直接用浏览器访问Servlet相当于调用get方法。由于service方法会自动调用doGet()和doPost()，所以在实际编程中，不需要编写service方法，只需要编写doGet()和doPost()。doPost可以直接调用doGet，除非Servlet对于GET请求和POST请求的处理方式不一致。
不同于JavaBean，Servlet需要在web.xml里注册后才能生效。
更改或添加Servlet必须重启tomcat才能生效，修改web.xml不需要重启服务器。

下面是一个简单的Servlet。
javax包在servlet-api.jar里，如用javac命令编译Servlet，需要在环境变量classpath里加上“tomcat根目录\lib\servlet-api.jar”。
```java
package test;
import java.io.IOException;
import java.io.PrintWriter;
import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class ServletTest extends HttpServlet
{
    public ServletTest()
    {
        super();
    }
    public void destroy()
    {
        super.destroy();
    }
    public void init() throws ServletException {}

    public void doGet(HttpServletRequest request, HttpServletResponse response)
    throws ServletException, IOException
    {
        request.setCharacterEncoding("utf-8");
        response.setContentType("text/html;charset=utf-8");
        PrintWriter out = response.getWriter();
        out.println("<html><head><title>A Servlet Test</title></head><body>");
        String name = request.getParameter("name");
        out.print("你好！欢迎"+name+"使用servlet!<br>");
        out.print("你请求的servlet是：" + this.getClass());
        out.println("</body></html>");
    }
    public void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException
    {
        doGet(request, response);
    }
}
```
web.xml里的相应配置
```xml
<!-- servlet的名称和对应的servlet类路径 -->
    <servlet> 
        <servlet-name>Hello</servlet-name>
        <servlet-class>test.ServletTest</servlet-class>
    </servlet>
<!--访问这个servlet的url映射路径-->
    <servlet-mapping>
        <servlet-name>Hello</servlet-name>
        <url-pattern>/abc/which.do</url-pattern>
     </servlet-mapping>
```
访问http://IP:PORT/project/abc/which.do即可见到效果。

Servlet的API很多，这里就不说了。