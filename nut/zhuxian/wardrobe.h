//�����ķ�Ⱦɫʱװ
struct gconsign_suite
{
    int figure_id;
    std::vector<int> items0;
    std::vector<int> items1;
    std::vector<int> items2;
};
//������Ⱦɫʱװ
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
    int sleeve_colour;//���ӵ���ɫ
    int upper_colour;//�ϰڵ���ɫ
    int down_colour;//�°ڵ���ɫ
    int expire_date;
};
struct colour_fashion_head_scheme
{
    int hair_colour;//ͷ������ɫ
    int bun_colour;//���ٵ���ɫ
    int expire_date;
};
struct colour_fashion_foot_scheme
{
    int foot_colour;//Ь�ӵ���ɫ
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
    int unlock_colour;//��������ɫ

    //��ǰʹ�õ�ͷ�����
    int item_id1;
    int hair_colour;//ͷ������ɫ
    int bun_colour;//���ٵ���ɫ
    int expire_date1;//����ʱ��,���<=0���޵���ʱ��

    //��ǰʹ�õ��·����
    int item_id2;
    int sleeve_colour;//���ӵ���ɫ
    int upper_colour;//�ϰڵ���ɫ
    int down_colour;//�°ڵ���ɫ
    int expire_date2;

    //��ǰʹ�õ�Ь�����
    int item_id3;
    int foot_colour;//Ь�ӵ���ɫ
    int expire_date3;

    int item0,item1,item2,item3;//����ʹ�õ�ͷ�Ρ��·���Ь�ӡ���ߵ�id

    std::vector<gconsign_suite> suite;
    std::vector<gconsign_colour_suite> colour_suite;

    std::vector<int> masks;//���������
    std::vector<int> titles;//������ͼ�γƺ�

    int weapon_feature;//��ǰʹ�õ��������

    int item4;//����ʹ�õ�����id
    int item5,item6,item7;//δʹ��

    //��ǰʹ�õĸ�ħ����:foot body weapon
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

    std::vector<gconsign_wardrobe_magic> weapon;//������ʱװ����
    std::vector<gconsign_wardrobe_magic> foot;//������ʱװ��ħЬ��
    std::vector<gconsign_wardrobe_magic> body;//������ʱװ��ħ�·�
};
