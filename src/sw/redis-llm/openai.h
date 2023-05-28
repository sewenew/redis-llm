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

#ifndef SEWENEW_REDIS_LLM_OPENAI_H
#define SEWENEW_REDIS_LLM_OPENAI_H

#include "nlohmann/json.hpp"
#include "sw/redis-llm/llm_model.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

namespace sw::redis::llm {

class OpenAi : public LlmModel {
public:
    explicit OpenAi(const nlohmann::json &conf);

    virtual std::vector<float> embedding(const std::string_view &input) override;

    virtual std::string predict(const std::string_view &input) override;

private:
    struct Options {
        std::string api_key;

        std::string model;
    };

    Options _parse_options(const nlohmann::json &conf) const;

    std::unique_ptr<httplib::Client> _make_client(const Options &opts) const;

    Options _opts;

    std::unique_ptr<httplib::Client> _cli;
};

}

#endif // end SEWENEW_REDIS_LLM_OPENAI_H
