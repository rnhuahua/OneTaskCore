// /demo/main.cpp
#include <chrono>
#include <iostream>
#include <thread>

// 引入总头文件
#include "OneTaskCore.h"

using namespace OneTaskCore;

// ==========================================
// 演示任务 1：单次耗时任务 (例如：初始化/网络请求)
// ==========================================
class NetworkRequestTask : public Task {
 public:
  NetworkRequestTask(uint32_t id, uint8_t priority) : Task(id, priority) {}

  bool Init() override {
    std::cout << "[Task " << GetId() << "] NetworkRequestTask 初始化成功。\n";
    return true;
  }

  TaskResult Execute() override {
    std::cout << "[Task " << GetId() << "] 正在发送网络请求...\n";
    // 模拟网络延迟 120ms
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    std::cout << "[Task " << GetId() << "] 网络请求完成！\n";

    return TaskResult::SUCCESS;  // 执行完成 [cite: 2]
  }
};

// ==========================================
// 演示任务 2：周期性任务 (例如：心跳包/传感器读取)
// ==========================================
class HeartbeatTask : public Task {
 public:
  HeartbeatTask(uint32_t id, uint8_t priority) : Task(id, priority) {
    SetPeriod(500);  // 设置为每 500ms 执行一次
  }

  bool Init() override {
    std::cout << "[Task " << GetId()
              << "] HeartbeatTask 初始化成功，周期: 500ms。\n";
    return true;
  }

  TaskResult Execute() override {
    m_ticks++;
    std::cout << "[Task " << GetId() << "] 噗通... (心跳次数: " << m_ticks
              << ")\n";

    // 模拟极短暂的处理耗时 5ms
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    // 为了让 Demo 能够自然结束，我们在心跳 5 次后主动取消任务
    if (m_ticks >= 5) {
      std::cout << "[Task " << GetId()
                << "] 达到最大心跳次数，准备取消任务。\n";
      Cancel();  // 取消任务
    }

    return TaskResult::SUCCESS;
  }

 private:
  int m_ticks = 0;
};

// ==========================================
// 主函数
// ==========================================
int main() {
  std::cout << "--- OneTaskCore Demo 启动 ---\n\n";

  Scheduler scheduler;

  // 1. 创建任务实例 (ID, 优先级)
  // ID=101, 优先级=10 (普通优先级)
  auto net_task = std::make_shared<NetworkRequestTask>(101, 10);
  // ID=201, 优先级=5 (高优先级，数值越小优先级越高)
  auto hb_task = std::make_shared<HeartbeatTask>(201, 5);

  // 2. 将任务添加到调度器 [cite: 16]
  scheduler.AddTask(net_task);
  scheduler.AddTask(hb_task);

  // 3. 启动主循环
  std::cout << "\n开始调度循环...\n";
  scheduler.Run();  // 调度器将一直运行，直到所有任务都被清理 (m_tasks 为空)

  std::cout << "\n调度循环已结束 (所有任务均已销毁)。\n";

  // 4. 打印监控器收集到的性能报告
  scheduler.GetMonitor().PrintAll();

  std::cout << "--- OneTaskCore Demo 结束 ---\n";
  return 0;
}