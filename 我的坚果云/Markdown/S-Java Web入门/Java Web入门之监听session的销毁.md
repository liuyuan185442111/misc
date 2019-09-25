统计在线用户数等功能需要监听session的销毁，有两种方式：
1. 使用HttpSessionListener监听session的销毁。
2. 使用HttpSessionBindingListener监听session的销毁。
##使用HttpSessionListener
编写一个OnlineUserListener：
```java
package advance;

import java.util.List;
import javax.servlet.ServletContext;
import javax.servlet.http.HttpSession;
import javax.servlet.http.HttpSessionListener;
import javax.servlet.http.HttpSessionEvent;

public class OnlineUserListener implements HttpSessionListener {
	public void sessionCreated(HttpSessionEvent event) {}
	public void sessionDestroyed(HttpSessionEvent event) {
		HttpSession session = event.getSession();
		ServletContext application = session.getServletContext();
		// 取得登录的用户名
		String username = (String)session.getAttribute("username");
		// 从在线列表中删除用户名
		List onlineUserList = (List)application.getAttribute("onlineUserList");
		onlineUserList.remove(username);
		System.out.println(username + "超时退出");
	}
}
```
为了让监听器发挥作用，将它添加到web.xml中：
```xml
	<listener>
		<listener-class>advance.OnlineUserListener</listener-class>
	</listener>
```
以下两种情况下就会发生sessionDestoryed（会话销毁）事件：
1.执行session.invalidate()方法时。
2.如果用户长时间没有访问服务器，超过了会话最大超时时间，服务器就会自动销毁超时的session。
##JSP中设置Session有效时间的三种方式
（1）在主页面或者公共页面中加入：

	HttpSession session = request.getSession(true);  
	session.setMaxInactiveInterval(900);
单位是秒，即在没有活动15分钟后，session将失效。这里要注意这个session设置的时间是根据服务器来计算的，而不是客户端。
（2）在项目的web.xml中设置

	<session-config>
		<session-timeout>15</session-timeout>
	</session-config>
单位是分钟。
（3）直接在应用服务器中设置，如果是tomcat，可以在tomcat目录下conf/web.xml中找到`<session-config>`元素，tomcat默认设置是30分钟，只要修改这个值就可以了。

优先级：(1)>(2)>(3)
##使用HttpSessionBindingListener
HttpSessionBindingListener虽然叫做监听器，但使用方法与HttpSessionListener完全不同。
HttpSessionListener只需要设置到web.xml中就可以监听整个应用中的所有session；HttpSessionBindingListener必须实例化后放入某一个session中，才可以进行监听。
从监听范围上比较，HttpSessionListener设置一次就可以监听所有session，HttpSessionBindingListener通常都是一对一的。

javax.servlet.http.HttpSessionBindingListener接口提供了下面的方法：
public void valueBound(HttpSessionBindingEvent event)，当对象正在被绑定到Session中时，Servlet容器会调用这个方法来通知该对象
public void valueUnbound(HttpSessionBindingEvent event)，当从Session中删除对象时，Servlet容器调用这个方法来通知该对象
Servlet容器通过HttpSessionBindingEvent对象来通知实现了HttpSessionBindingListener接口的对象，而该对象可以利用HttpSessionBindingEvent对象来访问与它相联系的HttpSession对象，javax.servlet.http.HttpSessionBindingEvent类提供了以下方法：
public HttpSessionBingdingEvent(HttpSession session,java.lang.String name)
public HttpSessionBingdingEvent(HttpSession session,java.lang.String name,java.lang.Object value)
public javax.lang.String getName()，返回绑定到Session中或者从Session中删除的属性的名字
public java.lang.Object getValue()，返回被添加/删除/替换的属性的值，如果属性被添加或者被删除，这个方法返回属性的值，如果这个属性被替换，这个方法返回属性先前的值
public HttpSession getSession()，返回HttpSession对象

看一个简单的例子
```java
package advance;

import javax.servlet.http.HttpSessionBindingListener;
import javax.servlet.http.HttpSessionBindingEvent;

public class BindingListener implements HttpSessionBindingListener
{
	public void valueBound(HttpSessionBindingEvent arg0) {
		System.out.println("valueBound");
	}
	public void valueUnbound(HttpSessionBindingEvent arg0) {
		System.out.println("valueUnbound");
	}
}
```
不需要配置web.xml，但要将listener加入session的属性中：

	BindingListener  binding = new BindingListener();  
	session.setAttribute("anyname", binding );//这个时候要触发valueBound方法了 
valueUnbound()的触发条件是以下三种情况：
1. 执行session.invalidate()时。
2. session超时自动销毁时。
3. 执行session.setAttribute("anyname", "其他对象")或session.removeAttribute("anyname")将listener从session中删除时。只要不将listener从session中删除，就可以监听到session的销毁。
##参考
[java web session监听销毁跳转](http://blog.csdn.net/mywordandyourword/article/details/18937775)
[JSP中设置Session有效时间的三种方式](http://blog.163.com/oyhj_nicholas/blog/static/323592520107310249109/)