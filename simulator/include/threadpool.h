#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

namespace simulator {
class ThreadPool {
   public:
    explicit ThreadPool(size_t threadCount) : stop(false) {
        for (size_t i = 0; i < threadCount; ++i) {
            threads.push_back(std::thread(&ThreadPool::workerThread, this));
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (auto& thread : threads) {
            thread.join();
        }
    }

    template <typename FunctionType, typename... Args>
    auto submit(FunctionType&& f, Args&&... args)
        -> std::future<typename std::result_of<FunctionType(Args...)>::type> {
        using ReturnType = typename std::result_of<FunctionType(Args...)>::type;

        auto task =
            std::make_shared<std::packaged_task<ReturnType()>>(std::bind(
                std::forward<FunctionType>(f), std::forward<Args>(args)...));

        std::future<ReturnType> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop) {
                throw std::runtime_error("submit on stopped ThreadPool");
            }
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return result;
    }

   private:
    void workerThread() {
        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]() { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) {
                return;
            }
            auto task = std::move(tasks.front());
            tasks.pop();
            lock.unlock();

            task();
        }
    }

    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

}  // namespace simulator

#endif  // THREADPOOL_H_