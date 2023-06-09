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

#ifndef SEWENEW_REDIS_LLM_OPENAI_EMBEDDING_H
#define SEWENEW_REDIS_LLM_OPENAI_EMBEDDING_H

#include "nlohmann/json.hpp"
#include "sw/redis-llm/embedding_model.h"
#include "sw/redis-llm/openai.h"

namespace sw::redis::llm {

class OpenAiEmbedding : public EmbeddingModel {
public:
    OpenAiEmbedding(const nlohmann::json &conf) : EmbeddingModel("openai", conf), _open_ai(conf) {}

    virtual std::vector<float> embedding(const std::string_view &input) override;

private:
    OpenAi _open_ai;
};

}

#endif // end SEWENEW_REDIS_LLM_OPENAI_EMBEDDING_H
