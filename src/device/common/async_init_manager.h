/*
 * @Description: 异步初始化任务生命周期管理接口
 * @Author: LILYGO_L
 * @Date: 2026-08-17
 * @License: GPL 3.0
 */

#pragma once

#include <atomic>
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace lilygo_device_driver {

// 管理设备异步初始化任务的创建、停止和完成同步。
class AsyncInitManager {
 public:
  /**
   * @brief 清除停止请求，为新一轮异步初始化做准备。
   */
  void Reset();

  /**
   * @brief 创建异步初始化任务并将其加入运行计数。
   * @param task 任务入口函数。
   * @param name 任务名称。
   * @param stack_size 任务栈大小。
   * @param argument 传递给任务入口函数的参数。
   * @param priority 任务优先级。
   * @return 任务创建成功时返回 true，否则返回 false。
   */
  bool StartTask(TaskFunction_t task, const char* name, uint32_t stack_size,
      void* argument, UBaseType_t priority);

  /**
   * @brief 查询是否已请求停止异步初始化。
   * @return 已请求停止时返回 true，否则返回 false。
   */
  bool stop_requested() const;

  /**
   * @brief 将当前任务移出运行计数并删除当前任务。
   */
  void FinishTask();

  /**
   * @brief 请求停止异步初始化并等待所有已创建任务结束。
   * @param timeout_ms 最长等待时间，单位为毫秒。
   * @return 所有任务在超时前结束时返回 true，否则返回 false。
   */
  bool StopAndWait(uint32_t timeout_ms);

 private:
  // 当前仍在运行的异步初始化任务数量。
  std::atomic<uint32_t> task_count_{0};
  // 是否已请求停止异步初始化。
  std::atomic<bool> stop_requested_{false};
};

}  // namespace lilygo_device_driver
