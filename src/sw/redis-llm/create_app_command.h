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

#ifndef SEWENEW_REDIS_LLM_CREATE_APP_COMMAND_H
#define SEWENEW_REDIS_LLM_CREATE_APP_COMMAND_H

#include "nlohmann/json.hpp"
#include "sw/redis-llm/command.h"
#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

// LLM.CREATE-APP key [--NX] [--XX] --LLM llm-info [--PARAMS '{}'] [--PROMPT prompt]
class CreateAppCommand : public Command {
public:
    explicit CreateAppCommand(const std::string &type) : _type(type) {}

    CreateAppCommand() : CreateAppCommand("app") {}

    const std::string& type() const {
        return _type;
    }

protected:
    struct Args {
        RedisModuleString *key_name = nullptr;

        LlmInfo llm;

        api::CreateOption opt = api::CreateOption::NONE;

        nlohmann::json params = nlohmann::json::object();
    };

private:
    virtual void _run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const override;

    virtual Args _parse_args(RedisModuleString **argv, int argc) const;

    int _create(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const;

    std::string _type;
};

}

#endif // end SEWENEW_REDIS_LLM_CREATE_APP_COMMAND_H
