<%@ page language="java" import="java.util.*" pageEncoding="UTF-8"%>
<%@ page import="java.io.InputStreamReader"%>
<%@ page import="java.io.LineNumberReader"%>
<%@ page import="java.io.IOException"%>
<%@ page import="java.util.Date.*"%>

<%
//response.setHeader("Cache-Control","no-store");
//response.setHeader("Pragrma","no-cache");
//response.setDateHeader("Expires",0);
%>

<%! String locker = null; %>

<%
String url = request.getRequestURL().toString();
String target = "action=\""+url+"\"";

String restart = request.getParameter("restart");
String update = request.getParameter("update");
String dateval = request.getParameter("dateval");
String timeval = request.getParameter("timeval");
String lock    = request.getParameter("lock");
String unlock  = request.getParameter("unlock");

boolean normal = false;
if(restart == null && update == null && dateval == null && timeval == null && lock == null && unlock == null)
    normal = true;
if(dateval == null) dateval = "20160801";
if(timeval == null) timeval = "8:15";
String oldlocker = null;
if(lock != null)
{
    if(locker != null) oldlocker = locker;
    locker = lock;
}
if(unlock != null)
{
    if(locker != null) oldlocker = locker;
    locker = null;
}

Map<String,String> map = new HashMap<String,String>();
map.put("10.64.16.12","someone");

String ip = request.getRemoteAddr();
String user = map.get(ip);
if(user == null) user = "other";
%>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html><head>
<title>服务器管理</title>
</head><body>
服务器当前时间为:
<%
Date date = new Date();
out.print(date.toString()+"<br>");

String disabled = "";
if(locker != null)
//if(locker != null && !(user.equals(locker)))
{
    //disabled = "disabled=\"true\"";
    out.print("<br><br><form method=\"post\""+target+"><font color=red><b>服务器被 "+locker+" 占用！</b></font>&nbsp;&nbsp;");
    out.print("<input type=\"hidden\" name=\"unlock\"><input type=\"submit\" value=\"强制解除占用\"></form>");
}
%>

<br><form method="post" <%=target%>>
<p>设置日期和时间(日期必须8位):</p>
<input type="text" name="dateval" value="<%=dateval%>" size="8">
<input type="text" name="timeval" value="<%=timeval%>" size="5">
<input type="submit" value="确定" <%=disabled%>>
</form><br>

<form method="post" <%=target%>>
<input type="hidden" name="restart" value="a">
<input type="hidden" name="update">
<input type="submit" value="不更新资源重启"<%=disabled%>>
</form>

<br><a href="log.jsp">查看最近的日志</a><br>
<%
if(!(locker != null && !(user.equals(locker))))
    out.print("<a href=\""+url+"?lock="+user+"\">占用服务器(锁定不被重启)</a><br>");
if(locker != null && user.equals(locker))
    out.print("<a href=\""+url+"?unlock=\">解除占用服务器</a><br>");
%>

<%
if(normal) return;

String[] remotedate = {"sh", "-c", "ssh game@10.68.12.21 date +%Y/%m/%d-%H:%M:%S"};
String curdate = "";
try {
    Process proc = Runtime.getRuntime().exec(remotedate);
    InputStreamReader ir = new InputStreamReader(proc.getInputStream());
    LineNumberReader input = new LineNumberReader(ir);
    curdate = input.readLine();
} catch (IOException e) {
    e.printStackTrace();
}

if(lock != null) {
    String[] log = {"sh", "-c", "echo \""+curdate+" LOCK user:"+user+" from "+ip+" locker:"+locker+" oldlocker:"+oldlocker+"\" >> log.txt"};
    try {
        Runtime.getRuntime().exec(log);
    } catch (IOException e) {
        e.printStackTrace();
    }
    return;
}
if(unlock != null) {
    String[] log = {"sh", "-c", "echo \""+curdate+" UNLOCK user:"+user+" from "+ip+" oldlocker:"+oldlocker+"\" >> log.txt"};
    try {
        Runtime.getRuntime().exec(log);
    } catch (IOException e) {
        e.printStackTrace();
    }
    return;
}

if(restart != null)
{
    String[] log = {"sh", "-c", "echo \""+curdate+" "+user+" from "+ip+" restart "+restart+" "+update+"\" >> log.txt"};
    String[] cmd = {"sh", "-c", "./pass.sh "+restart+" "+update};
    try {
        Runtime.getRuntime().exec(log);
        Process proc = Runtime.getRuntime().exec(cmd);
        InputStreamReader ir = new InputStreamReader(proc.getInputStream());
        LineNumberReader input = new LineNumberReader(ir);
        String line;
        out.print("执行结果：<br>");
        while ((line = input.readLine()) != null) {
            out.print(line+"<br/>");
        }
    } catch (IOException e) {
        e.printStackTrace();
    }
}
else if(dateval != null && timeval != null)
{
    if(dateval.length() != 8) {
        out.print("日期格式错误！<br>");
        return;
    }
    String[] log = {"sh", "-c", "echo \""+curdate+" "+user+" from "+ip+" settime "+dateval+" "+timeval+"\" >> log.txt"};
    String[] cmd = {"sh", "-c", "date -s \""+dateval+" "+timeval+"\""};
    try {
        Runtime.getRuntime().exec(log);
        Process proc = Runtime.getRuntime().exec(cmd);
        InputStreamReader ir = new InputStreamReader(proc.getInputStream());
        LineNumberReader input = new LineNumberReader(ir);
        String line;
        out.print("执行结果：<br>");
        while ((line = input.readLine()) != null) {
            out.print(line+"<br/>");
        }
    } catch (IOException e) {
        e.printStackTrace();
    }
}
/*
#!/bin/sh
cd ~game
su game <<EOF
./restart.sh $1 $2;
EOF
*/
%>
</body></html>