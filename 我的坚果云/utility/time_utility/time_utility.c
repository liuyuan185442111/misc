#include "time_utility.h"
//��˫�ܼ��, ֧��1970��2100��, ����2100�겻������֮��Ľ����������
//A week starts from Monday.
int is_odd_even()
{
	time_t cur_time = time(NULL);
	struct tm *t = localtime(&cur_time);
    return ((t->tm_year-70)*365+(t->tm_year-69)/4+t->tm_yday-4)/7%2;
}
//start_day: 0-7, 0 or 7 means Sunday
int is_odd_even_start(int start_day)
{
	time_t cur_time = time(NULL);
	struct tm *t = localtime(&cur_time);
    return ((t->tm_year-70)*365+(t->tm_year-69)/4+t->tm_yday+4-start_day)/7%2;
}
#define SECS_PER_DAY (24*60*60)
//�жϾ�last_record_time��cur_time�ǲ��ǿ�����
int is_another_day(time_t cur_time, time_t last_record_time, int gmtoff)
{
	//gmtoff: Seconds east of UTC, ������Ϊ��, ������Ϊ��
	//cur_time + gmtoff: ����ʱ���(�ӱ���ʱ��1970��1��1�տ�ʼ������������)
	//(cur_time + gmtoff) % SECS_PER_DAY: ���ؽ���0�㿪ʼ����������
	//local_zero_time: ���ؽ���0���Ӧ��Unixʱ���
	int local_zero_time = cur_time - (cur_time + gmtoff) % SECS_PER_DAY;
	return last_record_time < local_zero_time;
}
//�жϾ�last_record_time��cur_time�ǲ��ǿ�����
//A week starts from Monday.
int is_another_week(time_t cur_time, time_t last_record_time, int gmtoff)
{
    int days_since_monday = (4 + (cur_time + gmtoff) / SECS_PER_DAY)%7 - 1;
    if(days_since_monday < 0) days_since_monday += 7;
    return last_record_time < cur_time - (cur_time + gmtoff) % SECS_PER_DAY - days_since_monday * SECS_PER_DAY;
}
//start_day: 0-7, 0 or 7 means Sunday
int is_another_week_start(time_t cur_time, time_t last_record_time, int gmtoff, int start_day)
{
    int days_since_start = (4 + (cur_time + gmtoff) / SECS_PER_DAY)%7 - start_day;
    if(days_since_start < 0) days_since_start += 7;
    return last_record_time < cur_time - (cur_time + gmtoff) % SECS_PER_DAY - days_since_start * SECS_PER_DAY;
}
#undef SECS_PER_DAY

/**
����gmtoff
��man mktime, The glibc version of struct tm has additional fields
	long tm_gmtoff;           // Seconds east of UTC
	const char *tm_zone;      // Timezone abbreviation
defined when _BSD_SOURCE was set before including <time.h>.
gmtoff��Ϊstruct tm�е�tm_gmtoff, ��������ֵΪ8*3600.
����, ��man tzset, time.h��������
	void tzset(void);
	extern long timezone;
tzset()���ȡ���ص�ʱ������, ������timezone��ֵ, localtime()�оͻ����tzset(),
timezone��seconds West of UTC, ������tm_gmtoff���෴ֵ.
����struct tm������tm_gmtoff��ϵͳ�����������gmtoff:
	time_t time_utc = 0;
	struct tm *p_tm_time = localtime(&time_utc);
	//����12���ͱȽ��±���
	int gmtoff = (p_tm_time->tm_hour > 12) ? (p_tm_time->tm_hour -= 24) : p_tm_time->tm_hour;
*/
