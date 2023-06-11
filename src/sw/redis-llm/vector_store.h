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
#include <atomic>
#include <optional>
#include <unordered_map>
#include <utility>
#include "nlohmann/json.hpp"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

class VectorStore {
public:
    VectorStore(const std::string &type, const nlohmann::json &conf, const LlmInfo &llm) :
        _type(type), _conf(conf), _llm(llm) {}

    virtual ~VectorStore() = default;

    virtual void add(uint64_t id, const std::string_view &data, const Vector &embedding) = 0;

    void add(const std::string_view &data, const Vector &embedding);

    // @return false, if data does not exist. true, otherwise.
    virtual bool rem(uint64_t id) = 0;

    virtual std::optional<Vector> get(uint64_t id) = 0;

    virtual std::optional<std::string> data(uint64_t id) = 0;

    virtual std::vector<std::pair<uint64_t, float>> knn(const Vector &query, std::size_t k) = 0;

    const std::string& type() const {
        return _type;
    }

    const nlohmann::json& conf() const {
        return _conf;
    }

    const LlmInfo& llm() const {
        return _llm;
    }

    std::size_t dim() const;

    const std::unordered_map<uint64_t, std::string>& data_store() const {
        return _data_store;
    }

    uint64_t id_idx() const {
        return _id_idx;
    }

protected:
    std::unordered_map<uint64_t, std::string> _data_store;

private:
    uint64_t _auto_gen_id();

    std::string _type;

    nlohmann::json _conf;

    LlmInfo _llm;

    std::atomic<uint64_t> _id_idx{0};
};

using VectorStoreUPtr = std::unique_ptr<VectorStore>;

class VectorStoreCreator {
public:
    virtual ~VectorStoreCreator() = default;

    virtual VectorStoreUPtr create(const nlohmann::json &conf, const LlmInfo &llm) const = 0;
};

using VectorStoreCreatorUPtr = std::unique_ptr<VectorStoreCreator>;

template <typename T>
class VectorStoreCreatorTpl : public VectorStoreCreator {
public:
    virtual VectorStoreUPtr create(const nlohmann::json &conf, const LlmInfo &llm) const override {
        return std::make_unique<T>(conf, llm);
    }
};

class VectorStoreFactory {
public:
    VectorStoreFactory();

    VectorStoreUPtr create(const std::string &type, const nlohmann::json &conf, const LlmInfo &llm) const;

private:
    void _register(const std::string &type, VectorStoreCreatorUPtr creator);

    std::unordered_map<std::string, VectorStoreCreatorUPtr> _creators;
};

}

#endif // end SEWENEW_REDIS_LLM_VECTOR_STORE_H
