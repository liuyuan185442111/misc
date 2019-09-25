#include <algorithm>
#include <numeric>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <vector>
#include <iterator>
using namespace std;

#include "dota_skill_house.h"

#define UInt64 unsigned long long

struct dota_single_skill
{
    int skill0[40];
    int skill1[4];
    int skill2[10];
    int skill3[20];
    int skill4[8];
};

//检查技能是不是在配置表里
inline bool check_skill(int id, short skill_id, int type)
{
    return true;
    assert(type >= 0 && type < 5);
    dota_single_skill dss; //需要根据id获得,从data获取
    int *skill = dss.skill0;
    int count = sizeof(dss.skill0);
    if(type == 1) skill = dss.skill1, count = sizeof(dss.skill1);
    else if(type == 2) skill = dss.skill2, count = sizeof(dss.skill2);
    else if(type == 3) skill = dss.skill3, count = sizeof(dss.skill3);
    else if(type == 4) skill = dss.skill4, count = sizeof(dss.skill4);
    count /= sizeof(dss.skill0[0]);
    return std::find(skill, skill+count, skill_id) != skill + count;
}

//返回skill_id在skill中的位置,返回-1表示未找到
inline int get_order(int *skill, int count, int skill_id)
{
    if(skill_id <= 0) return -1;
    int ret = std::find(skill, skill+count, skill_id) - skill;
    return ret == count ? -1 : ret;
}

//保存技能,重复的技能只会保存一次
int dota_save_skill(int id, short *skill, int count, unsigned int mask[4])
{
    assert(id>=0 && id<54 && count>0);
    int point[5]={2,2,2,2,2}; //从data获取
    union
    {
        unsigned int mask[4];
        UInt64 mask64;
        unsigned short mask16[8];
    } save = {0};

    if(count != std::accumulate(point, point+5, 0))
        return -1;//技能点数不对

    int i = 0;
    //门派技能+飞升技能 填充mask64
    for(; i < point[0]+point[1]; ++i)
    {
        if(!check_skill(id,skill[i],i<point[0]?0:1))
            return -3;//不在配置表中
        int order = get_order(skill_common[id/3], 64, skill[i]);
        if(order < 0) return -5;//不符合职业
        save.mask64 |= (UInt64)1<<order;
    }
    //造化技能 填充mask[2]和mask16[6]
    for(; i < point[0]+point[1]+point[2]+point[3]; ++i)
    {
        if(!check_skill(id,skill[i],i<point[0]+point[1]+point[2]?2:3))
            return -7;//不在配置表中
        int order = get_order(skill_coculture[id%3], 32, skill[i]);
        if(order >= 0) save.mask[2] |= 1u<<order;
        else
        {
            order = get_order(skill_spculture[id], 16, skill[i]);
            if(order < 0) return -9;//不符合阵营
            save.mask16[6] |= 1<<order;
        }
    }
    //师徒技能 填充mask16[7]
    for(; i < point[0]+point[1]+point[2]+point[3]+point[4]; ++i)
    {
        if(!check_skill(id,skill[i],4))
            return -11;//不在配置表中
        int order = get_order(skill_sect, 16, skill[i]);
        if(order < 0) return -13;//不是师徒技能
        save.mask16[7] |= 1<<order;
    }

    mask[0] = save.mask[0];
    mask[1] = save.mask[1];
    mask[2] = save.mask[2];
    mask[3] = save.mask[3];
    return 0;
}

//读取技能
void dota_load_skill(int id, unsigned int mask[4], std::vector<int> &skill)
{
    union
    {
        unsigned int mask[4];
        UInt64 mask64;
        unsigned short mask16[8];
    } load = {mask[0],mask[1],mask[2],mask[3]};

    UInt64 t = load.mask64;
    for(int i=0; i<64; ++i)
    {
        if(t & 1 && skill_common[id/3][i]) skill.push_back(skill_common[id/3][i]);
        t = t>>1;
    }
    unsigned int u = load.mask[2];
    for(int i=0; i<32; ++i)
    {
        if(u & 1 && skill_coculture[id%3][i]) skill.push_back(skill_coculture[id%3][i]);
        u = u>>1;
    }
    unsigned short s = load.mask16[6];
    for(int i=0; i<16; ++i)
    {
        if(s & 1 && skill_spculture[id][i]) skill.push_back(skill_spculture[id][i]);
        s = s>>1;
    }
    s = load.mask16[7];
    for(int i=0; i<16; ++i)
    {
        if(s & 1 && skill_sect[i]) skill.push_back(skill_sect[i]);
        s = s>>1;
    }
}

int main()
{
    int id = 0;
    const int count = 10;
    short skill[count] = {2,1,3,4,5,6,7,8,9,10};

    unsigned int mask[4];
    int ret = dota_save_skill(0,skill,count,mask);
    printf("\ndota_save_skill ret is %d", ret);
    if(ret < 0) return -1;

    printf(", mask is %08x %08x %08x %08x", mask[0],mask[1],mask[2],mask[3]);
    puts("\n---------------------------------------------------------------------");
    puts("dota_load_skill:");
    std::vector<int> vskill;
    dota_load_skill(id, mask, vskill);
    std::ostream_iterator<int> oi(cout, ",");
    copy(vskill.begin(), vskill.end(), oi);
    cout << endl;
    return 0;
}
