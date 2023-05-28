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

#ifndef SEWENEW_REDIS_LLM_GET_COMMAND_H
#define SEWENEW_REDIS_LLM_GET_COMMAND_H

#include <optional>
#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/command.h"

namespace sw::redis::llm {

class GetCommand : public Command {
private:
    virtual void _run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const override;

    struct Args {
        RedisModuleString *key_name = nullptr;

        uint64_t id;
    };

    Args _parse_args(RedisModuleString **argv, int argc) const;

    auto _get(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const ->
        std::optional<std::pair<std::string, std::string>>;
};

}

#endif // end SEWENEW_REDIS_LLM_GET_COMMAND_H
