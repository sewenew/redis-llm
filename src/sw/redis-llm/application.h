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

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "sw/redis-llm/llm_model.h"
#include "sw/redis-llm/embedding_model.h"
#include "sw/redis-llm/vector_store.h"

namespace sw::redis::llm {

class Application {
public:
    Application(const nlohmann::json &llm_config,
            const nlohmann::json &embedding_config,
            const nlohmann::json &vector_store_config);

    Application(const nlohmann::json &llm_config,
            const nlohmann::json &embedding_config,
            const nlohmann::json &vector_store_config,
            std::unordered_map<uint64_t, std::pair<std::string, Vector>> data_store);

    std::string ask(const std::string_view &question, bool without_private_data);

    std::string chat(uint64_t chat_id, const std::string_view &msg);

    void add(uint64_t id, const std::string_view &data);

    void add(uint64_t id, const std::string_view &data, const Vector &embedding);

    bool rem(uint64_t id);

    std::optional<std::string> get(uint64_t id);

    std::optional<Vector> embedding(uint64_t id);

    nlohmann::json conf() const;

    const std::unordered_map<uint64_t, std::string>& data_store() const {
        return _data_store;
    }

    VectorStore& vector_store() {
        return *_vector_store;
    }

    std::size_t dim() const {
        return _vector_store->dim();
    }

private:
    std::string _ask(const std::string_view &question);

    std::vector<std::string> _search_private_data(const std::string_view &question);

    std::string _build_llm_input(const std::string_view &question, const std::vector<std::string> &context);

    std::unique_ptr<LlmModel> _llm;

    std::unique_ptr<EmbeddingModel> _embedding;

    std::unique_ptr<VectorStore> _vector_store;

    std::unordered_map<uint64_t, std::string> _data_store;
};

}

#endif // end SEWENEW_REDIS_LLM_APPLICATION_H
