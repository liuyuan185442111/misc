//逻辑运算, 支持小括号, 1为true, 0为false, +为与, *为或
//表达式合法方可正常计算, 没有处理碰到')'异常的情况
#include <iostream>
#include <string>
#include <stack>

namespace {
bool is_number_char(char c)
{
	return c == '0' || c == '1';
}
int get_priority(char c)
{
	if (c == '(') return 0;
	if (c == '+') return 1;
	if (c == '*') return 3;
	return 1;
}
bool pop_caculate(std::stack<int> &num_stk, std::stack<char> &oper_stk)
{
	//防御性措施
	if(num_stk.size() < 2 || oper_stk.empty()) return false;
	char oper = oper_stk.top();
	oper_stk.pop();
	int right = num_stk.top();
	num_stk.pop();
	int left = num_stk.top();
	num_stk.pop();
	int res = 0;
	if (oper == '+') res = (left + right) > 0 ? 1 : 0;
	else if (oper == '*') res = left * right;
	num_stk.push(res);
	std::cout << "caculate: " << left << oper << right << "=" << res << std::endl;
	return true;
}
}

//using namespace std;
int caculate(const std::string &expr)
{
	std::stack<int> num_stk;
	std::stack<char> oper_stk;
	bool number_flag = false;
	for(size_t i=0,s=expr.size(); i<s; ++i)
	{
		if(number_flag == false && is_number_char(expr[i]))
		{
			number_flag = true;
			num_stk.push(expr[i] - '0');
			std::cout << "resolve num " << expr[i] - '0' << std::endl;
		}
		else
		{
			number_flag = false;
			if(oper_stk.empty()) {
				oper_stk.push(expr[i]);
			} else if(expr[i] == '(') {
				oper_stk.push(expr[i]);
			} else if(expr[i] == ')') {
				while(oper_stk.top() != '(') {
					pop_caculate(num_stk, oper_stk);
				}
				oper_stk.pop();
			} else if(get_priority(expr[i]) <= get_priority(oper_stk.top())) {
				pop_caculate(num_stk, oper_stk);
				oper_stk.push(expr[i]);
			} else {
				oper_stk.push(expr[i]);
			}
		}
	}
	while(!oper_stk.empty()) {
		if(!pop_caculate(num_stk, oper_stk)) break;
	}
	//防御性措施
	if(num_stk.empty()) return 0;
	return num_stk.top();
}

int main()
{
	std::string experssion;
	std::cout << "input experssion:" << std::endl;
	while(std::cin >> experssion) {
		std::cout << experssion << '=' << caculate(experssion) << std::endl << std::endl;
	}
	return 0;
}
