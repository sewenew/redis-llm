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

#ifndef SEWENEW_REDIS_LLM_HNSW_H
#define SEWENEW_REDIS_LLM_HNSW_H

#include "sw/redis-llm/vector_store.h"
#include <hnswlib/hnswlib.h>

namespace sw::redis::llm {

class Hnsw : public VectorStore {
public:
    Hnsw(const nlohmann::json &conf, const LlmInfo &llm);

private:
    virtual void _add(uint64_t id, const Vector &embedding) override;

    // @return false, if data does not exist. true, otherwise.
    virtual void _rem(uint64_t id) override;

    virtual std::optional<Vector> _get(uint64_t id) override;

    virtual std::vector<std::pair<uint64_t, float>> _knn(const Vector &query, std::size_t k) override;

    virtual void _lazily_init(std::size_t dim) override;

    struct Options {
        std::size_t max_elements = 10000;
        std::size_t m = 16;
        std::size_t ef_construction = 200;
    };

    Options _parse_options(const nlohmann::json &conf) const;

    Options _opts;

    std::unique_ptr<hnswlib::SpaceInterface<float>> _space;
    std::unique_ptr<hnswlib::HierarchicalNSW<float>> _hnsw;
};

}

#endif // end SEWENEW_REDIS_LLM_HNSW_H
