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

#include "sw/redis-llm/worker_pool.h"
#include <cassert>

namespace sw::redis::llm {

WorkerPool::WorkerPool(const WorkerPoolOptions &opts) : _opts(opts), _quit(false) {
    for (auto idx = 0U; idx != opts.pool_size; ++idx) {
        _workers.emplace_back(std::thread([this]() { this->_run(); }));
    }
}

WorkerPool::~WorkerPool() {
    _stop();

    for (auto &worker : _workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void WorkerPool::_run() {
    while (true) {
        std::packaged_task<void ()> task;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait(lock, [this]() { return this->_quit || !this->_tasks.empty(); });

            if (_tasks.empty()) {
                assert(_quit);
                break;
            }

            task = std::move(_tasks.front());
            _tasks.pop();
        }

        task();
    }
}

void WorkerPool::_stop() {
    {
        std::lock_guard<std::mutex> lock(_mutex);

        _quit = true;
    }

    _cv.notify_all();
}

}
