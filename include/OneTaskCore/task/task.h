// /include/OneTaskCore/task/task.h
#pragma once
#include <chrono>
#include <cstdint>
#include <mutex>
namespace OneTaskCore
{

// =========================
// 任务状态
// =========================
enum class TaskState : uint8_t
{
    CREATED = 0,
    READY,
    RUNNING,
    BLOCKED,
    FINISHED,
    ERROR
};

// =========================
// 执行结果
// =========================
enum class TaskResult : uint8_t
{
    SUCCESS,  // 执行完成
    RETRY,    // 需要再次调度
    FAILED    // 执行失败
};

// =========================
// Task 基类
// =========================
class Task
{
   public:
    Task(uint32_t id, uint8_t priority = 0) : m_id(id), m_priority(priority), m_state(TaskState::CREATED) {}

    virtual ~Task() = default;

    // =========================
    // 生命周期接口
    // =========================

    // 初始化（只调用一次）
    virtual bool Init() = 0;

    // 核心执行（由调度器反复调用）
    virtual TaskResult Execute() = 0;

    // 取消任务
    virtual void Cancel()
    {
        m_cancelled = true;
    }

    // =========================
    // Getter\Setter
    // =========================
    uint32_t GetId() const
    {
        return m_id;
    }
    void SetId(uint32_t id)
    {
        m_id = id;
    }

    uint8_t GetPriority() const
    {
        return m_priority;
    }
    void SetPriority(uint8_t priority)
    {
        m_priority = priority;
    }

    TaskState GetState() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return m_state;
    }
    void SetState(TaskState state)
    {
        std::lock_guard<std::mutex> lock(mtx);
        m_state = state;
    }

    bool IsCancelled() const
    {
        return m_cancelled;
    }
    bool IsRunning()
    {
        return GetState() == TaskState::RUNNING;
    }
    uint64_t GetNextRunTime() const
    {
        return m_next_run_time;
    }
    void SetNextRunTime(uint64_t t)
    {
        m_next_run_time = t;
    }

    void SetPeriod(uint64_t period)
    {
        m_period = period;
    }
    uint64_t GetPeriod() const
    {
        return m_period;
    }

    bool IsPeriodic() const
    {
        return m_period > 0;
    }

   protected:
    uint32_t m_id;
    uint8_t m_priority;
    TaskState m_state;

    mutable std::mutex mtx;

    bool m_cancelled = false;

    // 时间调度相关
    // 下次执行时间（单位：ms）
    uint64_t m_next_run_time = 0;
    // 周期（0 表示非周期任务）
    uint64_t m_period = 0;
};

uint64_t NowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

}  // namespace OneTaskCore