#include <iostream>
#include <string>
#include <stack>
using namespace std;

stack<double> num_stk;
stack<char> oper_stk;
string experssion;
double result;
int number_flag;

void pop_caculate();
int cal_priority(char c);
string::size_type get_number_size(string &expr,string::size_type currentPos);
bool is_number(char c);
void caculate(string expr);

int main()
{
    cout << "input experssion!" << endl;
    while (cin >> experssion) {
        number_flag = 0;
        caculate(experssion);
    }
}

/*
 * get number length from currentPos of string expr
*/
string::size_type get_number_size(string &expr,string::size_type currentPos)
{
    string numbers = "0123456789.";
    return expr.find_first_not_of(numbers,currentPos) - currentPos;
} 

/*
 * char c is number?
*/
bool is_number(char c)
{
    string numbers = "0123456789-";
    if (numbers.find(c) == string::npos) {
        return false;
    }
    return true;

}
/*
 * the main caculate method
*/
void caculate(string expr)
{
    string::size_type size = 0;
    double num = 0;
    for (string::size_type i=0;i<expr.size();) {
        if (number_flag == 0 && is_number(expr[i])) {
            num = stod(expr.substr(i));
            if (expr[i] == '-') {
            ++i;
            num = -stod(expr.substr(i));
            }
            size = get_number_size(expr,i);
        num_stk.push(num);
        i += size;
        cout << "num:" << num << endl;
            number_flag = 1;
        } else {
        number_flag = 0;
        if (oper_stk.empty()) {
                oper_stk.push(expr[i]);
        } else if (expr[i] == '(') {
            oper_stk.push(expr[i]);
        } else if (expr[i] == ')') {
                while (oper_stk.top() != '(') {
                    pop_caculate();
            }
            oper_stk.pop();
        } else if (cal_priority(expr[i]) <= cal_priority(oper_stk.top())) {
                pop_caculate();
            oper_stk.push(expr[i]);
        } else {
                oper_stk.push(expr[i]);
        }
        ++i;
        }
    }
    while (!oper_stk.empty()) {
        pop_caculate();
    }
    result = num_stk.top();
    cout << result << endl;
}
/*
 * pop and caculate the two stack to caculate
*/
void pop_caculate()
{
    double left=0,right=0,res=0;
    char oper = oper_stk.top();
    right = num_stk.top();
    num_stk.pop();
    left = num_stk.top();
    num_stk.pop();
    oper_stk.pop();
    if (oper == '+') res = left + right;
    else if (oper == '-') res = left - right;
    else if (oper == '*') res = left * right;
    else if (oper == '/') res = left / right;
    num_stk.push(res);
    cout << "caculate:" << left << oper << right << "=" << res << endl;
}
/* 
 *  caculate operator priority 
*/
int cal_priority(char c)
{
    if (c == '(') return 0;
    else if (c == '+') return 1;
    else if (c == '-') return 2;
    else if (c == '*') return 3;
    else if (c == '/') return 4;
    return 0;
}