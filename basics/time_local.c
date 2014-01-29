#include <stdio.h>      /* puts, printf */
#include <time.h>       /* time_t, struct tm, time, localtime */

int main ()
{
  time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime); //Note pointer to tm struct
  printf ("Current local time and date: %s", asctime(timeinfo));

  /*
   * STRUCT tm
   tm_sec  int seconds after the minute    0-61*
   tm_min   int minutes after the hour  0-59
   tm_hour  int hours since midnight    0-23
   tm_mday  int day of the month    1-31
   tm_mon   int months since January    0-11
   tm_year  int years since 1900    
   tm_wday  int days since Sunday   0-6
   tm_yday  int days since January 1    0-365
   tm_isdst int Daylight Saving Time flag
   */
   printf("month = %d, weekday = %d, dayofmonth = %d\n hour = %d, minute = %d\n",
           timeinfo->tm_mon, timeinfo->tm_wday, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min
         );

  return 0;
}
