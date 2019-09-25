#include "time_utility.h"
//单双周间隔, 支持1970到2100年, 由于2100年不是闰年之后的结果会有问题
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
//判断距last_record_time到cur_time是不是跨了天
int is_another_day(time_t cur_time, time_t last_record_time, int gmtoff)
{
	//gmtoff: Seconds east of UTC, 东半球为正, 西半球为负
	//cur_time + gmtoff: 本地时间戳(从本地时间1970年1月1日开始所经过的秒数)
	//(cur_time + gmtoff) % SECS_PER_DAY: 本地今日0点开始所经过秒数
	//local_zero_time: 本地今日0点对应的Unix时间戳
	int local_zero_time = cur_time - (cur_time + gmtoff) % SECS_PER_DAY;
	return last_record_time < local_zero_time;
}
//判断距last_record_time到cur_time是不是跨了周
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
关于gmtoff
据man mktime, The glibc version of struct tm has additional fields
	long tm_gmtoff;           // Seconds east of UTC
	const char *tm_zone;      // Timezone abbreviation
defined when _BSD_SOURCE was set before including <time.h>.
gmtoff即为struct tm中的tm_gmtoff, 东八区该值为8*3600.
另外, 据man tzset, time.h中声明了
	void tzset(void);
	extern long timezone;
tzset()会读取本地的时区设置, 并设置timezone的值, localtime()中就会调用tzset(),
timezone是seconds West of UTC, 是上述tm_gmtoff的相反值.
对于struct tm不包含tm_gmtoff的系统可以这样获得gmtoff:
	time_t time_utc = 0;
	struct tm *p_tm_time = localtime(&time_utc);
	//东西12区就比较懵逼了
	int gmtoff = (p_tm_time->tm_hour > 12) ? (p_tm_time->tm_hour -= 24) : p_tm_time->tm_hour;
*/
