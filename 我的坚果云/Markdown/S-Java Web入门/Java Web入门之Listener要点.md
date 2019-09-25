Servlet监听器用来给Web应用增加事件处理机制，以便更好地监视和控制Web应用的状态变化，从而在后台调用相应处理程序。
在一个web应用程序的整个运行周期内，web容器会创建和销毁三个重要的对象，ServletContext，HttpSession，ServletRequest，可以对它们设置属性信息，它们是监听器的监听对象。
Servlet规范中为每种事件监听器都定义了相应的接口，在编写事件监听器程序时只需实现这些接口就可以了。
<table><tr><td><b>监听对象</b></td>
<td><b>接口</b></td>
<td><b>监听事件</b></td></tr>
<tr><td rowspan="2">ServletRequest</td>
<td>ServletRequestListener</td>
<td>ServletRequestEvent</td></tr>
<tr><td>ServletRequestAttributeListener</td>
<td>ServletRequestAttributeEvent</td></tr>
<tr><td rowspan="4">HttpSession</td>
<td>HttpSessionListener</td>
<td rowspan="2">HttpSessionEvent</td></tr>
<tr><td>HttpSessionActivationListener</td></tr>
<tr><td>HttpSessionAttributeListener</td>
<td rowspan="2">HttpSessionBindingEvent</td></tr>
<tr><td>HttpSessionBindingListener</td></tr>
<tr><td rowspan="2">ServletContext</td>
<td>ServletContextListener</td>
<td>ServletContextEvent</td></tr>
<tr><td>ServletContextAttributeListener</td>
<td>ServletContextAttributeEvent</td></tr></table>
##监听器类型
###按监听的对象划分
1.用于监听应用程序环境对象（ServletContext）的事件监听器
2.用于监听用户会话对象（HttpSession）的事件监听器
3.用于监听请求消息对象（ServletRequest）的事件监听器
###按监听的事件类项划分
1.生命周期监听器，用于监听域对象自身的创建和销毁的事件监听器
包括ServletContextListener，ServletRequestListener，  HttpSessionListener
2.属性操作监听器，用于监听域对象中的属性的增加和删除的事件监听器
包括ServletContextAttributeListener，ServletRequestAttributeListener， HttpSessionAttributeListener
3.会话属性类监听器，用于监听绑定到HttpSession域中的某个对象的状态的事件监听器
包括HttpSessionBindingListener，HttpSessionActivationListener
前两种监听器都要在web.xml中进行声明，声明形式如下：
```xml
<web-app>
	<listener>
		<listener-class>类的全限定名</listener-class>
	</listener>
</web-app>
```
第三种类型的监听器不用在web.xml中进行声明。
##部署
监听器的部署在web.xml文件中配置，它的位置应该在Filter的后面Servlet的前面。
一个web.xml可以注册多个servlet事件监听器，web服务器按照它们在web.xml中注册顺序来加载和注册这些servlet事件监听器。
##部分接口
ServletContextListener接口方法有：
void contextInitialized(ServletContextEvent sce)：通知正在接受的对象，应用程序已经被加载及初始化
void contextDestroyed(ServletContextEvent sce)：通知正在接受的对象，应用程序已经被销毁

ServletRequestListener接口方法有：
requestInitialized (ServletRequestEvent sre)：通知当前对象，请求已经被加载及初始化
requestDestroyed (ServletRequestEvent sre)：通知当前对象，请求已经被消除

HttpSessionListener接口方法有：
sessionCreated(HttpSessionEvent e)：当创建一个Session时，调用该方法
sessionDestroyed(HttpSessionEvent e)：当销毁一个Session时，调用该方法

HttpSessionAttributeListener接口的方法有：
attributeAdded(HttpSessionBindingEvent se)：当在Session增加一个属性时，调用此方法
attributeRemoved(HttpSessionBindingEvent se)：当在Session删除一个属性时，调用此方法
attributeReplaced(HttpSessionBindingEvent se)：在Session属性被重新设置时，调用此方法

HttpSessionBindingEvent当中的方法：
HttpSession getSession()：可以获取Session对象
java.lang.String getName：返回Session增加、删除或者替换的属性名称
java.lang.Object getValue()：返回Session增加、删除或者替换的属性的值
##简单测试
```java
package advance;

import javax.servlet.ServletContext;
import javax.servlet.ServletContextListener;
import javax.servlet.ServletContextEvent;

public class Listener implements ServletContextListener
{
	public void contextInitialized(ServletContextEvent sce)
	{
		System.out.println("contextInitialized");
	}
	public void contextDestroyed(ServletContextEvent sce)
	{
		System.out.println("contextDestroyed");
	}
}
```
![结果](http://img.blog.csdn.net/20150502165954777)
##参考
 jsp中监听器的使用
 http://blog.csdn.net/wxwzy738/article/details/8615244