/**************************************************************************
   Copyright (c) 2023 sewenew

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 *************************************************************************/

#ifndef SEWENEW_REDIS_LLM_WORKER_POOL_H
#define SEWENEW_REDIS_LLM_WORKER_POOL_H

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include "sw/redis-llm/errors.h"

namespace sw::redis::llm {

struct WorkerPoolOptions {
    std::size_t pool_size = 10;

    std::size_t queue_size = 1000;
};

class WorkerPool {
public:
    explicit WorkerPool(const WorkerPoolOptions &opts);

    WorkerPool(const WorkerPool &) = delete;
    WorkerPool& operator=(const WorkerPool &) = delete;

    WorkerPool(WorkerPool &&) = delete;
    WorkerPool& operator=(WorkerPool &&) = delete;

    ~WorkerPool();

    template <typename Func, typename ...Args>
    auto enqueue(Func &&func, Args &&...args)
        -> std::future<typename std::invoke_result_t<Func, Args...>> {
        std::packaged_task<std::invoke_result_t<Func, Args...> ()> task(
                std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        auto result = task.get_future();

        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (_tasks.size() == _opts.queue_size) {
                throw Error("worker queue is full");
            }

            _tasks.emplace(std::move(task));
        }

        _cv.notify_one();

        return result;
    }

private:
    void _stop();

    void _run();

    WorkerPoolOptions _opts;

    std::queue<std::packaged_task<void ()>> _tasks;

    bool _quit;

    std::mutex _mutex;

    std::condition_variable _cv;

    std::vector<std::thread> _workers;
};

}

#endif // end SEWENEW_REDIS_LLM_WORKER_POOL_H
