//���ַ������͵�����������ʽ����������У�֧��С������Ͷ༶�������㣬ͬʱ�ܹ����������ṹ�������Ļ����ĳ��txt�ļ�(һ�ú��ŵĶ�����)��Ŀǰһ��ȱ���ǲ��ܼ�������Ƿ�����,

/*
��������:1+2  1*9/3  764+743/90*45+56-87   8*(9+2*(3+(4*(5+6*(7+8)))))    [(7.1-5.6)*0.9-1.15]/2.5       [(7-5)*0.9-1]/2   [(7-5)*9-1]/2
        {[4+3*(5+6)]*[2+8*(1+2)]+5}/4    {[4+3*(5+6)]*[2+8*(1+2)]}/4 
        a+g  a+g/c
*/

#include<iostream>//�̳���ostream
#include<fstream>
#include<string>
using namespace std;

//���ַ���ת����double��������,֧��С��
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

//���ʽ����
class Expression
{
public:
    Expression() { leftChild = rightChild = nullptr; }
    Expression(Expression* theLeftChild, Expression* theRightChild) {
        leftChild = theLeftChild; 
        rightChild = theRightChild; 
    }
public:
    Expression* leftChild, *rightChild;//������ڵ�϶��������ڵ�:����Ϊ�˷�����ڻ�����
};
//������
class Constant :public Expression
{
    friend class Operation;
    friend double getResult(Expression*);//���ض���������ṹ
public:
    Constant(string value) { this->value = value; }//O(1)
private:
    string value;//���ܴ��������ܴ����
};
//�������
class Operation :public Expression
{
    friend void output(Operation,ostream&);//������Ľṹ
    friend double getResult(Expression*);//���ض���������ṹ
public:
    Operation(Expression* theLeftChild, char theType, Expression* theRightChild)//O(1)
    {
        leftChild = theLeftChild;
        rightChild = theRightChild;
        type = theType;
        depth = 1;
    }
    void inOrder(Expression* ,int ,ostream& out)const;//�������O(n)
private:
    char type;//+ - * / ����������е�һ��
    int depth;//�ݹ����
};
//ע��������������,��cout����ʹ��ostream,��������iostream����Ϊiostream��ostream����
//���ʹ��cout�����ض����ܹ�ѡ��д�����
void output(Operation o,ostream& out=cout)//O(1)
{
    //���ʱ�õ����������
    o.inOrder(&o, o.depth,out);
}
void deleteSpace(string& s)//ȥ���ַ����еĿո�  O(n)
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
    //ʹ�ú��������˼��
    double leftResult = 0,rightResult=0;
    //�������������Ϊ�շ�����������������������˵������ڵ���Ҷ�ӣ�����Ҷ�ӵ�ֵ����
    if (o->leftChild)leftResult = getResult(o->leftChild);
    else return strToDouble(((Constant*)o)->value,0, (((Constant*)o)->value).length()-1);
    if (o->rightChild)rightResult = getResult(o->rightChild);
    else return strToDouble(((Constant*)o)->value, 0, (((Constant*)o)->value).length() - 1);
    //ȡ�������
    switch (((Operation*)o)->type)
    {
    case '+':return leftResult + rightResult; 
    case '-':return leftResult - rightResult;
    case '*':return leftResult*rightResult;
    case '/':return leftResult / rightResult;
    }
    return 0;
}
//������෽��ʵ��
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
//����Ƿ���Ҫɾ����������
bool deleteBracket(string& expression,int& start,int& end) //O(n)
{
    int lastPS = -1, lastMD = -1;
    int brackets = 0;//������¼������������(��һ������)��һ����bracketsΪ0ʱ��ʾ��ǰ��ȡ���ַ���������
        for (size_t i = start; i <= end; ++i)
        {
            //��ǰ�ַ���������
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

//��ؼ��ĺ���:���ַ���ת������
Expression* strToTree(string& expression, int start, int end)//startΪ�ַ�����Ҫת�����ֵ���ʼλ�ã�end�ǽ���λ��O(n^2)
{
    //������ʽ���������ţ�����ʼλ�úͽ���λ�÷ֱ����м�û�����ŵ�λ�ÿ�
    //Ϊ�˲�Ӱ�� (1+2)*(3+4) ���ֱ��ʽ�ļ��㣬Ҫ������������Ƿ����ȥ��:�ȱ��������ַ������ҳ���ʽ���Ƿ����lastPS(���һ�γ��ֵĲ��������еļӺŻ����)��lastMD(���һ�γ��ֵĲ��������еĳ˺Ż����)
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
    bool findOperation = false;//��¼�Ƿ��в��ҵ������������û�б�ʾ����ַ���ȫ������
    //���������ֵļӼ��źͳ˳���
    for (int i = start; i <= end;++i)
    {
        //��ǰ�ַ���������
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
    //�ж�������ʽ�ǲ���ȫ������
    if (!findOperation)
    {
        string str = "";
        return new Constant(str.assign(expression, start, end - start + 1));
    }

    //���������ʽ��û�мӼ��ž��ó˳���
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
    cout << "��������ȷ��ʽ������������ʽ��ע����Ӣ�����뷨����������" << endl;
    string expression;
    getline(cin,expression);//cinֻ�ܶ�ȡ
    deleteSpace(expression);//ȥ���ո�
    Expression* fun = strToTree(expression,0,expression.length()-1);

    output(*(Operation*)fun,fout);//ֻ����һ���������ڿ���̨������Ľṹ 
    output(*(Operation*)fun);
    cout<<getResult(fun)<<endl;

    system("pause");
    return 0;
}