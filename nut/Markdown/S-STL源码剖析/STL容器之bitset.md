bitset用来方便地管理一系列的bit位，它并不是一个标准的容器。
bitset定义于&lt;bitset>中：

	template <size_t N> class bitset;
##bitset的接口
构造函数
```cpp
// 默认构造函数, 初始化为全0
bitset();
// 以val初始化各比特位
bitset(unsigned long val);
// 用str从pos开始的n个字符初始化bitset的前n个比特, 这n个字符必须是0或1,
// 如果含有其他字符, 抛出invalid_argument异常,
// pos如果超出str范围抛出out_of_range异常
template<class charT, class traits, class Alloc>
  explicit bitset (const basic_string<charT,traits,Alloc>& str,
    typename basic_string<charT,traits,Alloc>::size_type pos = 0,
    typename basic_string<charT,traits,Alloc>::size_type n =
      basic_string<charT,traits,Alloc>::npos);
//没有显式的复制构造函数和赋值运算符，因为默认的就足够了
```
运算符
```cpp
// 成员函数
bitset& operator&=(const bitset& rhs);
bitset& operator|=(const bitset& rhs);
bitset& operator^=(const bitset& rhs);
bitset& operator<<=(size_t pos);
bitset& operator>>=(size_t pos);
bitset operator~() const;
bitset operator<<(size_t pos) const;
bitset operator>>(size_t pos) const;
bool operator==(const bitset& rhs) const;
bool operator!=(const bitset& rhs) const;
// 非成员函数
template <size_t N>
  bitset<N> operator&(const bitset<N>& lhs, const bitset<N>& rhs);
template <size_t N>
  bitset<N> operator|(const bitset<N>& lhs, const bitset<N>& rhs);
template <size_t N>
  bitset<N> operator^(const bitset<N>& lhs, const bitset<N>& rhs);
// 输入输出
template<class charT, class traits, size_t N>
  basic_istream<charT, traits>&
    operator>> (basic_istream<charT,traits>& is, bitset<N>& rhs);
template<class charT, class traits, size_t N>
  basic_ostream<charT, traits>&
    operator<< (basic_ostream<charT,traits>& os, const bitset<N>& rhs);
```
成员访问
```cpp
// 除了不进行越界检查, 与test()行为相同
bool operator[](size_t pos) const;
// 返回一个引用, 可进行赋值操作
reference operator[](size_t pos);
// 返回bitset中1的个数
size_t count() const;
// 返回bitset的大小
size_t size() const;
// 检测是否为1, 可能会抛出out_of_range异常
bool test(size_t pos) const;
// 如存在任意一个为1的位则返回true
bool any() const;
// 全零返回true, 等价于!any();
bool none() const;
```
位操作
```
// 将所有位置为1
bitset& set();
// 将相应位置为val, 可能会抛出out_of_range异常
bitset& set(size_t pos, bool val = true);
// 将所有位清零
bitset& reset();
// 将相应位清零, 可能会抛出out_of_range异常
bitset& reset(size_t pos);
// 翻转所有位
bitset& flip();
// 翻转相应位, 可能会抛出out_of_range异常
bitset& flip(size_t pos);
```
转换
```
// 返回一个01字符串, 与<<运算符产生的输出相同
template <class charT, class traits, class Alloc>
  basic_string<charT,traits,Alloc> to_string() const;
// 返回一个unsigned long, 如果bitset太大以至于无法转换, 抛出overflow_error异常
unsigned long to_ulong() const;
```
##解析
###内部数据结构
bitset内部以unsigned long数组来存储各比特位，所以如果unsigned long占4字节的话，sizeof(bitset对象)的值总是4的整数倍。哦，sizeof(bitset<0>)除外，此时unsigned long数组长度为0，bitset<0>就是一个没有数据成员的类，其大小是1。
在sgi的实现里，定义了_Base_bitset的模板类，bitset私有继承此类，bitset的大部分操作都在_Base_bitset里完成，unsigned long数组也定义在_Base_bitset里：

	unsigned long M_w[Nw];
其中Nw是所需unsigned long的数目，可由(n+31)/32算得。
M_w数组的下标越大，所表示比特序列的位数越高。
_Base_bitset还针对只有一个unsigned long的情况进行了特殊化，提高了效率。
###查表法
count()内部使用了查表法。先定义了一个256长度的unsigned char数组，用来存储所有unsigned char对应的1的个数。将每个unsigned long分成4个unsigned char，然后以unsigned char为数组下标直接可获得1的个数，依次累加即可获得最终结果。
###未使用部分
由于unsigned long数组不一定恰好用完，未使用部分由一个内部函数进行清零操作。在构造对象，或进行有可能更改未使用部分的操作时，都调用了此函数。所以，未使用的部分始终是0。
##reference类型
bitset内部定义了一个reference类，它是operator[]其中一个版本的返回值，operator[]有两个版本：

	bool operator[] (size_t pos) const;
	reference operator[] (size_t pos);
第一个版本只读，第二个版本可以写。reference就是为了实现operator[]写的功能。

	reference operator[](size_t pos) { return reference(*this, pos); }
reference有一个unsigned long *M_wp的数据成员，通过

	reference( bitset& b, size_t pos );
进行构造时，M_wp会指向pos对应的那个unsigned long，这样就可以通过M_wp来对b进行操作了。
除了可以对b[i]进行赋值操作，还可以对它进行`~b[i]和b[i].flip()`。
##完整源码（摘自std_bitset.h，有修改）
```cpp
// 内部以unsigned long来存储比特位, 所以占用字节数是sizeof(unsigned long)的整数倍
// 大小为n的bitset占用的unsigned long的数量为__BITSET_WORDS(n)
// 在unsigned long占4字节的机器, bitset<0>占1字节, bitset<1>占4字节, bitset<32>占4字节, bitset<33>占8字节
// 未使用比特始终是0
#define _GLIBCPP_BITSET_BITS_PER_WORD (CHAR_BIT*sizeof(unsigned long))
#define __BITSET_WORDS(__n) \
 ((__n) < 1 ? 1 : ((__n) + _GLIBCPP_BITSET_BITS_PER_WORD - 1)/_GLIBCPP_BITSET_BITS_PER_WORD)
/**
// gcc 4.7.1里是这样的
#define _GLIBCXX_BITSET_WORDS(__n) \
  ((__n) / _GLIBCXX_BITSET_BITS_PER_WORD + \
   ((__n) % _GLIBCXX_BITSET_BITS_PER_WORD == 0 ? 0 : 1))

   区别在于当n为0的时候, 前者是1, 后者是0
   实际上, 当n为0的时, _Base_bitset的_M_w数组大小为0即可, 此时对象占用空间为1
   不做n<1的判断即等价:
   #define __BITSET_WORDS(__n) \
 (((__n) + _GLIBCPP_BITSET_BITS_PER_WORD - 1)/_GLIBCPP_BITSET_BITS_PER_WORD)
*/

// 查表法获得unsigned char中1的个数, 可在哪初始化的, 我咋没找着
template<bool __dummy>
struct _Bit_count {
  static unsigned char _S_bit_count[256];
};

// Mapping from 8 bit unsigned integers to the index of the first one bit
template<bool __dummy>
struct _First_one {
  static unsigned char _S_first_one[256];
};


// _Nw是bitset占用Word的个数
template<size_t _Nw>
struct _Base_bitset {
  typedef unsigned long _WordT;

  _WordT _M_w[_Nw]; // 实际的存储位置, 从低位到高位

  _Base_bitset( void ) { _M_do_reset(); }
  _Base_bitset(unsigned long __val) {
    _M_do_reset();
    _M_w[0] = __val;
  }
  void _M_do_reset() { memset(_M_w, 0, _Nw * sizeof(_WordT)); }


  static size_t _S_whichword( size_t __pos )
    { return __pos / _GLIBCPP_BITSET_BITS_PER_WORD; }
  static size_t _S_whichbyte( size_t __pos )
    { return (__pos % _GLIBCPP_BITSET_BITS_PER_WORD) / CHAR_BIT; }
  static size_t _S_whichbit( size_t __pos )
    { return __pos % _GLIBCPP_BITSET_BITS_PER_WORD; }
    // 获得掩码, 以Word为单位操作
  static _WordT _S_maskbit( size_t __pos )
    { return (static_cast<_WordT>(1)) << _S_whichbit(__pos); }

  _WordT& _M_getword(size_t __pos)       { return _M_w[_S_whichword(__pos)]; }
  _WordT  _M_getword(size_t __pos) const { return _M_w[_S_whichword(__pos)]; }

  // 最高位Word
  _WordT& _M_hiword()       { return _M_w[_Nw - 1]; }
  _WordT  _M_hiword() const { return _M_w[_Nw - 1]; }

  void _M_do_and(const _Base_bitset<_Nw>& __x) {
    for ( size_t __i = 0; __i < _Nw; __i++ ) {
      _M_w[__i] &= __x._M_w[__i];
    }
  }

  void _M_do_or(const _Base_bitset<_Nw>& __x) {
    for ( size_t __i = 0; __i < _Nw; __i++ ) {
      _M_w[__i] |= __x._M_w[__i];
    }
  }

  void _M_do_xor(const _Base_bitset<_Nw>& __x) {
    for ( size_t __i = 0; __i < _Nw; __i++ ) {
      _M_w[__i] ^= __x._M_w[__i];
    }
  }

  void _M_do_left_shift(size_t __shift);
  void _M_do_right_shift(size_t __shift);

  void _M_do_flip() {
    for ( size_t __i = 0; __i < _Nw; __i++ ) {
      _M_w[__i] = ~_M_w[__i];
    }
  }

  void _M_do_set() {
    for ( size_t __i = 0; __i < _Nw; __i++ ) {
      _M_w[__i] = ~static_cast<_WordT>(0);
    }
  }

  bool _M_is_equal(const _Base_bitset<_Nw>& __x) const {
    for (size_t __i = 0; __i < _Nw; ++__i) {
      if (_M_w[__i] != __x._M_w[__i])
        return false;
    }
    return true;
  }

  bool _M_is_any() const {
    for ( size_t __i = 0; __i < _Nw; __i++ ) {
      if ( _M_w[__i] != static_cast<_WordT>(0) )
        return true;
    }
    return false;
  }

  // 统计1的个数, 直接强转成char*, 灵活极了
  size_t _M_do_count() const {
    size_t __result = 0;
    const unsigned char* __byte_ptr = (const unsigned char*)_M_w;
    const unsigned char* __end_ptr = (const unsigned char*)(_M_w+_Nw);

    while ( __byte_ptr < __end_ptr ) {
      // 用查表法得到unsigned char中1的个数
      __result += _Bit_count<true>::_S_bit_count[*__byte_ptr];
      __byte_ptr++;
    }
    return __result;
  }

  unsigned long _M_do_to_ulong() const;

  // find first "on" bit
  size_t _M_do_find_first(size_t __not_found) const;

  // find the next "on" bit that follows "prev"
  size_t _M_do_find_next(size_t __prev, size_t __not_found) const;
};


// _Base_bitset的非内联函数定义
template<size_t _Nw>
void _Base_bitset<_Nw>::_M_do_left_shift(size_t __shift)
{
  if (__shift != 0) {
    const size_t __wshift = __shift / _GLIBCPP_BITSET_BITS_PER_WORD;
    const size_t __offset = __shift % _GLIBCPP_BITSET_BITS_PER_WORD;

    if (__offset == 0)
      for (size_t __n = _Nw - 1; __n >= __wshift; --__n)
        _M_w[__n] = _M_w[__n - __wshift];

    else {
      const size_t __sub_offset = _GLIBCPP_BITSET_BITS_PER_WORD - __offset;
      for (size_t __n = _Nw - 1; __n > __wshift; --__n)
        _M_w[__n] = (_M_w[__n - __wshift] << __offset) |
                    (_M_w[__n - __wshift - 1] >> __sub_offset);
      _M_w[__wshift] = _M_w[0] << __offset;
    }

    fill(_M_w + 0, _M_w + __wshift, static_cast<_WordT>(0));
  }
}

template<size_t _Nw>
void _Base_bitset<_Nw>::_M_do_right_shift(size_t __shift)
{
  if (__shift != 0) {
    const size_t __wshift = __shift / _GLIBCPP_BITSET_BITS_PER_WORD;
    const size_t __offset = __shift % _GLIBCPP_BITSET_BITS_PER_WORD;
    const size_t __limit = _Nw - __wshift - 1;

    if (__offset == 0)
      for (size_t __n = 0; __n <= __limit; ++__n)
        _M_w[__n] = _M_w[__n + __wshift];

    else {
      const size_t __sub_offset = _GLIBCPP_BITSET_BITS_PER_WORD - __offset;
      for (size_t __n = 0; __n < __limit; ++__n)
        _M_w[__n] = (_M_w[__n + __wshift] >> __offset) |
                    (_M_w[__n + __wshift + 1] << __sub_offset);
      _M_w[__limit] = _M_w[_Nw-1] >> __offset;
    }

    fill(_M_w + __limit + 1, _M_w + _Nw, static_cast<_WordT>(0));
  }
}

// 仅返回最低字, 如高位有数据, 抛出overflow_error异常
template<size_t _Nw>
unsigned long _Base_bitset<_Nw>::_M_do_to_ulong() const
{
  for (size_t __i = 1; __i < _Nw; ++__i)
    if (_M_w[__i])
      __STL_THROW(overflow_error("bitset"));

  return _M_w[0];
}

// 同样使用了查表法
template<size_t _Nw>
size_t _Base_bitset<_Nw>::_M_do_find_first(size_t __not_found) const
{
  for ( size_t __i = 0; __i < _Nw; __i++ ) {
    _WordT __thisword = _M_w[__i];
    if ( __thisword != static_cast<_WordT>(0) ) {
      // find byte within word
      for ( size_t __j = 0; __j < sizeof(_WordT); __j++ ) {
        unsigned char __this_byte
          = static_cast<unsigned char>(__thisword & (~(unsigned char)0));
        if ( __this_byte )
          return __i*_GLIBCPP_BITSET_BITS_PER_WORD + __j*CHAR_BIT +
            _First_one<true>::_S_first_one[__this_byte];

        __thisword >>= CHAR_BIT;
      }
    }
  }
  // not found, so return an indication of failure.
  return __not_found;
}

template<size_t _Nw>
size_t
_Base_bitset<_Nw>::_M_do_find_next(size_t __prev, size_t __not_found) const
{
  // make bound inclusive
  ++__prev;

  // check out of bounds
  if ( __prev >= _Nw * _GLIBCPP_BITSET_BITS_PER_WORD )
    return __not_found;

    // search first word
  size_t __i = _S_whichword(__prev);
  _WordT __thisword = _M_w[__i];

    // mask off bits below bound
  __thisword &= (~static_cast<_WordT>(0)) << _S_whichbit(__prev);

  if ( __thisword != static_cast<_WordT>(0) ) {
    // find byte within word
    // get first byte into place
    __thisword >>= _S_whichbyte(__prev) * CHAR_BIT;
    for ( size_t __j = _S_whichbyte(__prev); __j < sizeof(_WordT); __j++ ) {
      unsigned char __this_byte
        = static_cast<unsigned char>(__thisword & (~(unsigned char)0));
      if ( __this_byte )
        return __i*_GLIBCPP_BITSET_BITS_PER_WORD + __j*CHAR_BIT +
          _First_one<true>::_S_first_one[__this_byte];

      __thisword >>= CHAR_BIT;
    }
  }

  // check subsequent words
  __i++;
  for ( ; __i < _Nw; __i++ ) {
    __thisword = _M_w[__i];
    if ( __thisword != static_cast<_WordT>(0) ) {
      // find byte within word
      for ( size_t __j = 0; __j < sizeof(_WordT); __j++ ) {
        unsigned char __this_byte
          = static_cast<unsigned char>(__thisword & (~(unsigned char)0));
        if ( __this_byte )
          return __i*_GLIBCPP_BITSET_BITS_PER_WORD + __j*CHAR_BIT +
            _First_one<true>::_S_first_one[__this_byte];

        __thisword >>= CHAR_BIT;
      }
    }
  }

  // not found, so return an indication of failure.
  return __not_found;
}


// 只有一个word时的特化
template<> struct _Base_bitset<1> {
  typedef unsigned long _WordT;
  _WordT _M_w;

  _Base_bitset( void ) : _M_w(0) {}
  _Base_bitset(unsigned long __val) : _M_w(__val) {}

  static size_t _S_whichword( size_t __pos )
    { return __pos / _GLIBCPP_BITSET_BITS_PER_WORD; }
  static size_t _S_whichbyte( size_t __pos )
    { return (__pos % _GLIBCPP_BITSET_BITS_PER_WORD) / CHAR_BIT; }
  static size_t _S_whichbit( size_t __pos )
    {  return __pos % _GLIBCPP_BITSET_BITS_PER_WORD; }
  static _WordT _S_maskbit( size_t __pos )
    { return (static_cast<_WordT>(1)) << _S_whichbit(__pos); }

  _WordT& _M_getword(size_t)       { return _M_w; }
  _WordT  _M_getword(size_t) const { return _M_w; }

  _WordT& _M_hiword()       { return _M_w; }
  _WordT  _M_hiword() const { return _M_w; }

  void _M_do_and(const _Base_bitset<1>& __x) { _M_w &= __x._M_w; }
  void _M_do_or(const _Base_bitset<1>& __x)  { _M_w |= __x._M_w; }
  void _M_do_xor(const _Base_bitset<1>& __x) { _M_w ^= __x._M_w; }
  void _M_do_left_shift(size_t __shift)     { _M_w <<= __shift; }
  void _M_do_right_shift(size_t __shift)    { _M_w >>= __shift; }
  void _M_do_flip()                       { _M_w = ~_M_w; }
  void _M_do_set()                        { _M_w = ~static_cast<_WordT>(0); }
  void _M_do_reset()                      { _M_w = 0; }

  bool _M_is_equal(const _Base_bitset<1>& __x) const
    { return _M_w == __x._M_w; }
  bool _M_is_any() const
    { return _M_w != 0; }

  size_t _M_do_count() const {
    size_t __result = 0;
    const unsigned char* __byte_ptr = (const unsigned char*)&_M_w;
    const unsigned char* __end_ptr
      = ((const unsigned char*)&_M_w)+sizeof(_M_w);
    while ( __byte_ptr < __end_ptr ) {
      __result += _Bit_count<true>::_S_bit_count[*__byte_ptr];
      __byte_ptr++;
    }
    return __result;
  }

  unsigned long _M_do_to_ulong() const { return _M_w; }

  size_t _M_do_find_first(size_t __not_found) const;

  // find the next "on" bit that follows "prev"
  size_t _M_do_find_next(size_t __prev, size_t __not_found) const;

};


// 辅助类, 用来将最高word没有用到的比特清零
template <size_t _Extrabits> struct _Sanitize {
  static void _M_do_sanitize(unsigned long& __val)
    { __val &= ~((~static_cast<unsigned long>(0)) << _Extrabits); }
};

template<> struct _Sanitize<0> {
  static void _M_do_sanitize(unsigned long) {}
};


// Class bitset
template<size_t _Nb>
class bitset : private _Base_bitset<__BITSET_WORDS(_Nb)>
{
private:
  typedef _Base_bitset<__BITSET_WORDS(_Nb)> _Base;
  typedef unsigned long _WordT;

private:
  // 将最高word没有用到的比特清零
  void _M_do_sanitize() {
    _Sanitize<_Nb%_GLIBCPP_BITSET_BITS_PER_WORD>::_M_do_sanitize(this->_M_hiword());
  }

public:

  // bit reference:
  class reference;
  friend class reference;

  class reference {
    friend class bitset;

    _WordT *_M_wp;
    size_t _M_bpos;

    // left undefined
    reference();

  public:
    reference( bitset& __b, size_t __pos ) {
      _M_wp = &__b._M_getword(__pos);
      _M_bpos = _Base::_S_whichbit(__pos);
    }

    ~reference() {}

    // for b[i] = __x;
    reference& operator=(bool __x) {
      if ( __x )
        *_M_wp |= _Base::_S_maskbit(_M_bpos);
      else
        *_M_wp &= ~_Base::_S_maskbit(_M_bpos);

      return *this;
    }

    // for b[i] = b[__j];
    reference& operator=(const reference& __j) {
      if ( (*(__j._M_wp) & _Base::_S_maskbit(__j._M_bpos)) )
        *_M_wp |= _Base::_S_maskbit(_M_bpos);
      else
        *_M_wp &= ~_Base::_S_maskbit(_M_bpos);

      return *this;
    }

    // flips the bit
    bool operator~() const
      { return (*(_M_wp) & _Base::_S_maskbit(_M_bpos)) == 0; }

    // for __x = b[i];
    operator bool() const
      { return (*(_M_wp) & _Base::_S_maskbit(_M_bpos)) != 0; }

    // for b[i].flip();
    reference& flip() {
      *_M_wp ^= _Base::_S_maskbit(_M_bpos);
      return *this;
    }
  };

  // constructors:
  bitset() {}
  bitset(unsigned long __val) : _Base_bitset<__BITSET_WORDS(_Nb)>(__val)
    { _M_do_sanitize(); }

  template<class _CharT, class _Traits, class _Alloc>
  explicit bitset(const basic_string<_CharT, _Traits, _Alloc>& __s,
                  size_t __pos = 0)
    : _Base()
  {
    if (__pos > __s.size())
      __STL_THROW(out_of_range("bitset"));
    _M_copy_from_string(__s, __pos,
                        basic_string<_CharT, _Traits, _Alloc>::npos);
  }
  template<class _CharT, class _Traits, class _Alloc>
  bitset(const basic_string<_CharT, _Traits, _Alloc>& __s,
         size_t __pos,
         size_t __n)
    : _Base()
  {
    if (__pos > __s.size())
      __STL_THROW(out_of_range("bitset"));
    _M_copy_from_string(__s, __pos, __n);
  }

  // bitset operations:
  bitset<_Nb>& operator&=(const bitset<_Nb>& __rhs) {
    this->_M_do_and(__rhs);
    return *this;
  }

  bitset<_Nb>& operator|=(const bitset<_Nb>& __rhs) {
    this->_M_do_or(__rhs);
    return *this;
  }

  bitset<_Nb>& operator^=(const bitset<_Nb>& __rhs) {
    this->_M_do_xor(__rhs);
    return *this;
  }

  bitset<_Nb>& operator<<=(size_t __pos) {
    this->_M_do_left_shift(__pos);
    this->_M_do_sanitize();
    return *this;
  }

  bitset<_Nb>& operator>>=(size_t __pos) {
    this->_M_do_right_shift(__pos);
    this->_M_do_sanitize();
    return *this;
  }

  // 单比特操作
  bitset<_Nb>& _Unchecked_set(size_t __pos) {
    this->_M_getword(__pos) |= _Base::_S_maskbit(__pos);
    return *this;
  }

  bitset<_Nb>& _Unchecked_set(size_t __pos, int __val) {
    if (__val)
      this->_M_getword(__pos) |= _Base::_S_maskbit(__pos);
    else
      this->_M_getword(__pos) &= ~_Base::_S_maskbit(__pos);

    return *this;
  }

  bitset<_Nb>& _Unchecked_reset(size_t __pos) {
    this->_M_getword(__pos) &= ~_Base::_S_maskbit(__pos);
    return *this;
  }

  bitset<_Nb>& _Unchecked_flip(size_t __pos) {
    this->_M_getword(__pos) ^= _Base::_S_maskbit(__pos);
    return *this;
  }

  bool _Unchecked_test(size_t __pos) const {
    return (this->_M_getword(__pos) & _Base::_S_maskbit(__pos))
      != static_cast<_WordT>(0);
  }

  // Set, reset, and flip
  bitset<_Nb>& set() {
    this->_M_do_set();
    this->_M_do_sanitize();
    return *this;
  }

  bitset<_Nb>& set(size_t __pos, bool __val = true) {
    if (__pos >= _Nb)
      __STL_THROW(out_of_range("bitset"));

    return _Unchecked_set(__pos, __val);
  }

  bitset<_Nb>& reset() {
    this->_M_do_reset();
    return *this;
  }

  bitset<_Nb>& reset(size_t __pos) {
    if (__pos >= _Nb)
      __STL_THROW(out_of_range("bitset"));

    return _Unchecked_reset(__pos);
  }

  bitset<_Nb>& flip() {
    this->_M_do_flip();
    this->_M_do_sanitize();
    return *this;
  }

  bitset<_Nb>& flip(size_t __pos) {
    if (__pos >= _Nb)
      __STL_THROW(out_of_range("bitset"));

    return _Unchecked_flip(__pos);
  }

  bitset<_Nb> operator~() const {
    return bitset<_Nb>(*this).flip();
  }

  // element access
  reference operator[](size_t __pos) { return reference(*this,__pos); }
  bool operator[](size_t __pos) const { return _Unchecked_test(__pos); }

  unsigned long to_ulong() const { return this->_M_do_to_ulong(); }

  template <class _CharT, class _Traits, class _Alloc>
  basic_string<_CharT, _Traits, _Alloc> to_string() const {
    basic_string<_CharT, _Traits, _Alloc> __result;
    _M_copy_to_string(__result);
    return __result;
  }

  // Helper functions for string operations.
  template<class _CharT, class _Traits, class _Alloc>
  void _M_copy_from_string(const basic_string<_CharT,_Traits,_Alloc>& __s,
                          size_t, size_t);

  template<class _CharT, class _Traits, class _Alloc>
  void _M_copy_to_string(basic_string<_CharT,_Traits,_Alloc>&) const;

  size_t count() const { return this->_M_do_count(); }

  size_t size() const { return _Nb; }

  bool operator==(const bitset<_Nb>& __rhs) const {
    return this->_M_is_equal(__rhs);
  }
  bool operator!=(const bitset<_Nb>& __rhs) const {
    return !this->_M_is_equal(__rhs);
  }

  bool test(size_t __pos) const {
    if (__pos >= _Nb)
      __STL_THROW(out_of_range("bitset"));
    return _Unchecked_test(__pos);
  }

  bool any() const { return this->_M_is_any(); }
  bool none() const { return !this->_M_is_any(); }

  bitset<_Nb> operator<<(size_t __pos) const
    { return bitset<_Nb>(*this) <<= __pos; }
  bitset<_Nb> operator>>(size_t __pos) const
    { return bitset<_Nb>(*this) >>= __pos; }

  // 扩展部分, 非标准的一部分, 不要依赖这些非标准部分
  // find the index of the first "on" bit
  size_t _Find_first() const
    { return this->_M_do_find_first(_Nb); }

  // find the index of the next "on" bit after prev
  size_t _Find_next( size_t __prev ) const
    { return this->_M_do_find_next(__prev, _Nb); }
};

// bitset和string的相互转换函数
template <size_t _Nb>
template<class _CharT, class _Traits, class _Alloc>
void bitset<_Nb>
  ::_M_copy_from_string(const basic_string<_CharT,_Traits,_Alloc>& __s,
                        size_t __pos, size_t __n)
{
  reset();
  const size_t __nbits = min(_Nb, min(__n, __s.size() - __pos));
  for (size_t __i = 0; __i < __nbits; ++__i) {
    switch(__s[__pos + __nbits - __i - 1]) {
    case '0':
      break;
    case '1':
      set(__i);
      break;
    default:
      __STL_THROW(invalid_argument("bitset"));
    }
  }
}

template <size_t _Nb>
template <class _CharT, class _Traits, class _Alloc>
void bitset<_Nb>
  ::_M_copy_to_string(basic_string<_CharT, _Traits, _Alloc>& __s) const
{
  __s.assign(_Nb, '0');

  for (size_t __i = 0; __i < _Nb; ++__i)
    if (_Unchecked_test(__i))
      __s[_Nb - 1 - __i] = '1';
}


// 运算符函数:与,或,异或,<<,>>
template <size_t _Nb>
inline bitset<_Nb> operator&(const bitset<_Nb>& __x, const bitset<_Nb>& __y) {
  bitset<_Nb> __result(__x);
  __result &= __y;
  return __result;
}

template <size_t _Nb>
inline bitset<_Nb> operator|(const bitset<_Nb>& __x, const bitset<_Nb>& __y) {
  bitset<_Nb> __result(__x);
  __result |= __y;
  return __result;
}

template <size_t _Nb>
inline bitset<_Nb> operator^(const bitset<_Nb>& __x, const bitset<_Nb>& __y) {
  bitset<_Nb> __result(__x);
  __result ^= __y;
  return __result;
}

// 先将输入读入一个临时字符串, 再用字符串初始化bitset, 可能会抛出invalid_argument异常
template <class _CharT, class _Traits, size_t _Nb>
basic_istream<_CharT, _Traits>&
operator>>(basic_istream<_CharT, _Traits>& __is, bitset<_Nb>& __x)
{
  typedef typename _Traits::char_type char_type;
  basic_string<_CharT, _Traits> __tmp;
  __tmp.reserve(_Nb);

  // Skip whitespace
  typename basic_istream<_CharT, _Traits>::sentry __sentry(__is);
  if (__sentry) {
    basic_streambuf<_CharT, _Traits>* __buf = __is.rdbuf();
    for (size_t __i = 0; __i < _Nb; ++__i) {
      static typename _Traits::int_type __eof = _Traits::eof();

      typename _Traits::int_type __c1 = __buf->sbumpc();
      if (_Traits::eq_int_type(__c1, __eof)) {
        __is.setstate(ios_base::eofbit);
        break;
      }
      else {
        char_type __c2 = _Traits::to_char_type(__c1);
        char_type __c  = __is.narrow(__c2, '*');

        if (__c == '0' || __c == '1')
          __tmp.push_back(__c);
        else if (_Traits::eq_int_type(__buf->sputbackc(__c2), __eof)) {
          __is.setstate(ios_base::failbit);
          break;
        }
      }
    }

    if (__tmp.empty())
      __is.setstate(ios_base::failbit);
    else
      __x._M_copy_from_string(__tmp, static_cast<size_t>(0), _Nb);
  }

  return __is;
}

// 先得到对应的字符串, 再输出
template <class _CharT, class _Traits, size_t _Nb>
basic_ostream<_CharT, _Traits>&
operator<<(basic_ostream<_CharT, _Traits>& __os, const bitset<_Nb>& __x)
{
  basic_string<_CharT, _Traits> __tmp;
  __x._M_copy_to_string(__tmp);
  return __os << __tmp;
}
```