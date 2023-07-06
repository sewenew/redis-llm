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

#ifndef SEWENEW_REDIS_LLM_CREATE_CHAT_COMMAND_H
#define SEWENEW_REDIS_LLM_CREATE_CHAT_COMMAND_H

#include "nlohmann/json.hpp"
#include "sw/redis-llm/create_app_command.h"

namespace sw::redis::llm {

// LLM.CREATE-CHAT key [--NX] [--XX] --LLM llm-info --HISTORY '{}' --PROMPT ''
class CreateChatCommand : public CreateAppCommand {
public:
    CreateChatCommand() : CreateAppCommand("chat") {}

private:
    virtual Args _parse_args(RedisModuleString **argv, int argc) const override;
};

}

#endif // end SEWENEW_REDIS_LLM_CREATE_CHAT_COMMAND_H
