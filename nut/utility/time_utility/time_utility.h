/** 2018-10-11
*/
#ifndef __GCORE_TIME_UTILITY_H
#define __GCORE_TIME_UTILITY_H
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif
int is_odd_even();
int is_odd_even_start(int start_day);
int is_another_day(time_t cur_time, time_t last_record_time, int gmtoff);
int is_another_week(time_t cur_time, time_t last_record_time, int gmtoff);
int is_another_week_start(time_t cur_time, time_t last_record_time, int gmtoff, int start_day);
#ifdef __cplusplus
}
#endif
#endif
