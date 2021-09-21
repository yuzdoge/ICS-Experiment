#include <am.h>
#include <nemu.h>

static uint64_t am_start_time;

void __am_timer_init() {
  am_start_time = inl(RTC_ADDR) + inl(RTC_ADDR + 4) * 1000 * 1000;
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
//  uptime->us = 0;
  uptime->us = inl(RTC_ADDR) + inl(RTC_ADDR + 4) * 1000 * 1000 - am_start_time;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
