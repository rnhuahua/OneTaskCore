#include "OneTaskCore/threadpool/thread_pool.h"
ThreadPool::ThreadPool(size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        workers.emplace_back(
            [this]()
            {
                while (true)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this]() { return stop || !tasks.empty(); });

                        if (stop && tasks.empty())
                            return;

                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    task();  // 执行任务
                }
            });
    }
}

void ThreadPool::Enqueue(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.push(std::move(task));
    }
    cv.notify_one();
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();

    for (auto& t : workers) t.join();
}