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
#include <string>
#include <string_view>
#include <unordered_map>
#include "nlohmann/json.hpp"
#include "sw/redis-llm/llm_model.h"
#include "sw/redis-llm/redismodule.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

class Application {
public:
    Application(const std::string &type, const LlmInfo &llm, const nlohmann::json &conf);

    Application(const Application &) = delete;
    Application& operator=(const Application &) = delete;

    Application(Application &&) = delete;
    Application& operator=(Application &&) = delete;

    virtual ~Application() = default;

    virtual std::string run(RedisModuleCtx *ctx, LlmModel &llm, const nlohmann::json &context, const std::string_view &input, bool verbose) = 0;

    const std::string& type() const {
        return _type;
    }

    const LlmInfo& llm() const {
        return _llm;
    }

    const nlohmann::json& conf() const {
        return _conf;
    }

private:
    std::string _type;

    // {"key" : "xxx", "params" : {}}
    LlmInfo _llm;

    nlohmann::json _conf;
};

using ApplicationUPtr = std::unique_ptr<Application>;

class ApplicationCreator {
public:
    virtual ~ApplicationCreator() = default;

    virtual ApplicationUPtr create(const LlmInfo &llm, const nlohmann::json &conf) const = 0;
};

using ApplicationCreatorUPtr = std::unique_ptr<ApplicationCreator>;

template <typename T>
class ApplicationCreatorTpl : public ApplicationCreator {
public:
    virtual ApplicationUPtr create(const LlmInfo &llm, const nlohmann::json &conf) const override {
        return std::make_unique<T>(llm, conf);
    }
};

class ApplicationFactory {
public:
    ApplicationFactory();

    ApplicationUPtr create(const std::string &type,
            const LlmInfo &llm, const nlohmann::json &conf) const;

private:
    void _register(const std::string &type, ApplicationCreatorUPtr creator);

    std::unordered_map<std::string, ApplicationCreatorUPtr> _creators;
};

}

#endif // end SEWENEW_REDIS_LLM_APPLICATION_H
