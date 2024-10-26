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
  Debug::instance().Init();
}

extern "C" {
/*
 * @brief IMU任务
 */
void ImuTask(void *argument) {
  static const char *TAG = "IMU";
  static int i = 0;
  static TickType_t xLastWakeTime = xTaskGetTickCount();
  log_w("Hello nichijou warning!");
  log_e("Hello World!");
  int xx = 0;
  while (1) {
    log_w("Hello World!:%d", xx);
    xx++;
//    mpu_get_data();
//    imu_ahrs_update();
//    imu_attitude_update();
//    mlx90393_a.Loop();
//    mlx90393_b.Loop();
//    mlx90393_c.Loop();
    i++;
    vTaskDelayUntil(&xLastWakeTime, 500);
  }
}

}
