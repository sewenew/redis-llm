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

#ifndef SEWENEW_REDIS_LLM_OPTIONS_H
#define SEWENEW_REDIS_LLM_OPTIONS_H

#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/worker_pool.h"
#include <string>

namespace sw::redis::llm {

struct Options {
    void load(RedisModuleString **argv, int argc);

    WorkerPoolOptions worker_pool_opts;
};

}

#endif // end SEWENEW_REDIS_LLM_OPTIONS_H
