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

#include "sw/redis-llm/application.h"
#include <cassert>
#include "sw/redis-llm/errors.h"
#include "sw/redis-llm/simple_application.h"
#include "sw/redis-llm/search_application.h"

namespace sw::redis::llm {

Application::Application(const std::string &type,
        const LlmInfo &llm, const nlohmann::json &conf) :
    _type(type), _llm(llm), _conf(conf) {}

ApplicationFactory::ApplicationFactory() {
    _register("app", std::make_unique<ApplicationCreatorTpl<SimpleApplication>>());
    _register("search", std::make_unique<ApplicationCreatorTpl<SearchApplication>>());
}

ApplicationSPtr ApplicationFactory::create(const std::string &type,
        const LlmInfo &llm, const nlohmann::json &conf) const {
    auto iter = _creators.find(type);
    if (iter == _creators.end()) {
        throw Error(std::string("unknown application model: ") + type);
    }

    return iter->second->create(llm, conf);
}

void ApplicationFactory::_register(const std::string &type, ApplicationCreatorUPtr creator) {
    if (!_creators.emplace(type, std::move(creator)).second) {
        throw Error("duplicate application creators");
    }
}

}
