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

#ifndef SEWENEW_REDIS_LLM_APPLICATION_H
#define SEWENEW_REDIS_LLM_APPLICATION_H

#include <memory>
#include "sw/redis-llm/llm_model.h"
#include "sw/redis-llm/embedding_model.h"
#include "sw/redis-llm/vector_store.h"

namespace sw::redis::llm {

class Application {
public:
    Application(std::unique_ptr<LlmModel> llm,
            std::unique_ptr<VectorStore> vector_store) :
        _llm(std::move(llm)),
        _vector_store(std::move(vector_store)) {}

    Application(std::unique_ptr<LlmModel> llm,
            std::unique_ptr<EmbeddingModel> embedding,
            std::unique_ptr<VectorStore> vector_store) :
        _llm(std::move(llm)),
        _embedding(std::move(embedding)),
        _vector_store(std::move(vector_store)) {}

    std::string ask(const std::string_view &question, bool without_private_data);

    std::string chat(uint64_t chat_id, const std::string_view &msg);

private:
    std::string _ask(const std::string_view &question);

    std::vector<std::string> _search_private_data(const std::string_view &question);

    std::string _build_llm_input(const std::string_view &question, const std::vector<std::string> &context);

    std::unique_ptr<LlmModel> _model;

    std::unique_ptr<EmbeddingModel> _embedding;

    std::unique_ptr<VectorStore> _vector_store;
};

}

#endif // end SEWENEW_REDIS_LLM_APPLICATION_H
