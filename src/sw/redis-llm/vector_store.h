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

#ifndef SEWENEW_REDIS_LLM_VECTOR_STORE_H
#define SEWENEW_REDIS_LLM_VECTOR_STORE_H

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>
#include "nlohmann/json.hpp"
#include <hnswlib/hnswlib.h>
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

class VectorStore {
public:
    explicit VectorStore(const nlohmann::json &conf);

    void add(uint64_t id, const std::vector<float> &data);

    void rem(uint64_t id);

    std::optional<Vector> get(uint64_t id);

    std::vector<std::pair<uint64_t, float>> knn(const std::vector<float> &query, std::size_t k);

    const nlohmann::json& conf() const {
        return _conf;
    }

    std::size_t dim() const {
        return _opts.dim;
    }

private:
    struct Options {
        std::size_t dim;
        std::size_t max_elements = 10000;
        std::size_t m = 16;
        std::size_t ef_construction = 200;
    };

    Options _parse_options(const nlohmann::json &conf) const;

    Options _opts;

    std::unique_ptr<hnswlib::SpaceInterface<float>> _space;
    std::unique_ptr<hnswlib::HierarchicalNSW<float>> _hnsw;

    nlohmann::json _conf;
};

}

#endif // end SEWENEW_REDIS_LLM_VECTOR_STORE_H
