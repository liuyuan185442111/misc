- 删除容器中有特定值的所有元素：
如果容器是vector、string、deque，则使用erase-remove用法
如果容器是list，则使用list::remove
如果容器是一个标准关联容器，则使用它的erase成员函数

- 删除容器中满足特定条件的所有元素：
如果容器是vector、string、deque，则使用erase-remove_if用法
如果容器是list，则使用list::remove_if
如果容器是一个标准关联容器，写一个循环来遍历容器中的元素，在循环中使用erase方法删除元素，当把迭代器传给erase时，要对它进行后缀递增

- 如果要在循环内部做些操作，只能手写一个循环了。标准关联容器本来就需要一个循环；而对于vector、string、deque，每次调用erase时，要返回它的返回值以更新迭代；对于list，两种方式都可以。

[remove系列函数](http://blog.csdn.net/liuyuan185442111/article/details/46573515#t12)，they cannot alter the size of an array or a container，所以需要使用erase来删掉后面无用的元素。

```
//ex 1
vector<int> c;
c.erase(remove(c.begin(), c.end(), 2017), c.end();

//ex 2
map<int> c;
c.erase(2017);

//ex 3
list<int> c;
c.remove(2017);

//ex 4
map<int> c;
for(map<int>::iterator iter = c.begin(); iter != c.end(); ) 
{
	//erase(iter++):先创建一个临时变量,用iter的值初始化,然后iter自加,然后将临时变量传递给erase函数
	if(predict(*iter)) c.erase(iter++);
	else ++iter;
}

//ex 5
vector<int> c;
for(vector<int>::iterator iter = c.begin(); iter != c.end(); )
{
	if(predict(*iter)) iter = c.erase(iter);
	else ++iter;
}
```
iterator vector::erase (iterator pos);
返回被删除序列之后的元素的位置，被删除位置及之后的原有迭代器将失效。
Invalidate iterators and references at or after the point of the erase, including the end() iterator.

void map::erase (iterator position);
当容器中一个元素被删除时，指向该元素的所有迭代器都将失效。
References and iterators to the erased elements are invalidated. Other references and iterators are not affected.

reference：
effective STL item 9
[cppreference](http://en.cppreference.com)
![vector](http://img.blog.csdn.net/20170322210331182?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvbGl1eXVhbjE4NTQ0MjExMQ==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)
![map](http://img.blog.csdn.net/20170322210344688?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvbGl1eXVhbjE4NTQ0MjExMQ==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)