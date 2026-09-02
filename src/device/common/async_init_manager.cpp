/*
 * @Description: 异步初始化任务生命周期管理实现
 * @Author: LILYGO_L
 * @Date: 2026-08-17
 * @License: GPL 3.0
 */

#include "async_init_manager.h"

namespace lilygo_device_driver {
namespace {

constexpr uint32_t kStopPollIntervalMs = 10;

}  // namespace

void AsyncInitManager::Reset() {
  stop_requested_.store(false, std::memory_order_release);
}

bool AsyncInitManager::StartTask(TaskFunction_t task, const char* name,
    uint32_t stack_size, void* argument, UBaseType_t priority) {
  if (stop_requested()) {
    return false;
  }

  task_count_.fetch_add(1, std::memory_order_relaxed);
  if (xTaskCreate(task, name, stack_size, argument, priority, nullptr) ==
      pdPASS) {
    return true;
  }
  task_count_.fetch_sub(1, std::memory_order_release);
  return false;
}

bool AsyncInitManager::stop_requested() const {
  return stop_requested_.load(std::memory_order_acquire);
}

void AsyncInitManager::FinishTask() {
  task_count_.fetch_sub(1, std::memory_order_release);
  vTaskDelete(nullptr);
}

bool AsyncInitManager::StopAndWait(uint32_t timeout_ms) {
  stop_requested_.store(true, std::memory_order_release);
  for (uint32_t elapsed_ms = 0;
      task_count_.load(std::memory_order_acquire) != 0 &&
      elapsed_ms < timeout_ms;
      elapsed_ms += kStopPollIntervalMs) {
    vTaskDelay(pdMS_TO_TICKS(kStopPollIntervalMs));
  }
  return task_count_.load(std::memory_order_acquire) == 0;
}

}  // namespace lilygo_device_driver
