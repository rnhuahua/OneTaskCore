// /src/scheduler/scheduler.cpp
#include "scheduler/scheduler.h"

#include <algorithm>
#include <thread>

namespace OneTaskCore
{

void Scheduler::AddTask(std::shared_ptr<Task> task)
{
    if (!task)
        return;

    if (task->Init())
    {
        task->SetState(TaskState::READY);
        m_tasks.push_back(task);
    }
    else
    {
        task->SetState(TaskState::ERROR);
    }
}

// ========================
// 僵尸任务清理 (内存回收)
// ========================
void Scheduler::CleanupTasks()
{
    std::lock_guard<std::mutex> lock(mtx);
    m_tasks.erase(std::remove_if(m_tasks.begin(), m_tasks.end(),
                                 [](const std::shared_ptr<Task>& t)
                                 {
                                     TaskState state = t->GetState();
                                     return (state == TaskState::FINISHED || state == TaskState::ERROR) &&
                                            !t->IsRunning();
                                 }),
                  m_tasks.end());
}

// ========================
// 核心调度循环
// ========================
void Scheduler::Run()
{
    while (true)
    {
        // 1. 每轮循环开始前，清理掉已经结束的任务
        CleanupTasks();

        // 2. 如果任务队列彻底空了，才真正退出调度器
        if (m_tasks.empty())
        {
            break;
        }

        auto task = PickNextTask();

        // 3. 提早停机与 CPU 空转修复
        if (!task)
        {
            uint64_t now = NowMs();
            uint64_t next_wake_time = UINT64_MAX;

            // 寻找距离现在最近的下一个任务的执行时间
            for (const auto& t : m_tasks)
            {
                if (t->GetState() == TaskState::READY && t->GetNextRunTime() < next_wake_time)
                {
                    next_wake_time = t->GetNextRunTime();
                }
            }

            if (next_wake_time != UINT64_MAX && next_wake_time > now)
            {
                // 精准休眠到下一个任务的触发时间
                std::this_thread::sleep_for(std::chrono::milliseconds(next_wake_time - now));
            }
            else
            {
                // 兜底休眠：防止状态异常导致的 CPU 100% 占用
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            continue;  // 醒来后重新挑选任务
        }
        if (task->GetState() != TaskState::READY)
            continue;

        m_pool.Enqueue(
            [this, task]()
            {
                uint64_t start = NowMs();

                TaskResult result = task->Execute();

                uint64_t end = NowMs();

                m_monitor.Record(task->GetId(), end - start);

                // ⚠️ 状态修改要小心（共享数据）
                uint64_t now = NowMs();

                if (task->IsCancelled())
                {
                    task->SetState(TaskState::FINISHED);
                    return;
                }

                switch (result)
                {
                    case TaskResult::SUCCESS:
                        if (task->IsPeriodic())
                        {
                            task->SetNextRunTime(now + task->GetPeriod());
                            task->SetState(TaskState::READY);
                        }
                        else
                        {
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
            });
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// ========================
// 选择最高优先级任务
// ========================
std::shared_ptr<Task> Scheduler::PickNextTask()
{
    uint64_t now = NowMs();
    std::shared_ptr<Task> best = nullptr;

    for (auto& task : m_tasks)
    {
        if (task->GetState() != TaskState::READY)
            continue;
        if (task->GetNextRunTime() > now)
            continue;

        if (!best)
        {
            best = task;
            continue;
        }

        // 时间优先 + 优先级次之
        if (task->GetNextRunTime() < best->GetNextRunTime() ||
            (task->GetNextRunTime() == best->GetNextRunTime() && task->GetPriority() > best->GetPriority()))
        {
            best = task;
        }
    }
    if (best)
    {
        best->SetState(TaskState::RUNNING);
    }
    return best;
}

}  // namespace OneTaskCore