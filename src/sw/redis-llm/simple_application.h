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

#ifndef SEWENEW_REDIS_LLM_SIMPLE_APPLICATION_H
#define SEWENEW_REDIS_LLM_SIMPLE_APPLICATION_H

#include "nlohmann/json.hpp"
#include "sw/redis-llm/application.h"
#include "sw/redis-llm/prompt.h"

namespace sw::redis::llm {

class SimpleApplication : public Application {
public:
    SimpleApplication(const nlohmann::json &llm, const nlohmann::json &conf);

    virtual std::string run(LlmModel &llm, const nlohmann::json &context, const std::string_view &input, bool verbose) override;

private:
    Prompt _prompt;
};

}

#endif // end SEWENEW_REDIS_LLM_SIMPLE_APPLICATION_H
