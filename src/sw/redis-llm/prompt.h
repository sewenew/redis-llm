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

#ifndef SEWENEW_REDIS_LLM_PROMPT_H
#define SEWENEW_REDIS_LLM_PROMPT_H

#include <string>
#include <string_view>
#include "inja/inja.hpp"
#include "nlohmann/json.hpp"

namespace sw::redis::llm {

class Prompt {
public:
    explicit Prompt(const std::string_view &tpl);

    std::string render(const nlohmann::json &data = nlohmann::json{}) const;

private:
    inja::Template _tpl;
};

}

#endif // end SEWENEW_REDIS_LLM_PROMPT_H
