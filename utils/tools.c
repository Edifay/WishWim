#include "tools.h"
#include <unistd.h>
#include <sys/time.h>

time_val timeInMilliseconds(void) {
  struct timeval tv;

  gettimeofday(&tv,NULL);
  return (((time_val)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

time_val diff2Time(time_val start, time_val end) {
  time_val diff = start - end;
  if (diff < 0)
    return -diff;
  return diff;
}


int min(int a, int b) {
  if (a < b) return a;
  return b;
}
