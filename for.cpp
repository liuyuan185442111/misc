//将字符串类型的四则运算表达式存入二叉树中，支持小数运算和多级括号运算，同时能够将二叉树结构输出到屏幕或者某个txt文件(一棵横着的二叉树)，目前一大缺陷是不能检查输入是否有误,

/*
测试样例:1+2  1*9/3  764+743/90*45+56-87   8*(9+2*(3+(4*(5+6*(7+8)))))    [(7.1-5.6)*0.9-1.15]/2.5       [(7-5)*0.9-1]/2   [(7-5)*9-1]/2
        {[4+3*(5+6)]*[2+8*(1+2)]+5}/4    {[4+3*(5+6)]*[2+8*(1+2)]}/4 
        a+g  a+g/c
*/

#include<iostream>//继承自ostream
#include<fstream>
#include<string>
using namespace std;

//将字符串转换成double类型数字,支持小数
double strToDouble(string& s,int start,int end)
{
    double num = 0.0;
    int dot = 0;
    for (int i = start; i <= end; ++i)
    {
        if (s[i] == '.')
            dot = i;
        else
            num = num * 10 + s[i] - '0';
    }
    if (dot != 0)
        return num / pow(10, end - start - dot);
    return num;
}

//表达式基类
class Expression
{
public:
    Expression() { leftChild = rightChild = nullptr; }
    Expression(Expression* theLeftChild, Expression* theRightChild) {
        leftChild = theLeftChild; 
        rightChild = theRightChild; 
    }
public:
    Expression* leftChild, *rightChild;//运算符节点肯定有两个节点:但是为了方便放在基类中
};
//常数类
class Constant :public Expression
{
    friend class Operation;
    friend double getResult(Expression*);//返回二叉树运算结构
public:
    Constant(string value) { this->value = value; }//O(1)
private:
    string value;//既能存数字又能存变量
};
//运算符类
class Operation :public Expression
{
    friend void output(Operation,ostream&);//输出树的结构
    friend double getResult(Expression*);//返回二叉树运算结构
public:
    Operation(Expression* theLeftChild, char theType, Expression* theRightChild)//O(1)
    {
        leftChild = theLeftChild;
        rightChild = theRightChild;
        type = theType;
        depth = 1;
    }
    void inOrder(Expression* ,int ,ostream& out)const;//中序遍历O(n)
private:
    char type;//+ - * / 四种运算符中的一种
    int depth;//递归深度
};
//注：这儿必须加引用,且cout必须使用ostream,而不能是iostream，因为iostream是ostream子类
//这儿使用cout方便重定向，能够选择写入对象
void output(Operation o,ostream& out=cout)//O(1)
{
    //输出时用到了中序遍历
    o.inOrder(&o, o.depth,out);
}
void deleteSpace(string& s)//去除字符串中的空格  O(n)
{
    string expression = "";
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] != ' ')
            expression += s[i];
    }
    s = expression;
    //cout << s << endl;
}
double getResult(Expression* o)//O(n)
{
    //使用后序遍历的思想
    double leftResult = 0,rightResult=0;
    //如果是左子树不为空返回左子树的运算结果，否则说明这个节点是叶子，返回叶子的值即可
    if (o->leftChild)leftResult = getResult(o->leftChild);
    else return strToDouble(((Constant*)o)->value,0, (((Constant*)o)->value).length()-1);
    if (o->rightChild)rightResult = getResult(o->rightChild);
    else return strToDouble(((Constant*)o)->value, 0, (((Constant*)o)->value).length() - 1);
    //取出运算符
    switch (((Operation*)o)->type)
    {
    case '+':return leftResult + rightResult; 
    case '-':return leftResult - rightResult;
    case '*':return leftResult*rightResult;
    case '/':return leftResult / rightResult;
    }
    return 0;
}
//运算符类方法实现
void Operation::inOrder(Expression* t,int d,ostream& out)const//O(n)
{
    ++d;
    if (t)
    {
        inOrder(t->rightChild,d,out);
        int dd = d;
        while (dd--)out << "  ";
        if(t->leftChild)
            out << ((Operation*)t)->type<<endl;
        else 
            out << ((Constant*)t)->value << endl;
        inOrder(t->leftChild,d,out);
    }
}
//检查是否需要删除两端括号
bool deleteBracket(string& expression,int& start,int& end) //O(n)
{
    int lastPS = -1, lastMD = -1;
    int brackets = 0;//用来记录括号数，碰到(加一，碰到)减一，当brackets为0时表示当前读取的字符在括号外
        for (size_t i = start; i <= end; ++i)
        {
            //当前字符不是数字
            if (!(expression[i] >= '0'&&expression[i] <= '9') && !(expression[i] >= 'a'&&expression[i] <= 'z') && !(expression[i] >= 'A'&&expression[i] <= 'Z') && !(expression[i] == '.'))
            {
                switch (expression[i])
                {
                case '{':
                case '[':
                case '(':++brackets; break;
                case '}':
                case ']':
                case ')':--brackets; break;
                case '+':
                case '-':if (!brackets)lastPS = i; break;
                case '*':
                case '/':if (!brackets)lastMD = i; break;
                }
            }
        }
        if (lastPS == -1)lastPS = lastMD;
        if (lastPS == -1)
            return true;
        return false;
}

//最关键的函数:将字符串转化成树
Expression* strToTree(string& expression, int start, int end)//start为字符串中要转化部分的起始位置，end是结束位置O(n^2)
{
    //如果表达式最外层带括号，将起始位置和结束位置分别向中间没有括号的位置靠
    //为了不影响 (1+2)*(3+4) 这种表达式的计算，要检查左右括号是否可以去除:先遍历整个字符串，找出算式中是否存在lastPS(最后一次出现的不在括号中的加号或减号)和lastMD(最后一次出现的不在括号中的乘号或除号)
    while (start <= end && (expression[start] == '(' && expression[end] == ')' || (expression[start] == '[' && expression[end] == ']') || (expression[start] == '{' && expression[end] == '}')))
    {
        if (deleteBracket(expression, start, end))
            ++start, --end;
        else break;
    }

    if (start > end)return new Constant("0");

    int brackets = 0;
    int lastPS = -1, lastMD = -1;
    if (start > end)
        return new Constant("0");
    bool findOperation = false;//记录是否有查找到操作符，如果没有表示这个字符串全是数字
    //查找最后出现的加减号和乘除号
    for (int i = start; i <= end;++i)
    {
        //当前字符不是数字
        if (!(expression[i] >= '0'&&expression[i] <= '9') && !(expression[i]>='a'&&expression[i]<='z') && !(expression[i]>='A'&&expression[i]<='Z') &&!(expression[i]=='.'))
        {
            findOperation = true;
            switch (expression[i])
            {
            case '{':
            case '[':
            case '(':++brackets; break;
            case '}':
            case ']':
            case ')':--brackets; break;
            case '+':
            case '-':if (!brackets)lastPS = i; break;
            case '*':
            case '/':if (!brackets)lastMD = i; break;
            }
        }
    }
    //判断这个表达式是不是全是数字
    if (!findOperation)
    {
        string str = "";
        return new Constant(str.assign(expression, start, end - start + 1));
    }

    //如果这个表达式中没有加减号就用乘除号
    if (lastPS == -1)
        lastPS = lastMD;
    return new Operation(strToTree(expression, start, lastPS - 1), expression[lastPS], strToTree(expression, lastPS + 1, end));
}

int main()
{
    fstream fout;
    fout.open("input.txt");
    if (!fout.is_open())
    {
        cerr << "wrong" << endl;
        system("pause");
        exit(EXIT_FAILURE);
    }
    cout << "请输入正确格式的四则运算表达式，注意在英文输入法下输入括号" << endl;
    string expression;
    getline(cin,expression);//cin只能读取
    deleteSpace(expression);//去除空格
    Expression* fun = strToTree(expression,0,expression.length()-1);

    output(*(Operation*)fun,fout);//只传入一个参数会在控制台输出树的结构 
    output(*(Operation*)fun);
    cout<<getResult(fun)<<endl;

    system("pause");
    return 0;
}