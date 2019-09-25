//点亮的非染色时装
struct gconsign_suite
{
    int figure_id;
    std::vector<int> items0;
    std::vector<int> items1;
    std::vector<int> items2;
};
//点亮的染色时装
struct gconsign_colour_suite
{
    int figure_id;
    int item_id0;
    std::vector<colour_fashion_body_scheme> schemes0;
    int item_id1;
    std::vector<colour_fashion_head_scheme> schemes1;
    int item_id2;
    std::vector<colour_fashion_foot_scheme> schemes2;
};

struct colour_fashion_body_scheme
{
    int sleeve_colour;//袖子的颜色
    int upper_colour;//上摆的颜色
    int down_colour;//下摆的颜色
    int expire_date;
};
struct colour_fashion_head_scheme
{
    int hair_colour;//头发的颜色
    int bun_colour;//发髻的颜色
    int expire_date;
};
struct colour_fashion_foot_scheme
{
    int foot_colour;//鞋子的颜色
    int expire_date;
};

struct gconsign_wardrobe_magic
{
    int figure_id;
    int items[3];
};

struct ConsignWardrobe
{
    int version;
    int unlock_colour;//解锁的颜色

    //当前使用的头饰外观
    int item_id1;
    int hair_colour;//头发的颜色
    int bun_colour;//发髻的颜色
    int expire_date1;//过期时间,如果<=0则无到期时间

    //当前使用的衣服外观
    int item_id2;
    int sleeve_colour;//袖子的颜色
    int upper_colour;//上摆的颜色
    int down_colour;//下摆的颜色
    int expire_date2;

    //当前使用的鞋子外观
    int item_id3;
    int foot_colour;//鞋子的颜色
    int expire_date3;

    int item0,item1,item2,item3;//真正使用的头饰、衣服、鞋子、面具的id

    std::vector<gconsign_suite> suite;
    std::vector<gconsign_colour_suite> colour_suite;

    std::vector<int> masks;//点亮的面具
    std::vector<int> titles;//点亮的图形称号

    int weapon_feature;//当前使用的武器外观

    int item4;//真正使用的武器id
    int item5,item6,item7;//未使用

    //当前使用的附魔道具:foot body weapon
    int item_id11;
    int expire_date11;
    int item_id12;
    int expire_date12;
    int item_id13;
    int expire_date13;
    int item_id14;
    int expire_date14;
    int item_id15;
    int expire_date15;

    std::vector<gconsign_wardrobe_magic> weapon;//点亮的时装武器
    std::vector<gconsign_wardrobe_magic> foot;//点亮的时装附魔鞋子
    std::vector<gconsign_wardrobe_magic> body;//点亮的时装附魔衣服
};
