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
#include "sw/redis-llm/http_client.h"
#include "sw/redis-llm/llm_model.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

namespace sw::redis::llm {

class OpenAi : public LlmModel {
public:
    explicit OpenAi(const nlohmann::json &conf);

    virtual std::vector<float> embedding(const std::string_view &input,
            const nlohmann::json &params = nlohmann::json::object()) override;

    virtual std::string predict(const std::string_view &input,
            const nlohmann::json &params = nlohmann::json::object()) override;

    virtual std::string chat(const std::string_view &input,
            const nlohmann::json &params = nlohmann::json::object()) override;

private:
    struct Options {
        std::string api_key;

        std::string uri;

        nlohmann::json chat;

        std::string chat_path;

        nlohmann::json embedding;

        std::string embedding_path;

        HttpClientOptions http_opts;

        HttpClientPoolOptions http_pool_opts;
    };

    Options _parse_options(const nlohmann::json &conf) const;

    auto _parse_http_options(const nlohmann::json &conf) const
        -> std::pair<HttpClientOptions, HttpClientPoolOptions>;

    nlohmann::json _construct_msg(const std::string_view &input) const;

    nlohmann::json _query(const std::string &path, const nlohmann::json &input);

    Options _opts;

    HttpClientPool _client_pool;
};

}

#endif // end SEWENEW_REDIS_LLM_OPENAI_H
