#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <cstdio>
#include <cstdint>
#include <string>
#include <map>

#include "stm32h7xx_hal.h"
#include "elog.h"

class Debug {
  enum {
    KEY_ENTER = 0x0D, /*< 回车 */
    KEY_BS = 127, /*< 退格键 */
    KEY_CANCLE = 0x03, /*< Ctrl+C */
    KEY_ESC = 0x1B /*< ESC键 */
  };
  enum { CMD_MENU = 'h', CMD_INFO = 'i', CMD_SET = 's', CMD_LOOKUP = 'l', CMD_TEST = 't', CMD_CANCEL = KEY_ESC };
  enum {
    STATE_START = 0, /*< 开始 */
    STATE_CMD,  /*< 等待命令模式 */
    STATE_SET, /*< 设置参数 */
    STATE_TEST, /*< 测试，连续输出调试信息 */
  };
 public:
  static Debug &instance();

  void Init();
  void ParseCommand(uint8_t *str, int32_t length);
  void ParseParameter(uint8_t *str, int32_t length);
  void StartReceive();
  void Test();
  void PrintInfoMenu();
  void PrintHelpMenu();
  void PrintCurrentParameters();
  void FsmInput(uint8_t *str, int32_t length);

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
  uint8_t parameter_buffer_[128];
  uint16_t parameter_length_ = 0;
  uint8_t rx_buffer_[256];
  UART_HandleTypeDef *huart;
  uint8_t current_state_ = STATE_START;
  uint8_t next_state_ = STATE_CMD;
};

#endif
