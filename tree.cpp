//将字符串类型的四则运算表达式存入二叉树中，支持小数运算和多级括号运算，同时能够将二叉树结构输出
//目前一大缺陷是不能检查输入是否有误，另一缺陷是用'!'作为负号

/*
测试样例:1+2  1*9/3  764+743/90*45+56-87   8*(9+2*(3+(4*(5+6*(7+8)))))    [(7.1-5.6)*0.9-1.15]/2.5       [(7-5)*0.9-1]/2   [(7-5)*9-1]/2
        {[4+3*(5+6)]*[2+8*(1+2)]+5}/4    {[4+3*(5+6)]*[2+8*(1+2)]}/4 
        a+g  a+g/c
*/

#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>

double strToDouble(const std::string& str)
{
    double num = 0.0;
    int dot = 0;
	int start = 0;
	int end = str.size() -1;
    for(int i = start; i <= end; ++i)
    {
        if(str[i] == '.')
            dot = i;
        else
            num = num * 10 + str[i] - '0';
    }
    if(dot != 0)
        return num / pow(10, end - start - dot);
    return num;
}

//表达式基类
struct Expression
{
    Expression(Expression* theLeftChild = NULL, Expression* theRightChild = NULL)
		: leftChild(theLeftChild), rightChild(theRightChild) { }
    Expression *leftChild, *rightChild;
	bool is_leaf() { return !leftChild && !rightChild; }
};

//常数类
struct Constant: public Expression
{
    Constant(const std::string &theValue) : value(theValue) { }
	std::string value; //既能存数字又能存变量
};

//运算符类
//对于一元运算符'!', 其左节点为空
struct Operation: public Expression
{
    Operation(Expression* theLeftChild, char theType, Expression* theRightChild)
		: Expression(theLeftChild, theRightChild), type(theType) { }
    char type;
};

//中序遍历
void traverseTree(const Expression* node, std::ostream& out, int depth = 0)
{
	if(!node) return;
	++depth;
	traverseTree(node->rightChild, out, depth);
	int tmp = depth;
	while(--tmp) out << "  ";
	if(node->leftChild || node->rightChild)
		out << ((Operation*)node)->type << std::endl;
	else 
		out << ((Constant*)node)->value << std::endl;
	traverseTree(node->leftChild, out, depth);
}

double getResult(Expression* node)
{
	//使用后序遍历的思想
	double leftResult = 0, rightResult = 0;
	if(node->leftChild) leftResult = getResult(node->leftChild);
	if(node->rightChild) rightResult = getResult(node->rightChild);
	if(node->is_leaf()) return strToDouble(((Constant*)node)->value);
	//取出运算符
	switch(((Operation*)node)->type)
	{
		case '+':
			return leftResult + rightResult; 
		case '-':
			return leftResult - rightResult;
		case '*':
			return leftResult * rightResult;
		case '/':
			return leftResult / rightResult;
		case '!':
			return 0 - rightResult;
		default:
			break;
	}
	return 0;
}

//检查是否需要删除两端括号
bool deleteBracket(const std::string& expr, int start, int end)
{
    int brackets = 0; //用来记录括号数，碰到'('加一，碰到')'减一，当brackets为0时表示当前读取的字符在括号外
	for(size_t i = start; i <= end; ++i)
	{
		//当前字符不是数字
		if(!(isalnum(expr[i]) || expr[i] == '.'))
		{
			switch(expr[i])
			{
				case '{': case '[': case '(': ++brackets; break;
				case '}': case ']': case ')': --brackets; break;
				default:
					//存在不被括号包裹的运算符
					if(brackets == 0) return false;
					break;
			}
		}
	}
	return true;
}

Expression* strToTree(std::string& expr, int start, int end)
{
    //去除表达式最外层括号, 为了不影响"(1+2)*(3+4)"这种表达式, 要检查左右括号是否可以去除
    while(start <= end && ((expr[start] == '(' && expr[end] == ')') || (expr[start] == '[' && expr[end] == ']') || (expr[start] == '{' && expr[end] == '}')))
    {
        if(deleteBracket(expr, start, end))
            ++start, --end;
        else
			break;
    }
    if(start > end) return new Constant("0");

    int brackets = 0;
	int firstNot = -1;
    int lastPS = -1, lastMD = -1; //最后出现的加减号和乘除号的位置
    for(int i = start; i <= end; ++i)
    {
        //当前字符不是数字
		if(!(isalnum(expr[i]) || expr[i] == '.'))
		{
			switch(expr[i])
			{
				case '{': case '[': case '(': ++brackets; break;
				case '}': case ']': case ')': --brackets; break;
				case '!':
					if(brackets == 0 && firstNot == -1) firstNot = i;
					break;
				case '*':
				case '/':
					if(brackets == 0) lastMD = i;
					break;
				case '+':
				case '-':
				default: //其他字符当做与'+'同优先级的运算符
					if(brackets == 0) lastPS = i;
					break;
			}
		}
    }
    if(lastPS != -1)
		return new Operation(strToTree(expr, start, lastPS - 1), expr[lastPS], strToTree(expr, lastPS + 1, end));
	if(lastMD != -1)
		return new Operation(strToTree(expr, start, lastMD - 1), expr[lastMD], strToTree(expr, lastMD + 1, end));
	if(firstNot != -1)
		return new Operation(NULL, expr[firstNot], strToTree(expr, firstNot + 1, end));
    //表达式是全是数字
	return new Constant(expr.substr(start, end - start + 1));
}

int main()
{
	std::cout << "请输入正确格式的四则运算表达式，注意在英文输入法下输入括号：" << std::endl;
	std::string expression;
    getline(std::cin, expression);
	expression.erase(std::remove(expression.begin(), expression.end(), ' '), expression.end());

    Expression* root = strToTree(expression, 0, expression.length()-1);
    traverseTree(root, std::cout);
	std::cout << expression << '=' << getResult(root) << std::endl;

	//TODO
	//delete all nodes

    return 0;
}
