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

#include "sw/redis-llm/module_entry.h"
#include <cassert>
#include "sw/redis-llm/errors.h"
#include "sw/redis-llm/redis_llm.h"
#include "sw/redis-llm/module_api.h"

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    assert(ctx != nullptr);

    using namespace sw::redis::llm;

    try {
        auto &m = RedisLlm::instance();

        m.load(ctx, argv, argc);
    } catch (const Error &e) {
        api::warning(ctx, "%s", e.what());
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}
