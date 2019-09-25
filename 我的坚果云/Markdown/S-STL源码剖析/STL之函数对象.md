函数对象和其适配器定义于&lt;functional>中，sgi将其实现于stl_function.h中，《STL源码剖析》第8章描述清晰，这里仅将源码列出：
```cpp
template <typename Arg, typename Result>
struct unary_function
{
    typedef Arg    argument_type;
    typedef Result result_type;
};

template <typename Arg1, typename Arg2, typename Result>
struct binary_function
{
    typedef Arg1   first_argument_type;
    typedef Arg2   second_argument_type;
    typedef Result result_type;
};


/******************** 简单的函数对象 **********************/
// 算术运算
template <typename T>
struct plus : public binary_function<T,T,T>
{
    T operator()(const T& x, const T& y) const
    { return x + y; }
};

template <typename T>
struct minus : public binary_function<T,T,T>
{
    T operator()(const T& x, const T& y) const
    { return x - y; }
};

template <typename T>
struct multiplies : public binary_function<T,T,T>
{
    T operator()(const T& x, const T& y) const
    { return x * y; }
};

template <typename T>
struct divides : public binary_function<T,T,T>
{
    T operator()(const T& x, const T& y) const
    { return x / y; }
};

template <typename T>
struct modulus : public binary_function<T,T,T>
{
    T operator()(const T& x, const T& y) const
    { return x % y; }
};

template <typename T>
struct negate : public unary_function<T,T>
{
    T operator()(const T& x) const
    { return -x; }
};

// 比较运算
template <typename T>
struct equal_to : public binary_function<T,T,bool>
{
    bool operator()(const T& x, const T& y) const
    { return x == y; }
};

template <typename T>
struct not_equal_to : public binary_function<T,T,bool>
{
    bool operator()(const T& x, const T& y) const
    { return x != y; }
};

template <typename T>
struct greater : public binary_function<T,T,bool>
{
    bool operator()(const T& x, const T& y) const
    { return x > y; }
};

template <typename T>
struct less : public binary_function<T,T,bool>
{
    bool operator()(const T& x, const T& y) const
    { return x < y; }
};

template <typename T>
struct greater_equal : public binary_function<T,T,bool>
{
    bool operator()(const T& x, const T& y) const
    { return x >= y; }
};

template <typename T>
struct less_equal : public binary_function<T,T,bool>
{
    bool operator()(const T& x, const T& y) const
    { return x <= y; }
};

// 逻辑运算
template <typename T>
struct logical_and : public binary_function<T,T,bool>
{
    bool operator()(const T& x, const T& y) const
    { return x && y; }
};

template <typename T>
struct logical_or : public binary_function<T,T,bool>
{
    bool operator()(const T& x, const T& y) const
    { return x || y; }
};

template <typename T>
struct logical_not : public unary_function<T,bool>
{
    bool operator()(const T& x) const
    { return !x; }
};

/********************** 适配器 **********************/
// 函数对象的适配器一般是和算法配合使用的

// 将一元函数对象适配成一元取非函数对象
template <typename Predicate>
class unary_negate
    : public unary_function<typename Predicate::argument_type, bool>
{
protected:
    Predicate _M_pred;
public:
    explicit unary_negate(const Predicate& x) : _M_pred(x) {}
    bool operator()(const typename Predicate::argument_type& x) const
    {
        return !_M_pred(x);
    }
};

template <typename Predicate>
inline unary_negate<Predicate>
not1(const Predicate& pred)
{
    return unary_negate<Predicate>(pred);
}

// 将二元函数对象适配成二元取非函数对象
template <typename Predicate>
class binary_negate
    : public binary_function<typename Predicate::first_argument_type,
      typename Predicate::second_argument_type, bool>
{
protected:
    Predicate _M_pred;
public:
    explicit binary_negate(const Predicate& x) : _M_pred(x) {}
    bool operator()(const typename Predicate::first_argument_type& x,
                    const typename Predicate::second_argument_type& y) const
    {
        return !_M_pred(x, y);
    }
};

template <typename Predicate>
inline binary_negate<Predicate>
not2(const Predicate& pred)
{
    return binary_negate<Predicate>(pred);
}

// 将二元函数第一个参数进行绑定, 这样就适配成为一元函数
template <typename Operation>
class binder1st
    : public unary_function<typename Operation::second_argument_type,
      typename Operation::result_type>
{
protected:
    Operation op;
    typename Operation::first_argument_type value;
public:
    binder1st(const Operation& x,
              const typename Operation::first_argument_type& y)
        : op(x), value(y) {}
    typename Operation::result_type
    operator()(const typename Operation::second_argument_type& x) const
    {
        return op(value, x);
    }
};

template <typename Operation, typename T>
inline binder1st<Operation>
bind1st(const Operation& fn, const T& x)
{
    typedef typename Operation::first_argument_type Arg1_type;
    return binder1st<Operation>(fn, Arg1_type(x));
}

// 将二元函数第二个参数进行绑定, 这样就适配成为一元函数
template <typename Operation>
class binder2nd
    : public unary_function<typename Operation::first_argument_type,
      typename Operation::result_type>
{
protected:
    Operation op;
    typename Operation::second_argument_type value;
public:
    binder2nd(const Operation& x,
              const typename Operation::second_argument_type& y)
        : op(x), value(y) {}
    typename Operation::result_type
    operator()(const typename Operation::first_argument_type& x) const
    {
        return op(x, value);
    }
};

template <typename Operation, typename T>
inline binder2nd<Operation>
bind2nd(const Operation& fn, const T& x)
{
    typedef typename Operation::second_argument_type Arg2_type;
    return binder2nd<Operation>(fn, Arg2_type(x));
}

// 将普通函数适配为函数对象
template <typename Arg, typename Result>
class pointer_to_unary_function : public unary_function<Arg, Result>
{
protected:
    Result (*_M_ptr)(Arg);
public:
    explicit pointer_to_unary_function(Result (*x)(Arg)) : _M_ptr(x) {}
    Result operator()(Arg x) const
    {
        return _M_ptr(x);
    }
};

template <typename Arg, typename Result>
inline pointer_to_unary_function<Arg, Result> ptr_fun(Result (*x)(Arg))
{
    return pointer_to_unary_function<Arg, Result>(x);
}

template <typename Arg1, typename Arg2, typename Result>
class pointer_to_binary_function :
    public binary_function<Arg1,Arg2,Result>
{
protected:
    Result (*_M_ptr)(Arg1, Arg2);
public:
    pointer_to_binary_function() {}
    explicit pointer_to_binary_function(Result (*x)(Arg1, Arg2))
        : _M_ptr(x) {}
    Result operator()(Arg1 x, Arg2 y) const
    {
        return _M_ptr(x, y);
    }
};

template <typename Arg1, typename Arg2, typename Result>
inline pointer_to_binary_function<Arg1,Arg2,Result>
ptr_fun(Result (*x)(Arg1, Arg2))
{
    return pointer_to_binary_function<Arg1,Arg2,Result>(x);
}

/**************** 成员函数的适配器 *********************/

// Adaptor function objects: pointers to member functions.
// 这个族群一共有8 = 2^3 个函数对象
//  (1) 无参数 vs 有一个参数
//  (2) 通过指针调用 vs 通过引用调用
//  (3) const成员函数 vs non-const成员函数

// 所有的复杂性都存在于函数对象内部, 你可以忽略他们,
// 直接使用辅助函数mem_fun和mem_fun_ref, 它们会产生适当的适配器

// 无参, 通过指针调用, non-const成员函数
template <typename Ret, typename T>
class mem_fun_t : public unary_function<T*,Ret>
{
public:
    explicit mem_fun_t(Ret (T::*pf)()) : _M_f(pf) {}
    Ret operator()(T* p) const
    {
        return (p->*_M_f)();
    }
private:
    Ret (T::*_M_f)();
};

// 无参, 通过指针调用, const成员函数
template <typename Ret, typename T>
class const_mem_fun_t : public unary_function<const T*,Ret>
{
public:
    explicit const_mem_fun_t(Ret (T::*pf)() const) : _M_f(pf) {}
    Ret operator()(const T* p) const
    {
        return (p->*_M_f)();
    }
private:
    Ret (T::*_M_f)() const;
};

// 无参, 通过引用调用, non-const成员函数
template <typename Ret, typename T>
class mem_fun_ref_t : public unary_function<T,Ret>
{
public:
    explicit mem_fun_ref_t(Ret (T::*pf)()) : _M_f(pf) {}
    Ret operator()(T& r) const
    {
        return (r.*_M_f)();
    }
private:
    Ret (T::*_M_f)();
};

// 无参, 通过引用调用, const成员函数
template <typename Ret, typename T>
class const_mem_fun_ref_t : public unary_function<T,Ret>
{
public:
    explicit const_mem_fun_ref_t(Ret (T::*pf)() const) : _M_f(pf) {}
    Ret operator()(const T& r) const
    {
        return (r.*_M_f)();
    }
private:
    Ret (T::*_M_f)() const;
};

// 有一个参数, 通过指针调用, non-const成员函数
template <typename Ret, typename T, typename Arg>
class mem_fun1_t : public binary_function<T*,Arg,Ret>
{
public:
    explicit mem_fun1_t(Ret (T::*pf)(Arg)) : _M_f(pf) {}
    Ret operator()(T* p, Arg x) const
    {
        return (p->*_M_f)(x);
    }
private:
    Ret (T::*_M_f)(Arg);
};

// 有一个参数, 通过指针调用, const成员函数
template <typename Ret, typename T, typename Arg>
class const_mem_fun1_t : public binary_function<const T*,Arg,Ret>
{
public:
    explicit const_mem_fun1_t(Ret (T::*pf)(Arg) const) : _M_f(pf) {}
    Ret operator()(const T* p, Arg x) const
    {
        return (p->*_M_f)(x);
    }
private:
    Ret (T::*_M_f)(Arg) const;
};

// 有一个参数, 通过引用调用, non-const成员函数
template <typename Ret, typename T, typename Arg>
class mem_fun1_ref_t : public binary_function<T,Arg,Ret>
{
public:
    explicit mem_fun1_ref_t(Ret (T::*pf)(Arg)) : _M_f(pf) {}
    Ret operator()(T& r, Arg x) const
    {
        return (r.*_M_f)(x);
    }
private:
    Ret (T::*_M_f)(Arg);
};

// 有一个参数, 通过引用调用, const成员函数
template <typename Ret, typename T, typename Arg>
class const_mem_fun1_ref_t : public binary_function<T,Arg,Ret>
{
public:
    explicit const_mem_fun1_ref_t(Ret (T::*pf)(Arg) const) : _M_f(pf) {}
    Ret operator()(const T& r, Arg x) const
    {
        return (r.*_M_f)(x);
    }
private:
    Ret (T::*_M_f)(Arg) const;
};

// 成员函数适配器的辅助函数: mem_fun, mem_fun_ref

template <typename Ret, typename T>
inline mem_fun_t<Ret,T> mem_fun(Ret (T::*f)())
{
    return mem_fun_t<Ret,T>(f);
}

template <typename Ret, typename T>
inline const_mem_fun_t<Ret,T> mem_fun(Ret (T::*f)() const)
{
    return const_mem_fun_t<Ret,T>(f);
}

template <typename Ret, typename T>
inline mem_fun_ref_t<Ret,T> mem_fun_ref(Ret (T::*f)())
{
    return mem_fun_ref_t<Ret,T>(f);
}

template <typename Ret, typename T>
inline const_mem_fun_ref_t<Ret,T> mem_fun_ref(Ret (T::*f)() const)
{
    return const_mem_fun_ref_t<Ret,T>(f);
}

template <typename Ret, typename T, typename Arg>
inline mem_fun1_t<Ret,T,Arg> mem_fun(Ret (T::*f)(Arg))
{
    return mem_fun1_t<Ret,T,Arg>(f);
}

template <typename Ret, typename T, typename Arg>
inline const_mem_fun1_t<Ret,T,Arg> mem_fun(Ret (T::*f)(Arg) const)
{
    return const_mem_fun1_t<Ret,T,Arg>(f);
}

template <typename Ret, typename T, typename Arg>
inline mem_fun1_ref_t<Ret,T,Arg> mem_fun_ref(Ret (T::*f)(Arg))
{
    return mem_fun1_ref_t<Ret,T,Arg>(f);
}

template <typename Ret, typename T, typename Arg>
inline const_mem_fun1_ref_t<Ret,T,Arg>
mem_fun_ref(Ret (T::*f)(Arg) const)
{
    return const_mem_fun1_ref_t<Ret,T,Arg>(f);
}
```