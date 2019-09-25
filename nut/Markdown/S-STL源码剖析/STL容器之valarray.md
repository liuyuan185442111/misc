valarray，为高速计算设计，以一些简化编程和一般用途的开支做为代价，主要用于数学计算。
valarray被定义于&lt;valarray>中，sgi将其实现于诸多源文件中。
valarray不是STL标准容器。

看起来很烦，专业领域所用，也就不再深究，列出valarray的接口：
```cpp
template <typename T>
class valarray
{
public:
    typedef T value_type;

    // 构造和析构
    valarray();
    explicit valarray(size_t);
    valarray(const T&, size_t);
    valarray(const T*, size_t);
    valarray(const valarray&);
    valarray(const slice_array<T>&);
    valarray(const gslice_array<T>&);
    valarray(const mask_array<T>&);
    valarray(const indirect_array<T>&);
    ~valarray();

    // 赋值
    valarray<T>& operator=(const valarray<T>&);
    valarray<T>& operator=(const T&); // 赋给所有元素
    valarray<T>& operator=(const slice_array<T>&);
    valarray<T>& operator=(const gslice_array<T>&);
    valarray<T>& operator=(const mask_array<T>&);
    valarray<T>& operator=(const indirect_array<T>&);

    // 下标访问
    const T&          operator[](size_t) const;
    T&                operator[](size_t);
    valarray<T>       operator[](slice) const;
    slice_array<T>    operator[](slice);
    valarray<T>       operator[](const gslice&) const;
    gslice_array<T>   operator[](const gslice&);
    valarray<T>       operator[](const valarray<bool>&) const;
    mask_array<T>     operator[](const valarray<bool>&);
    valarray<T>       operator[](const valarray<size_t>&) const;
    indirect_array<T> operator[](const valarray<size_t>&);

    // 运算符, 其他未列出
    valarray<T>& operator*= (const T&);
    valarray<T>& operator*= (const valarray<T>&);

    // member functions
    size_t size() const;
    T    sum() const;
    T    min() const;
    T    max() const;
    void resize(size_t size, T c = T());

    // 移位, 正数左移, 负数右移
    valarray<T> shift (int) const;
    valarray<T> cshift(int) const; // 循环移位
    
    // 对所有元素执行func函数
    valarray<T> apply (T func(T)) const;
    valarray<T> apply (T func(const T&)) const;

private:
    size_t _M_size;
    T* _M_data;
};
```

另外，&lt;valarray>还将cmath里的大部分数学函数都进行了重载，使之可应用于valarray。

valarray直接调用operator new()来获取内存，用operator delete()释放内存，所以模板参数里并没有空间配置器。
sub-array
-
<font color=red_>**可以用operator[]()来获得sub-array，可以对sub-array直接赋值，可以用sub-array来构造valarray。**</font>
valarray定义了与此相关的slice，slice_array，gslice，gslice_array，mask_array，indirect_array几个辅助类。

cplusplus.com上的例子：
```cpp
// valarray::operator[] example
#include <iostream>     // std::cout
#include <valarray>     // std::valarray, std::slice, std::gslice

int main ()
{
  std::valarray<int> myarray(10);             //  0  0  0  0  0  0  0  0  0  0

  // slice:
  myarray[std::slice(2,3,3)]=10;              //  0  0 10  0  0 10  0  0 10  0

  // gslice:
  size_t lengths[]={2,2};
  size_t strides[]={6,2};
  myarray[std::gslice(1, std::valarray<size_t>(lengths,2), std::valarray<size_t>(strides,2))]=20;
                                              //  0 20 10 20  0 10  0 20 10 20

  // mask:
  std::valarray<bool> mymask(10);
  for(int i=0; i<10; ++i) mymask[i]= ((i%2)==0);
  myarray[mymask] += std::valarray<int>(3,5);  //  3 20 13 20  3 10  3 20 13 20

  // indirect:
  size_t sel[]= {2,5,7};
  std::valarray<size_t> myselection (sel,3);   //  3 20 99 20  3 99  3 99 13 20
  myarray[myselection]=99;

  std::cout << "myarray: ";
  for (size_t i=0; i<myarray.size(); ++i)
	  std::cout << myarray[i] << ' ';
  std::cout << '\n';

  return 0;
}
```
**slice**
slice类有两个构造函数，一个是默认的，另一个接受三个参数，依次是起始位置，元素个数，元素间隔。

**gslice**
gslice类有三个构造函数：

	gslice();
	gslice(size_t, const valarray<size_t>&, const valarray<size_t>&);
	gslice(const gslice&);
以第二个构造函数为例，a gslice with：

	start = 1
	size = {2, 3}
	stride = {7, 2}
would select：
```
                    [0][1][2][3][4][5][6][7][8][9][10][11][12][13]
start=1:                *
                        |
size=2, stride=7:       *--------------------*
                        |                    |
size=3, stride=2:       *-----*-----*        *------*------*
                        |     |     |        |      |      |
gslice:                 *     *     *        *      *      *
                    [0][1][2][3][4][5][6][7][8][9][10][11][12][13]
```
**mask**
选则参数mask中为true的元素对应valarray中的元素。

**indirect**
将indirect中的元素作为下标，选择对应valarray中的元素。

<font color=red_>**这四种方式都指定了valarray中部分元素，以它们作为参数调用operator[]()，将得到四种对应的array，然后可用这四种array构造valarray（通过构造函数或赋值运算符函数），或对原valarray进行赋值。**</font>

主要参考：http://www.cplusplus.com/reference/valarray/