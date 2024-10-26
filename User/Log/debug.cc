//
// Created by A on 2024/6/18.
//

#include "debug.h"

#include <cstdint>
#include <cstdarg>
#include "usart.h"
#include "cmsis_os2.h"

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

extern "C" {
int _write(int fd, char *ptr, int len) {
  if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
    /* DMA send !!!IMPORTANT in hal uart interrupt need be open*/
    /* Used for last byte sending completion detection in DMA non circular mode */
    if (HAL_UART_GetState(&huart1) != HAL_UART_STATE_BUSY_TX)
      HAL_UART_Transmit_DMA(&huart1, (uint8_t *) ptr, len);
    return len;
  } else
    return -1;
}
}

void Debug::Init() {
  /* Disable I/O buffering for STDOUT stream, so that
   * 关闭printf缓冲区 chars are sent out as soon as they are printed. */
  setvbuf(stdout, nullptr, _IONBF, 0);
  /* Endable usart IDLE interrupt */
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer_, sizeof(rx_buffer_));
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  /* initialize EasyLogger */
  elog_init();
//  elog_async_enabled(1);
//  elog_buf_enabled(1);
  elog_output_lock_enabled(1);
/* set EasyLogger log format */
  elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
  elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_FUNC | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
  elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
/* start EasyLogger */
  elog_start();
}

/*
 * Parse the parameter from the string
 * Example: "key:value"
 */
void Debug::ParseParameter(uint8_t *str, int32_t length) {
  std::string cpp_str(reinterpret_cast<char *>(str), length);

  auto pos = cpp_str.find(':');
  if (pos != std::string::npos) {
    std::string key = cpp_str.substr(0, pos);
    std::string value = cpp_str.substr(pos + 1);
    parameter_list_[key] = std::stof(value);
    parameter_changed_list_[key] = 1;
    /* 使用特殊格式，不要覆盖vofa的数据 */
    printf("key--%s_value--%.2f\n", key.c_str(), parameter_list_.at(key));
  }
}

void Debug::StartReceive() {
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer_, sizeof(rx_buffer_));
  __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
}
void Debug::Test() {
  if (parameter_list_.find("test") != parameter_list_.end()) {
    printf("Test success value:%f\n", parameter_list_["test"]);
  } else {
    printf("Test no data\n");
  }
}
Debug &Debug::instance() {
  static Debug debug;
  return debug;
}

extern "C" {
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
  if (huart->Instance == huart1.Instance) {
    Debug &debug = Debug::instance(); // Copy will generate new instance, use &
    debug.ParseParameter(huart->pRxBuffPtr, Size);
    /* Start DMA again */
    Debug::instance().StartReceive();
  }
}

}
