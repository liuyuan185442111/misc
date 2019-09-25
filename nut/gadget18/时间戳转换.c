#include <stdio.h>
#include <stdint.h>//for int64_t

struct tm
{
	int tm_year;/* Year: 1970- */
	int tm_mon;/* Months since january: 1-12 */
	int tm_day;/* Day of the month: 1-31 */
	int tm_hour;/* Hours: 0-23 */
	int tm_min;/* Minutes: 0-59 */
	int tm_sec;/* Seconds: 0-59 */
	int tm_wday;/* Weekdays: 0-6, 0 is Sunday */
	int tm_yday;/* Days since Jan. 1: 1-366 */
};

/* How many days by each month. */
const int month_days[2][12] =
{
	/* Normal years.  */
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
	/* Leap years.  */
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }
};

#define SECS_PER_HOUR   (60 * 60)
#define SECS_PER_DAY    (24 * SECS_PER_HOUR)
#define isleap(year)    ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

int offset = 8*3600;

struct tm *offtime(int t)
{
	static struct tm tbuf;
	int days, year, rem;
	const int *pi;

	t += offset;
	days = t / SECS_PER_DAY;
	rem = t % SECS_PER_DAY;
	tbuf.tm_hour = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	tbuf.tm_min = rem / 60;
	tbuf.tm_sec = rem % 60;

	/* January 1, 1970 was a Thursday.  */
	tbuf.tm_wday = (4 + days) % 7;

	year = 1970;
	while(days >= (rem = isleap(year) ? 366 : 365))
	{
		++year;
		days -= rem;
	}
	tbuf.tm_year = year;
	tbuf.tm_yday = days + 1;

	pi = month_days[rem-365];
	for(rem = 11; days < pi[rem]; --rem);
	days -= pi[rem];
	tbuf.tm_mon = rem + 1;
	tbuf.tm_day = days + 1;

	return &tbuf;
}

struct tm *offtime64(int64_t t)
{
	static struct tm tbuf;
	int64_t days64;
	int days, year, rem;
	const int *pi;

	t += offset;
	days64 = t / SECS_PER_DAY;
	rem = t % SECS_PER_DAY;
	tbuf.tm_hour = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	tbuf.tm_min = rem / 60;
	tbuf.tm_sec = rem % 60;

	/* January 1, 1970 was a Thursday.  */
	tbuf.tm_wday = (4 + days64) % 7;

	year = 1970;
	while(days64 >= (rem = isleap(year) ? 366 : 365))
	{
		++year;
		days64 -= rem;
	}
	days = days64;
	tbuf.tm_year = year;
	tbuf.tm_yday = days + 1;

	pi = month_days[rem-365];
	for(rem = 11; days < pi[rem]; --rem);
	days -= pi[rem];
	tbuf.tm_mon = rem + 1;
	tbuf.tm_day = days + 1;

	return &tbuf;
}

int mktime(struct tm *t)
{
	return ((t->tm_year-1970)*365 + (t->tm_year-1969)/4 + month_days[isleap(t->tm_year)?1:0][t->tm_mon-1] + t->tm_day-1)
		* SECS_PER_DAY + (t->tm_hour*60+t->tm_min)*60 + t->tm_sec - offset;
}

//support from year 1970 to year 2400(included)
int64_t mktime64(struct tm *t)
{
	//Year 2100,2200,2300 is not leap year!
	return (t->tm_hour*60+t->tm_min)*60 + t->tm_sec - offset + (int64_t)SECS_PER_DAY *
		((t->tm_year-1970)*365 + (t->tm_year-1969)/4 + (t->tm_year>2100?-1-(t->tm_year-2101)/100:0)
			+ month_days[isleap(t->tm_year)?1:0][t->tm_mon-1] + t->tm_day-1);
}

int main()
{
	struct tm *p;
	int max = (unsigned int)-1>>1;
	int counter = 0;
	int last_time = time(NULL);
	int64_t t = 0;
	for(; ; ++t)
	{
		if(++counter > 100000000)
		{
			counter = 1;
			int cur_time = time(NULL);
			printf("walking %lld use seconds=%d\n",t,cur_time-last_time);
			fflush(stdout);
			last_time = cur_time;
		}
		p = offtime64(t);
		if(t != mktime64(p))
		{
			printf("error t=%lld,ret=%lld\n", t,mktime64(p));
			printf("%d %d %d\n", p->tm_year,p->tm_mon,p->tm_day);
			printf("%d %d %d\n", p->tm_hour,p->tm_min,p->tm_sec);
			return 0;
		}
	}
	return 0;
}
