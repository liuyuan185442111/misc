为数据库连接建立一个存储池，它是一个储存了多个数据库连接对象的容器，当程序需要连接数据库时，可直接从连接池中获取一个连接，使用结束时将连接还给连接池。
连接数据库的这个事情交给tomcat来做，需要连接数据库时，就通过调用getConnection()获取一个connection，使用结束close这个connection即可。

配置：
将jdbc驱动包mysql-connector-java-5.1.18-bin.jar放到lib文件夹里
conf\context.xml
```xml
<Resource name="jdbc/user" auth="Container" type="javax.sql.DataSource"
maxIdle="10" maxWait="1000" maxActive="10" username="root" password="root"
driverClassName="com.mysql.jdbc.Driver" url="jdbc:MySQL://10.108.27.48:3306/thunder" />
```
web.xml
```xml
<resource-ref>
  <description>db pool</description>
  <res-ref-name>jdbc/user</res-ref-name>
  <res-auth>Container</res-auth>
  <res-type>javax.sql.DataSource</res-type>
</resource-ref>
```
|context.xml|web.xml|含义|
|-|-|-|
|name|res-ref-name|指定资源相对于java:comp/env上下文的JNDI名（可按需修改）|
|auth|res-auth|指定资源的管理者（默认Container即可）
|type|res-type|指定资源所属的java类的完整限定名（默认即可）|
|maxIdle||指定连接池中保留的空闲数据库连接的最大数目（可按需修改）|
|maxWait||指定等待一个数据库连接成为可用状态的最大时间，单位毫秒（可按需修改）|
<br>
使用：
```jsp
<%@page import="javax.naming.InitialContext"%>
<%@page import="javax.sql.DataSource"%>
<%@page import="java.sql.*"%>
<%@page pageEncoding="utf-8"%>
<html>
<body>
	<%
       InitialContext ctx = new InitialContext();
       //在ctx里根据name查找数据库连接池
       DataSource ds = (DataSource)ctx.lookup("java:comp/env/jdbc/user");
       Connection conn = ds.getConnection();
       
       String sql = "select * from user";
       PreparedStatement pstmt = conn.prepareStatement(sql);
       ResultSet rs = pstmt.executeQuery();
       out.println("查询到的数据为:<br>");
       while(rs.next()) {
       %>
        <%=rs.getString(1)%><br>
        <%=rs.getString(2)%><br>
        <%=rs.getString(3)%><br>
       <%
       }
       conn.close();//将连接还给连接池
       conn = null;
       %>
</body>
</html>
```