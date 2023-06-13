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

#ifndef SEWENEW_REDIS_LLM_SEARCH_APPLICATION_H
#define SEWENEW_REDIS_LLM_SEARCH_APPLICATION_H

#include <string>
#include "nlohmann/json.hpp"
#include "sw/redis-llm/application.h"
#include "sw/redis-llm/prompt.h"
#include "sw/redis-llm/vector_store.h"

namespace sw::redis::llm {

class SearchApplication : public Application {
public:
    SearchApplication(const LlmInfo &llm, const nlohmann::json &conf);

    virtual std::string run(RedisModuleCtx *ctx, LlmModel &llm, const nlohmann::json &context, const std::string_view &input, bool verbose) override;

private:
    VectorStore& _get_vector_store(RedisModuleCtx *ctx, const nlohmann::json &context);

    std::vector<std::string> _get_similar_items(const Vector &embedding, VectorStore &store);

    Prompt _prompt;

    std::string _vector_store;

    std::size_t _k;
};

}

#endif // end SEWENEW_REDIS_LLM_SEARCH_APPLICATION_H
