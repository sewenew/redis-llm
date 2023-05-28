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

#include "sw/redis-llm/llama_cpp.h"
#include "sw/redis-llm/errors.h"

namespace sw::redis::llm {

LlamaCpp::LlamaCpp(const nlohmann::json &conf) :
    LlmModel(conf),
    _opts(_parse_options(conf)) {}

std::vector<float> LlamaCpp::embedding(const std::string_view &input) {
    return {};
}

std::string LlamaCpp::predict(const std::string_view &input) {
    return "";
}

LlamaCpp::Options LlamaCpp::_parse_options(const nlohmann::json &conf) const {
    Options opts;
    try {
        opts.sub_type = conf.at("sub_type").get<std::string>();
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse llama options: ") + e.what());
    }

    return opts;
}

}
