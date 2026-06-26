#include <odin/thread_pool.h>

#include <utility>

namespace odin {

ThreadPool::ThreadPool(std::size_t threads)
{
    if (threads == 0) {
        threads = 1;
    }

    workers_.reserve(threads);
    for (std::size_t i = 0; i < threads; ++i) {
        workers_.emplace_back([this]() { worker_loop(); });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }

    condition_.notify_all();

    for (auto &worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

std::size_t ThreadPool::size() const noexcept
{
    return workers_.size();
}

void ThreadPool::worker_loop()
{
    for (;;) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

            if (stop_ && tasks_.empty()) {
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
        }

        task();
    }
}

}  // namespace odin
