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

#include "create_vector_store_command.h"
#include "sw/redis-llm/errors.h"
#include "sw/redis-llm/redis_llm.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

void CreateVectorStoreCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    auto &llm = RedisLlm::instance();
    auto store = llm.vector_store_factory().create(args.type, args.params, args.llm);
    if (RedisModule_ModuleTypeSetValue(&_key, llm.vector_store_type(), store.get()) != REDISMODULE_OK) {
        throw Error("failed to create vector store");
    }

    store.release();
}

CreateVectorStoreCommand::Args CreateVectorStoreCommand::_parse_args(RedisModuleString **argv, int argc) const {
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
        } else if (util::str_case_equal(opt, "--LLM")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.llm = util::to_string(argv[idx]);
        } else {
            break;
        }
    }

    if (args.type.empty()) {
        args.type = "hnsw";
    }

    return args;
}

nlohmann::json CreateVectorStoreCommand::_parse_params(const std::string_view &opt) const {
    nlohmann::json params;
    try {
        params = nlohmann::json::parse(opt.begin(), opt.end());
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse LLM parameters") + e.what());
    }

    return params;
}

}
