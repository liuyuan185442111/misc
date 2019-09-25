JavaBean是一种java语言写成的可重用组件。JavaBean本质是一种特殊的java类，具有以下特点：
1.	JavaBean的类必须是具体的和公共的，类中如果有构造方法，那么这个构造方法也是public的并且是无参数的。
2.	类中的属性是private的，访问属性的方法都必须是public的。
3.	如果类的属性名是xxx，那么为了更改或获取属性，在类中可以使用两个public的方法，在这些get和set方法中，属性名的首字母为大写。get和set方法并不一定需要成对出现。
4.	对于boolean类型的成员变量，允许使用is方法代替上面的get方法，但是并不推荐。
5.	JavaBean处理表单方便，只要JavaBean属性和表单控件名称吻合，采用`<jsp:getProperty>`标签就可以直接得到表单提交的参数。
不过，我觉得只有第1条是必须遵守的，2-3是建议。

JavaBean的存放位置为\WEB-INF\classes，每更新或新添JavaBean，都需要重启tomcat才能生效。
##jsp:useBean
此标签用于在jsp页面中实例化一个或多个JavaBean组件，这些被实例化的JavaBean对象可以在jsp页面中被调用。它的语法格式为

	<jsp:useBean id="name" class="classname" scope="page|request|session|application" />
id是所创建的JavaBean实例的名称，class是JavaBean的包括完整路径的类名，scope是JavaBean实例对象的生命周期。
`<jsp:useBean>`兼具定义和声明的作用，如对象此前未定义则进行定义，如对象此前已定义则进行声明，实现过程是：用getAttribute获得对象，如果能获得表示对象已经定义过，如果不能获得自己new一个，再用setAttribute设置scope。
##jsp:getProperty
此标签用于从JavaBean获取相应的属性值，语法格式是

	<jsp:getProperty name="" property="" />
name对应`<jsp:useBean>`里的id，property是JavaBean的属性名。这个标签最终调用了getXxx函数，所以相应的get函数必须在JavaBean里定义。
##jsp:setProperty
此标签用来设置JavaBean相应的属性值。
按数据来源可分为两类。
1.从表单对象的参数中获取数据
自动匹配，语法格式为

	<jsp:setProperty name="title" property="*" />
接收来自表单输入的所有与属性名相同的参数值，它会自动匹配Bean中的属性，要保证JavaBean的属性名必须和request对象的参数名一致。

手动匹配，语法格式为

	<jsp:setProperty name="title" property="username" param="user" />
当表单对象中的参数名称与Bean的属性名称不一致时，则需要逐个设定属性值，而且要通过param指明属性值来自表单的哪个参数。如果Bean属性与request参数的名字相同，可以省略param，只需要指明property就行了。

2.直接赋值
指定任意值给JavaBean的属性赋值，语法格式为

	<jsp:setProperty name="title" property="username" value="string"|"<%=expression%>" />
如果将表达式的值设置为JavaBean属性的值，表达式值的类型必须和JavaBean属性的类型一致；如果将字符串设置为JavaBean属性的值，这个字符串将会自动转化成JavaBean属性的类型。

注意：
如果JavaBean属性无法被正确赋值，比如request对象的参数值中有空值，则JavaBean属性不被设定。与getProperty一样，相应的set函数也必须在JavaBean里定义。