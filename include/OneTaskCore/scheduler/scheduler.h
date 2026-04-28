// /include/OneTaskCore/scheduler/scheduler.h
#pragma once
#include <memory>
#include <vector>

#include "monitor/monitor.h"
#include "task/task.h"
namespace OneTaskCore {

class Scheduler {
 public:
  void AddTask(std::shared_ptr<Task> task);

  void Run();  // 主循环
  const TaskMonitor& GetMonitor() const { return m_monitor; }

 private:
  std::vector<std::shared_ptr<Task>> m_tasks;
  TaskMonitor m_monitor;

  std::shared_ptr<Task> PickNextTask();
  void CleanupTasks();
};

}  // namespace OneTaskCore