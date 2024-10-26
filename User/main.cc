#include "main.h"
#include "cmsis_os.h"

#define LOG_TAG "MAIN"
#include "debug.h"
#include "tim.h"
#include "stm32h7xx.h"

void Main() {
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 810);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 50);
}

extern "C" {
/*
 * @brief IMU任务
 */
void TestTask(void *argument) {
  static const char *TAG = "IMU";
  Debug::instance().Init();
  log_w("Hello nichijou warning!");
  log_i("Hello nichijou info!");
  log_d("Hello nichijou debug!");
  log_v("Hello nichijou verbose!");
  log_e("Hello World!");
  vTaskDelay(2000);
  static TickType_t xLastWakeTime = xTaskGetTickCount();
  int xx = 0;
  while (1) {
    log_w("Hello World!:%d", xx);
    elog_raw("raw:%d\n", xx);
    xx++;
    vTaskDelayUntil(&xLastWakeTime, 500);
  }
}

}
