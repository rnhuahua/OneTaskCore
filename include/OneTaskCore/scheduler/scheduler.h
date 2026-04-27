// /include/OneTaskCore/scheduler/scheduler.h
#pragma once
#include <memory>
#include <vector>

#include "task/task.h"

namespace OneTaskCore {

class Scheduler {
 public:
  void AddTask(std::shared_ptr<Task> task);

  void Run();  // 主循环

 private:
  std::vector<std::shared_ptr<Task>> m_tasks;

  std::shared_ptr<Task> PickNextTask();
};

}  // namespace OneTaskCore