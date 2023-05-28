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

#ifndef SEWENEW_REDIS_LLM_CREATE_COMMAND_H
#define SEWENEW_REDIS_LLM_CREATE_COMMAND_H

#include "nlohmann/json.hpp"
#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/command.h"

namespace sw::redis::llm {

class CreateCommand : public Command {
private:
    virtual void _run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const override;

    struct Args {
        enum class Opt {
            NX = 0,
            XX,
            NONE
        };

        Opt opt = Opt::NONE;

        RedisModuleString *key_name = nullptr;

        nlohmann::json llm_config;

        nlohmann::json embedding_config = nlohmann::json::object();

        nlohmann::json vector_store_config = nlohmann::json::object();
    };

    Args _parse_args(RedisModuleString **argv, int argc) const;

    nlohmann::json _parse_config(RedisModuleString *str) const;

    int _create(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const;
};

}

#endif // end SEWENEW_REDIS_LLM_CREATE_COMMAND_H
