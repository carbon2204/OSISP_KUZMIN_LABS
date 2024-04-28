#pragma once
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static timer_t createPTimer(void (*handler)(union sigval), union sigval val) {
  timer_t timer = NULL;
  struct sigevent sev = {
      .sigev_notify = SIGEV_THREAD,
      .sigev_notify_function = handler,
      .sigev_value = val
  };
  timer_create(CLOCK_REALTIME, &sev, &timer);
  return timer;
}

static timer_t createTimer(void (*handler)()) {
  return createPTimer(
      (void (*)(union sigval))handler,
      (union sigval){.sival_int = 0}
  );
}

static void startIntervalTimer(timer_t timer, long long nsec) {
  struct itimerspec its = {
      .it_value.tv_sec = nsec / 1000000000,
      .it_value.tv_nsec = nsec % 1000000000,
      .it_interval.tv_sec = nsec / 1000000000,
      .it_interval.tv_nsec = nsec % 1000000000
  };
  timer_settime(timer, 0, &its, NULL);
}

static inline long long ns(int ns) { return ns; }

static inline long long us(int us) { return us * 1000; }

static inline long long ms(int ms) { return ms * 1000000; }

static inline long long s(int s) { return s * 1000000000; }
