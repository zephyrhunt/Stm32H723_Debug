#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <cstdio>
#include <cstdint>
#include <string>
#include <map>

#include "stm32h7xx_hal.h"
#include "elog.h"

class Debug {
 public:
  static Debug &instance();

  void Init();
  void ParseParameter(uint8_t *str, int32_t length);
  void StartReceive();
  void Test();
  void PrintInfo();

  float GetParameter(const std::string &key) {
    parameter_changed_list_[key] = false;
    return parameter_list_[key]; /* If key not found, it will return 0 */
  }

  bool IsParameterChanged(const std::string &key) {
    return parameter_changed_list_[key];
  }

  void SetParameter(const std::string &key, float value) {
    parameter_changed_list_[key] = true;
    parameter_list_[key] = value;
  }

 private:
  /* 参数列表 */
  std::map<std::string, float> parameter_list_;
  /* 标志参数被修改 */
  std::map<std::string, bool> parameter_changed_list_;
  uint8_t rx_buffer_[256];
  UART_HandleTypeDef *huart;
};

#endif
