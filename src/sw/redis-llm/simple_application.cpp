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

#include "sw/redis-llm/simple_application.h"

namespace sw::redis::llm {

SimpleApplication::SimpleApplication(const LlmInfo &llm,
        const nlohmann::json &conf) :
    Application("app", llm, conf),
    _prompt(conf.value<std::string>("prompt", "")) {}

std::string SimpleApplication::run(RedisModuleCtx *ctx, LlmModel &model, const nlohmann::json &context, const std::string_view &input, bool verbose) {
    nlohmann::json vars;
    if (!context.is_null()) {
        vars = context.value<nlohmann::json>("vars", nlohmann::json::object());
    }
    auto request = _prompt.render(vars);

    if (!request.empty()) {
        request += "\n\n";
    }
    request += input;

    std::string output;
    if (verbose) {
        output += request;
        output += "\n\n";
    }

    output += model.predict(request, llm().params);

    return output;
}

}
