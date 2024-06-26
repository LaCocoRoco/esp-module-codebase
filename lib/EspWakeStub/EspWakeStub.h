#ifndef ESP_WAKE_STUB_H
#define ESP_WAKE_STUB_H

#include "driver/adc.h"
#include "esp_sleep.h"
#include "hal/rtc_cntl_ll.h"
#include "rom/rtc.h"
#include "soc/sens_reg.h"

void RTC_IRAM_ATTR rtcSetWakeupTime(uint64_t time_in_us);
void RTC_IRAM_ATTR rtcSleep(esp_deep_sleep_wake_stub_fn_t new_stub);
esp_sleep_wakeup_cause_t RTC_IRAM_ATTR rtcWakeupCause(void);
uint16_t RTC_IRAM_ATTR rtcAdc1GetRaw(adc1_channel_t channel);
bool RTC_IRAM_ATTR rtcGpioGetLevel(gpio_num_t gpio);
int RTC_IRAM_ATTR rtcMap(int value, int fromLow, int fromHigh, int toLow, int toHigh);

#endif