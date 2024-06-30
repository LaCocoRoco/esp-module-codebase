#ifndef GPIO_H
#define GPIO_H

#include <Arduino.h>
#include <driver/adc.h>

#ifdef ESP_CU
static const gpio_num_t GPIO_TX0 = GPIO_NUM_1;
static const gpio_num_t GPIO_RX0 = GPIO_NUM_3;
static const gpio_num_t GPIO_MOD = GPIO_NUM_34;
static const gpio_num_t GPIO_BUZ = GPIO_NUM_15;
static const gpio_num_t GPIO_MR = GPIO_NUM_4;
static const gpio_num_t GPIO_DS = GPIO_NUM_25;
static const gpio_num_t GPIO_SHCP = GPIO_NUM_26;
static const gpio_num_t GPIO_STCP = GPIO_NUM_27;
#endif

#ifdef ESP_OU
static const gpio_num_t GPIO_TX0 = GPIO_NUM_1;
static const gpio_num_t GPIO_RX0 = GPIO_NUM_3;
static const gpio_num_t GPIO_MOD = GPIO_NUM_12;
static const gpio_num_t GPIO_BUZ = GPIO_NUM_14;
static const gpio_num_t GPIO_BAT = GPIO_NUM_32;
static const gpio_num_t GPIO_CHG = GPIO_NUM_13;
static const gpio_num_t GPIO_SDA = GPIO_NUM_21;
static const gpio_num_t GPIO_SCL = GPIO_NUM_22;
#endif

#ifdef ESP_SU
static const gpio_num_t GPIO_TX0 = GPIO_NUM_1;
static const gpio_num_t GPIO_RX0 = GPIO_NUM_3;
static const gpio_num_t GPIO_MOD = GPIO_NUM_12;
static const gpio_num_t GPIO_BUZ = GPIO_NUM_14;
static const gpio_num_t GPIO_BAT = GPIO_NUM_32;
static const gpio_num_t GPIO_RX1 = GPIO_NUM_16;
static const gpio_num_t GPIO_TX1 = GPIO_NUM_17;
static const gpio_num_t GPIO_OUT = GPIO_NUM_13;
static const gpio_num_t GPIO_SNS = GPIO_NUM_33;
static const adc1_channel_t GPIO_SNS_CH = ADC1_CHANNEL_5;
#endif

static const int BUZZER_PWM_FREQUENCY = 2700;
static const int BUZZER_PWM_CHANNEL = 0;
static const int BUZZER_PWM_RESOLUTION = 8;
static const int BUZZER_PWM_DUTY_CYCLE_ON = 100;
static const int BUZZER_PWM_DUTY_CYCLE_OFF = 0;

#ifndef ESP_CU
extern float batteryVoltage;
#endif

void setupGpio();
void taskGpio();
void gpioController();
#ifndef ESP_CU
void batteryController();
#endif

#endif