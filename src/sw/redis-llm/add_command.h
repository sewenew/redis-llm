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

#ifndef SEWENEW_REDIS_LLM_ADD_COMMAND_H
#define SEWENEW_REDIS_LLM_ADD_COMMAND_H

#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/command.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

// LLM.ADD key [--ID id] [--EMBEDDING xxx] data
// This command works with VECTOR STORE
class AddCommand : public Command {
private:
    virtual void _run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const override;

    struct Args {
        RedisModuleString *key_name = nullptr;

        std::optional<uint64_t> id;

        std::string_view data;

        Vector embedding;
    };

    Args _parse_args(RedisModuleString **argv, int argc) const;

    Vector _get_embedding(RedisModuleCtx *ctx, const std::string_view &data, const std::string &llm_key) const;

    uint64_t _add(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const;
};

}

#endif // end SEWENEW_REDIS_LLM_ADD_COMMAND_H
