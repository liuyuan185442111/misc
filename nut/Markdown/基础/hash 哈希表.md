我觉得hash提高查询速度的根本原因是分组。
比如图书馆的书，如果是无序的，要找一本书，只能从头到尾依次找；如果将这些书分门别类，现在找一本书，直接去相应类别里，就快多了。从书名到其类别的转换就是一个映射过程，这是hash函数的作用。

用c实现的一个简单的例子，对key按余数分组，数组存储本组的头指针，余数相同的key依次链到头指针后面。
```
#define TABLE_SIZE 17

typedef struct _NODE
{
    int data;
    struct _NODE *next;
} NODE;

typedef struct _HASH_TABLE
{
    NODE *value[TABLE_SIZE];
} HASH_TABLE;

int hash_fun(int data)
{
    return data % TABLE_SIZE;
}

HASH_TABLE *create_hash_table()
{
    HASH_TABLE *pHashTbl = (HASH_TABLE*)malloc(sizeof(HASH_TABLE));
    memset(pHashTbl, 0, sizeof(HASH_TABLE));
    return pHashTbl;
}

void delete_hash_table(HASH_TABLE *pHashTbl)
{
    NODE *head, *pre;
    for(int i=0; i<TABLE_SIZE; ++i)
    {
        head = pHashTbl->value[i];
        if(head == NULL) continue;
        while(head)
        {
            pre = head;
            head = head->next;
            free(pre);
        }
    }
}

NODE *find_data_in_hash(HASH_TABLE *pHashTbl, int data)
{
    if(pHashTbl == NULL)
        return NULL;

    NODE *pNode = pHashTbl->value[hash_fun(data)];
    if(pNode == NULL)
        return NULL;

    while(pNode)
    {
        if(data == pNode->data)
            return pNode;
        pNode = pNode->next;
    }

    return NULL;
}

bool insert_data_into_hash(HASH_TABLE *pHashTbl, int data)
{
    if(pHashTbl == NULL)
        return false;

    if(pHashTbl->value[hash_fun(data)] == NULL)
    {
        NODE *pNode = (NODE*)malloc(sizeof(NODE));
        pNode->data = data;
        pNode->next = NULL;
        pHashTbl->value[hash_fun(data)] = pNode;
        return true;
    }

    if(find_data_in_hash(pHashTbl, data) != NULL)
        return false;

    NODE *pNode = pHashTbl->value[hash_fun(data)];
    while(pNode->next)
        pNode = pNode->next;

    pNode->next = (NODE*)malloc(sizeof(NODE));
    pNode->next->data = data;
    pNode->next->next = NULL;

    return true;
}

bool delete_data_from_hash(HASH_TABLE *pHashTbl, int data)
{
    if(NULL == pHashTbl || NULL == pHashTbl->value[hash_fun(data)])
        return false;

    NODE *pNode = find_data_in_hash(pHashTbl, data);
    if(NULL == pNode) return false;

    NODE *pHead = pHashTbl->value[hash_fun(data)];
    if(pNode == pHead)
    {
        pHashTbl->value[hash_fun(data)] = pNode->next;
    }
    else
    {
        while(pNode != pHead->next)
            pHead = pHead->next;
        pHead->next = pNode->next;
    }
    free(pNode);
    return true;
}
```
MPQ是Blizzard公司将游戏数据包装在一起的一种档案格式，里面有将文件名映射为整数的Hash算法：
```
unsigned long cryptTable[0x500];
void prepareCryptTable()
{
    unsigned long seed = 0x00100001, index1 = 0, index2 = 0, i;
    for( index1 = 0; index1 < 0x100; index1++ )
    {
        for( index2 = index1, i = 0; i < 5; i++, index2 += 0x100 )
        {
            unsigned long temp1, temp2;
            seed = (seed * 125 + 3) % 0x2AAAAB;
            temp1 = (seed & 0xFFFF) << 0x10;
            seed = (seed * 125 + 3) % 0x2AAAAB;
            temp2 = (seed & 0xFFFF);
            cryptTable[index2] = ( temp1 | temp2 );
       }
   }
}
//dwHashType为0,1或2，1和2用于校验
unsigned long HashString(char *lpszFileName, unsigned long dwHashType)
{
    unsigned char *key  = (unsigned char *)lpszFileName;
    unsigned long seed1 = 0x7FED7FED;
    unsigned long seed2 = 0xEEEEEEEE;
    int ch;
    while(*key != 0)
    {
        ch = toupper(*key++);
        seed1 = cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
        seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
    }
    return seed1;
}
```
用HashString构造哈希表时，如果两个文件名对应的位置相同，可以将它们链成一个链表，查找时
当文件名对应的位置上有数据时，还要对比它们的文件名是不是相同。但Blizzard并没有在哈希表中存储原文件名，而是另外存储两个哈希值来校验字符串。Blizzard使用的哈希表没有使用链表，而采用“顺延”的方式来解决问题。

其他比较简单一些的hash函数：
```
/*key为一个字符串，nTableLength为哈希表的长度，该函数得到的hash值分布比较均匀*/
unsigned long getHashIndex(const char *key, int nTableLength)
{
    unsigned long nHash = 0;
    while(*key)
    {
        nHash = (nHash<<5) + nHash + *key++;
    }
    return (nHash % nTableLength);
}

//PHP中出现的字符串Hash函数
static unsigned long hashpjw(const char *arKey, unsigned int nKeyLength)
{
    unsigned long h = 0, g;
    const char *arEnd = arKey + nKeyLength;

    while(arKey < arEnd)
    {
        h = (h << 4) + *arKey++;
        if((g = (h & 0xF0000000)))
        {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    return h;
}
```
参考
从头到尾彻底解析Hash表算法
http://kb.cnblogs.com/page/189480/
一步一步写算法（之hash表）
http://blog.csdn.net/feixiaoxing/article/details/6885657
打造最快的Hash表
http://blog.chinaunix.net/uid-20558494-id-2803048.html