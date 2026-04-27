// scheduler.cpp
#include "scheduler/scheduler.h"

#include <algorithm>

namespace OneTaskCore {

void Scheduler::AddTask(std::shared_ptr<Task> task) {
  if (!task) return;

  if (task->Init()) {
    task->SetState(TaskState::READY);
    m_tasks.push_back(task);
  } else {
    task->SetState(TaskState::ERROR);
  }
}

// ========================
// 核心调度循环
// ========================
void Scheduler::Run() {
  while (true) {
    auto task = PickNextTask();
    if (!task) {
      break;  // 没任务了
    }

    task->SetState(TaskState::RUNNING);

    // 执行任务
    TaskResult result = task->Execute();

    uint64_t now = NowMs();

    if (task->IsCancelled()) {
      task->SetState(TaskState::FINISHED);
      continue;
    }

    switch (result) {
      case TaskResult::SUCCESS:
        if (task->IsPeriodic()) {
          // 周期任务 → 设置下一次执行时间
          task->SetNextRunTime(now + task->GetPeriod());
          task->SetState(TaskState::READY);
        } else {
          task->SetState(TaskState::FINISHED);
        }
        break;

      case TaskResult::RETRY:
        task->SetState(TaskState::READY);
        break;

      case TaskResult::FAILED:
        task->SetState(TaskState::ERROR);
        break;
    }
  }
}

// ========================
// 选择最高优先级任务
// ========================
std::shared_ptr<Task> Scheduler::PickNextTask() {
  uint64_t now = NowMs();
  std::shared_ptr<Task> best = nullptr;

  for (auto& task : m_tasks) {
    // 只调度 READY
    if (task->GetState() != TaskState::READY) continue;

    // 时间还没到，不能执行
    if (task->GetNextRunTime() > now) continue;

    // 优先级选择
    if (!best || task->GetPriority() < best->GetPriority()) {
      best = task;
    }
  }

  return best;
}

}  // namespace OneTaskCore