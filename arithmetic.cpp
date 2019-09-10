//四则运算, 支持小括号
//表达式合法方可正常计算
#include <iostream>
#include <string>
#include <stack>

namespace {
bool is_number_char(char c)
{
	return std::isdigit(c) || c == '-';
}
size_t get_number_length(const std::string &expr, size_t currentPos)
{
	const std::string numbers = "0123456789.";
	size_t beginPos = (expr[currentPos] == '-') ? currentPos+1 : currentPos;
	return expr.find_first_not_of(numbers,beginPos) - currentPos;
} 
int get_priority(char c)
{
	if (c == '(') return 0;
	if (c == '+') return 1;
	if (c == '-') return 2;
	if (c == '*') return 3;
	if (c == '/') return 4;
	//其他运算符优先级等同于+, 计算结果恒为0
	return 1;
}
void pop_caculate(std::stack<double> &num_stk, std::stack<char> &oper_stk)
{
	double right = num_stk.top();
	num_stk.pop();
	double left = num_stk.top();
	num_stk.pop();
	char oper = oper_stk.top();
	oper_stk.pop();
	double res = 0;
	if (oper == '+') res = left + right;
	else if (oper == '-') res = left - right;
	else if (oper == '*') res = left * right;
	else if (oper == '/') res = left / right;
	num_stk.push(res);
	std::cout << "caculate: " << left << oper << right << "=" << res << std::endl;
}
}

double caculate(const std::string &expr)
{
	std::stack<double> num_stk;
	std::stack<char> oper_stk;
	bool number_flag = false;
	for(size_t i=0,s=expr.size(); i<s; )
	{
		if(number_flag == false && is_number_char(expr[i]))
		{
			number_flag = true;
			num_stk.push(strtod(&expr[i], nullptr));
			std::cout << "resolve num " << strtod(&expr[i], nullptr) << std::endl;
			i += get_number_length(expr, i);
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
			++i;
		}
	}
	while(!oper_stk.empty()) {
		pop_caculate(num_stk, oper_stk);
	}
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
