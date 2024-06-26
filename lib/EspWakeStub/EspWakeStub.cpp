#include "EspWakeStub.h"

void RTC_IRAM_ATTR rtcSetWakeupTime(uint64_t time_in_us) {
  uint64_t rtc_count_delta = rtc_cntl_ll_time_to_count(time_in_us);
  uint64_t rtc_curr_count = rtc_cntl_ll_get_rtc_time();
  rtc_cntl_ll_set_wakeup_timer(rtc_curr_count + rtc_count_delta);
}

void RTC_IRAM_ATTR rtcSleep(esp_deep_sleep_wake_stub_fn_t new_stub) {
  // Since the app core of esp32 does not support access to RTC_FAST_MEMORY,
  // esp_set_deep_sleep_wake_stub is not declared in RTC_FAST_MEMORY,
  REG_WRITE(RTC_ENTRY_ADDR_REG, (uint32_t)new_stub);

  // Set CRC of Fast RTC memory
  set_rtc_memory_crc();

  // Go to sleep.
  rtc_cntl_ll_sleep_enable();

  // A few CPU cycles may be necessary for the sleep to start
  asm volatile("waiti 0\n");
}

esp_sleep_wakeup_cause_t RTC_IRAM_ATTR rtcWakeupCause(void) {
  uint32_t wakeup_cause = rtc_cntl_ll_get_wakeup_cause();

  if (wakeup_cause & RTC_TIMER_TRIG_EN) {
    return ESP_SLEEP_WAKEUP_TIMER;
  } else if (wakeup_cause & RTC_GPIO_TRIG_EN) {
    return ESP_SLEEP_WAKEUP_GPIO;
  } else if (wakeup_cause & (RTC_UART0_TRIG_EN | RTC_UART1_TRIG_EN)) {
    return ESP_SLEEP_WAKEUP_UART;
  } else if (wakeup_cause & RTC_EXT0_TRIG_EN) {
    return ESP_SLEEP_WAKEUP_EXT0;
  } else if (wakeup_cause & RTC_EXT1_TRIG_EN) {
    return ESP_SLEEP_WAKEUP_EXT1;
  } else if (wakeup_cause & RTC_TOUCH_TRIG_EN) {
    return ESP_SLEEP_WAKEUP_TOUCHPAD;
  } else if (wakeup_cause & RTC_ULP_TRIG_EN) {
    return ESP_SLEEP_WAKEUP_ULP;
  } else if (wakeup_cause & RTC_BT_TRIG_EN) {
    return ESP_SLEEP_WAKEUP_BT;
  } else {
    return ESP_SLEEP_WAKEUP_UNDEFINED;
  }
}

uint16_t RTC_IRAM_ATTR rtcAdc1GetRaw(adc1_channel_t channel) {
  // adc aquire and power on
  SENS.sar_read_ctrl.sar1_dig_force = 0;
  SENS.sar_meas_wait2.force_xpd_sar = SENS_FORCE_XPD_SAR_PU;
  RTCIO.hall_sens.xpd_hall = false;
  SENS.sar_meas_wait2.force_xpd_amp = SENS_FORCE_XPD_AMP_PD;

  // disable FSM
  SENS.sar_meas_ctrl.amp_rst_fb_fsm = 0;
  SENS.sar_meas_ctrl.amp_short_ref_fsm = 0;
  SENS.sar_meas_ctrl.amp_short_ref_gnd_fsm = 0;
  SENS.sar_meas_wait1.sar_amp_wait1 = 1;
  SENS.sar_meas_wait1.sar_amp_wait2 = 1;
  SENS.sar_meas_wait2.sar_amp_wait3 = 1;

  // set controller rtc
  SENS.sar_read_ctrl.sar1_dig_force = false;
  SENS.sar_meas_start1.meas1_start_force = true;
  SENS.sar_meas_start1.sar1_en_pad_force = true;
  SENS.sar_touch_ctrl1.xpd_hall_force = true;
  SENS.sar_touch_ctrl1.hall_phase_force = true;

  // config adc
  SENS.sar_read_ctrl.sar1_data_inv = true;

  // start conversion
  SENS.sar_meas_start1.sar1_en_pad = (1 << channel);
  while (SENS.sar_slave_addr1.meas_status != 0);
  SENS.sar_meas_start1.meas1_start_sar = 0;
  SENS.sar_meas_start1.meas1_start_sar = 1;
  while (SENS.sar_meas_start1.meas1_done_sar == 0);

  // set adc value
  uint16_t adcValue = SENS.sar_meas_start1.meas1_data_sar;

  // adc power off
  SENS.sar_meas_wait2.force_xpd_sar = SENS_FORCE_XPD_SAR_PD;

  return adcValue;
}

bool RTC_IRAM_ATTR rtcGpioGetLevel(gpio_num_t gpio) {
  return (GPIO_REG_READ(GPIO_IN_REG) >> gpio) & 1U;
}

int RTC_IRAM_ATTR rtcMap(int value, int fromLow, int fromHigh, int toLow, int toHigh) {
  return toLow + ((value - fromLow) * (toHigh - toLow)) / (fromHigh - fromLow);
}
