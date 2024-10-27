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

/*
 * @brief 调试初始化，使能中断，配置elog
 * @note 该函数涉及信号量，需要在任务中初始化，不能在main函数中初始化
 */
void Debug::Init() {
  /* Disable I/O buffering for STDOUT stream, so that
   * 关闭printf缓冲区 chars are sent out as soon as they are printed. */
  setvbuf(stdout, nullptr, _IONBF, 0);
  /* Endable usart IDLE interrupt */
  StartReceive();
  __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  /* 下面语句分别会触发一次中断 */
//  __HAL_UART_ENABLE_IT(&huart1, UART_IT_TC);
//  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
  elog_init();
  elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
  elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_FUNC | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_VERBOSE, ~ELOG_FMT_ALL);
  elog_start();

  log_v("EasyLogger V%s is initialize success.ver", ELOG_SW_VERSION);
  /* 输出软件信息 */
  PrintInfoMenu();
  PrintHelpMenu();
  elog_set_output_enabled(true);
  elog_set_raw_output_enabled(false);
}

void Debug::StartReceive() {
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer_, sizeof(rx_buffer_));
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

/*
 * @brief 输出软件信息
 */
void Debug::PrintInfoMenu() {
  /* From https://patorjk.com/software/taag Calvin S*/
  log_v("");
  log_v("╔╗║╦╔═╗╦ ╦╦ ╦╔═╗╦ ╦");
  log_v("║║║║║  ╠═╣║ ║║ ║║ ║");
  log_v("╝╚╝╩╚═╝╩ ╩╩╚╝╚═╝╚═╝");
  log_v("Build time: %s %s", __DATE__, __TIME__);
  log_v("Project Stm32H723_Debug, Version 1.0");
  log_v("Press h for help ......");
}

/*
 * @brief 输出帮助信息
 */
void Debug::PrintHelpMenu() {
  log_v("Help Menu:");
  log_v("h: Help menu");
  log_v("i: Print software information");
  log_v("s: Enter set parameter, key:value");
  log_v("l: Look up current parameters");
  log_v("t: Test output");
  log_v("Esc: Quit test output");
}

/*
 * @brief 输出当前参数
 */
void Debug::PrintCurrentParameters() {
  log_v("Current key:value************");
  for (auto &it : parameter_list_) {
    log_v("|%s:%.3f", it.first.c_str(), it.second);
  }
  log_v("*****************************");
}

/*
 * @brief 接收信息进行状态切换
 */
void Debug::FsmInput(uint8_t *str, int32_t length) {

  current_state_ = next_state_;
  uint8_t cmd = *str;
  switch (current_state_) {
    case STATE_START:break;
    case STATE_CMD: /* 等待命令 */
      ParseCommand(str, length);
      break;
    case STATE_SET: /* 设置参数 */
      if (cmd == KEY_ESC) {
        parameter_length_ = 0;
        next_state_ = STATE_CMD;
        PrintHelpMenu();
      } else if (cmd == KEY_ENTER) {
        ParseParameter(parameter_buffer_, parameter_length_);
        parameter_length_ = 0;
      } else if (cmd == KEY_BS) {
        if (parameter_length_ > 0)
          parameter_length_--;
        printf("\b \b");
      } else {
        parameter_buffer_[parameter_length_++] = cmd;
        printf("%c", cmd);
      }
      break;
    case STATE_TEST: /* 测试 */
      if (cmd == CMD_CANCEL && length == 1) {
        elog_set_raw_output_enabled(false);
        next_state_ = STATE_CMD;
      }
  }

  if (next_state_ != current_state_) {
    /* 首次进入状态输出信息 */
    switch (next_state_) {
      case STATE_CMD:PrintInfoMenu();
        break;
      case STATE_SET:PrintCurrentParameters();
        log_v("Press ESC to exit, Enter to set");
        break;
      case STATE_TEST:elog_set_raw_output_enabled(true);
        break;
    }
  }
}

/*
 * @brief STATE_CMD模式下解析命令
 */
void Debug::ParseCommand(uint8_t *str, int32_t length) {
  if (1 != length) {
    return;
  }
  uint8_t cmd = *str;
  switch (cmd) {
    case CMD_INFO:PrintInfoMenu();
      break;
    case CMD_MENU:PrintHelpMenu();
      break;
    case CMD_SET:next_state_ = STATE_SET;
      break;
    case CMD_LOOKUP: PrintCurrentParameters();
      break;
    case CMD_TEST:next_state_ = STATE_TEST;
      break;
    default:break;
  }
}

/*
 * @brief  解析参数
 * 示例: "key:value"
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
    log_v("");
    log_v("Set key| %s:%.3f", key.c_str(), parameter_list_.at(key));
    log_v("Press ESC to exit, Enter to set");
//    PrintCurrentParameters();
  }
}
extern "C" {

/*
 * @brief  Rx事件(IDLE)接收回调函数
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
  if (huart->Instance == huart1.Instance) {
    Debug &debug = Debug::instance(); // Copy will generate new instance, use &
//    debug.ParseParameter(huart->pRxBuffPtr, Size);
    debug.FsmInput(huart->pRxBuffPtr, Size);
    /* Start DMA again */
    Debug::instance().StartReceive();
  }
}

/*
 * @brief  Rx接收回调函数，单个字符接收
 * @note 开启了DMA接收，这个函数不会被调用
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == huart1.Instance) {
    HAL_UART_Transmit_DMA(&huart1, huart->pRxBuffPtr, 1);
  }
}

extern osSemaphoreId_t elog_dma_lockHandle;
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == huart1.Instance) {
    /* DMA发送锁，发送完成才能进行下一次发送 */
    osSemaphoreRelease(elog_dma_lockHandle);
  }
}

/*
 * @brief 信息输出任务，异步输出
 * @note elog_port_output会根据DMA状态挂起任务，所以可能会在任务挂起过程中log中出现
 * 新的数据，因此每次执行需要使用循环获取log，外部释放多少次信号量任务就可以循环多少次
 */
extern osSemaphoreId_t elog_asyncHandle;
extern void elog_port_output(const char *log, size_t size);
void DefaultTask(void *argument) {
  size_t get_log_size = 0;
#ifdef ELOG_ASYNC_LINE_OUTPUT
  static char poll_get_buf[ELOG_LINE_BUF_SIZE - 4];
#else
  static char poll_get_buf[ELOG_ASYNC_OUTPUT_BUF_SIZE - 4];
#endif

  for (;;) {
    /* waiting log 由elog_async_output释放 */
    /* 释放多少次就可以循环多少次 */
    osSemaphoreAcquire(elog_asyncHandle, osWaitForever);
//    while (1) {
    {
      /* polling gets and outputs the log */
#ifdef ELOG_ASYNC_LINE_OUTPUT
      get_log_size = elog_async_get_line_log(poll_get_buf, sizeof(poll_get_buf));
#else
      get_log_size = elog_async_get_log(poll_get_buf, sizeof(poll_get_buf));
#endif
      if (get_log_size) {
        elog_port_output(poll_get_buf, get_log_size);
      } else {
        break;
      }
    }
  }
}
}
