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

#ifndef SEWENEW_REDIS_LLM_KNN_COMMAND_H
#define SEWENEW_REDIS_LLM_KNN_COMMAND_H

#include <chrono>
#include <exception>
#include "sw/redis-llm/command.h"
#include "sw/redis-llm/llm_model.h"
#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/vector_store.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

// LLM.KNN key [--K 10] [--embedding xxx] [query]
// This command works with VECTOR STORE
class KnnCommand : public Command {
private:
    virtual void _run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const override;

    struct Args {
        RedisModuleString *key_name = nullptr;

        std::string_view query;

        std::size_t k = 10;

        std::chrono::milliseconds timeout{0};

        Vector embedding;
    };

    struct AsyncResult {
        std::vector<std::pair<uint64_t, float>> neighbors;
        std::exception_ptr err;
    };

    Args _parse_args(RedisModuleString **argv, int argc) const;

    void _knn(RedisModuleBlockedClient *blocked_client,
        const Args &args, const VectorStoreSPtr &store, const LlmModelSPtr &model) const;

    static int _reply_func(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

    static int _timeout_func(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

    static void _free_func(RedisModuleCtx *ctx, void *privdata);
};

}

#endif // end SEWENEW_REDIS_LLM_KNN_COMMAND_H
