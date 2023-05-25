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

#include "sw/redis-llm/openai.h"

namespace sw::redis::llm {

OpenAi::OpenAi(const nlohmann::json &conf) : _opts(_parse_args(args)) {}

std::vector<float> OpenAi::embedding(const std::string_view &/*input*/) {
    return {};
}

std::string OpenAi::predict(const std::string &input) {
    return input;
}

OpenAi::Options OpenAi::_parse_options(const nlohmann::json &conf) const {
    Options opts;
    try {
        opts.api_key = conf.at("api_key").get<std::string>();
        opts.model = conf.at("model").get<std::string>();
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse openai options: ") + e.what());
    }

    return _opts;
}

}
