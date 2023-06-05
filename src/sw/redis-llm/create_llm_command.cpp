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

#include "create_llm_command.h"
#include "sw/redis-llm/errors.h"
#include "sw/redis-llm/redis_llm.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

void CreateLlmCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    auto &llm = RedisLlm::instance();
    auto model = llm.llm_factory().create(args.type, args.params);
    if (RedisModule_ModuleTypeSetValue(&_key, llm.llm_type(), model.get()) != REDISMODULE_OK) {
        throw Error("failed to create LLM model");
    }

    model.release();
}

CreateLlmCommand::Args CreateLlmCommand::_parse_args(RedisModuleString **argv, int argc) const {
    Args args;
    auto idx = 0;
    while (idx < argc) {
        auto opt = util::to_sv(argv[idx]);
        if (util::str_case_equal(opt, "--TYPE")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.type = util::to_sv(argv[idx]);
        } else if (util::str_case_equal(opt, "--PARAMS")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.params = _parse_params(util::to_sv(argv[idx]));
        } else {
            break;
        }

        ++idx;
    }

    return args;
}

nlohmann::json CreateLlmCommand::_parse_params(const std::string_view &opt) const {
    nlohmann::json params;
    try {
        params = nlohmann::json::parse(opt.begin(), opt.end());
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse LLM parameters") + e.what());
    }

    return params;
}

}
