/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */

#include <elog.h>
#include <stdio.h>
#include "cmsis_os2.h"
#include "usart.h"


/* 信号量，需要在CubeMX中配置 */
extern osSemaphoreId_t elog_lockHandle; /*< 发送信号量 */
extern osSemaphoreId_t elog_asyncHandle; /*< 异步发送信号量，释放时才一起发送 */
extern osSemaphoreId_t elog_dma_lockHandle; /*< 需要在发送完成时释放该信号量 */

/**
 * EasyLogger port initialize
 *
 * @return result
 */
extern void elog_port_output_unlock(void);
ElogErrCode elog_port_init(void) {
  ElogErrCode result = ELOG_NO_ERR;
  elog_port_output_unlock();
  /* add your code here */
  return result;
}

/**
 * EasyLogger port deinitialize
 *
 */
void elog_port_deinit(void) {

}

uint8_t log_buf[1024] = {0};
/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
  HAL_UART_Transmit_DMA(&huart1, (uint8_t *) log, size);
  /* 等待发送完成 */
  osSemaphoreAcquire(elog_dma_lockHandle, osWaitForever);
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
  osSemaphoreAcquire(elog_lockHandle, osWaitForever);
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
  osSemaphoreRelease(elog_lockHandle);
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
  static char cur_system_time[16] = "";
  snprintf(cur_system_time, 16, "%lu", osKernelGetTickCount());
  return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
  return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
  return "";
}

void elog_async_output_notice(void) {
  osSemaphoreRelease(elog_asyncHandle);
}



