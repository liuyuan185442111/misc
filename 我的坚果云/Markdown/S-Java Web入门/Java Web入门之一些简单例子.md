##获取页面的basePath
```html
<%
String path = request.getContextPath();
String basePath = request.getScheme() + "://" + request.getServerName() + ":" + request.getServerPort() + path + "/";
%>
<html>
<head>
	<title>base path</title>
</head>
<body>
    base path is <%=basePath%>
</body>
</html>
```
##error页面
```html
<%@ page isErrorPage="true" pageEncoding="utf-8" %>
<html>
<head>
	<title>errors occur</title>
</head>
<body>
	<p>the exception is:<br><%= exception %></p>
</body>
</html>
```
##图片部分超链接
```html
<%@ page contentType="text/html;charset=UTF-8" pageEncoding="utf-8" %>
<html>
    <meta http-equiv=Content-Type content="text/html;charset=utf-8">
<head>
    <title>FirstPage</title>
</head>
<body>
    <img width="270px" src="logo_white.png" usemap="#map" />
    <map name="map" id="M">
        <area shape="rect" coords="0,0,135,129" href="sample.jsp">
        <area shape="rect" coords="135,0,270,129" href="http://www.baidu.com/">
    </map>

    <marquee behavior="alternate" bgcolor="red" direction="right" scrollamount="8">
        <font color=#00FFFF size="7px">★☆★☆★</font>
    </marquee>

    <p><font color=blue size=4 face="Bookman Old Style">the time is: <%=new java.util.Date()%></font></p>

    <hr size="50px" align=left color=#ff0000>

    <ol start=3 type=A>
        <li>textarea</li>
    </ol>

    <textarea rows=3 cols=100 name="ght">hello
world</textarea>
</body>
</html>
```
##输入框为空提示
```html
<html>
<meta http-equiv=Content-Type content="text/html;charset=utf-8">
<head>
	<title>login</title>
	<script type="text/javascript">
	function Check() {
		with(document.info) {
			var username = user.value;
			var pass = password.value;
			if(username == null || username == "") alert("用户名为空");
			else if (pass == null || pass == "") alert("密码为空");
			else submit();
		}
	}
	</script>
</head>
<body><br><br><br><br><center>
	<form name="info" action="session.jsp" method="get">
		<table>
			<tr><td colspan="2" align="center">用户登录</td></tr>
			<tr><td>登录名：</td><td><input type="text" name="user"></td></tr>
			<tr><td>密码：</td><td><input type="password" name="password"></td></tr>
		</table>
		<input type="button" value="登录" onclick="Check()">
		<input type="reset" value="重置">
	</form>
</center></body>
</html>
```
##时间的显示和页面的自动刷新
```html
<%@ page language="java" import="java.util.*,java.text.SimpleDateFormat" %>
<%@ page contentType="text/html;charset=UTF-8" pageEncoding="utf-8" errorPage="error.jsp" %>
<html>
<head>
	<title>time</title>
</head>
<body bgcolor="#00ffff">
	<%
	response.setHeader("refresh","5");
	Date now = new Date();
	out.println("当前时间是："+now+"<br>");
	SimpleDateFormat f = new SimpleDateFormat("现在是"+"yyyy年MM月dd日Eahh点mm分ss秒");
	out.println(f.format(now));
	%>
</body>
</html>
```