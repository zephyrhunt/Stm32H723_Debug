#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <cstdio>
#include <cstdint>
#include <string>
#include <map>

#include "stm32h7xx_hal.h"
#include "elog.h"
//#include "EasyLog/inc/elog.h"
class Debug {
 public:
  static Debug &instance();

  void Init();
  void ParseParameter(uint8_t *str, int32_t length);
  void StartReceive();
  void Test();
  float GetParameter(const std::string &key) {
    parameter_changed_list_[key] = 0;
    return parameter_list_[key]; /* If key not found, it will return 0 */
  }

  uint16_t GetParameterInt16(const std::string &key) {
    parameter_changed_list_[key] = 0;
    return static_cast<uint16_t>(parameter_list_[key]); /* If key not found, it will return 0 */
  }

  uint8_t IsParameterChanged(const std::string &key) {
    return parameter_changed_list_[key];
  }

  void SetParameter(const std::string &key, float value) {
    parameter_changed_list_[key] = 1;
    parameter_list_[key] = value;
  }

 private:
  std::map<std::string, float> parameter_list_;
  std::map<std::string, uint8_t> parameter_changed_list_;
  uint8_t rx_buffer_[256];
  UART_HandleTypeDef *huart;
};

#endif
