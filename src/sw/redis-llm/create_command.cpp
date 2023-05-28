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

#include "sw/redis-llm/create_command.h"
#include "sw/redis-llm/application.h"
#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/redis_llm.h"
#include "sw/redis-llm/utils.h"
#include "sw/redis-llm/vector_store.h"

namespace sw::redis::llm {

void CreateCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto res = _create(ctx, argv, argc);

    RedisModule_ReplyWithLongLong(ctx, res);

    RedisModule_ReplicateVerbatim(ctx);
}

int CreateCommand::_create(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    auto key = api::open_key(ctx, args.key_name, api::KeyMode::WRITEONLY);
    assert(key);

    auto &llm = RedisLlm::instance();
    if (api::key_exists(key.get(), llm.type())) {
        if (args.opt == Args::Opt::NX) {
            return 0;
        }
    } else {
        if (args.opt == Args::Opt::XX) {
            return 0;
        }
    }

    auto app = std::make_unique<Application>(args.llm_config,
                args.embedding_config,
                args.vector_store_config);
    if (RedisModule_ModuleTypeSetValue(key.get(), llm.type(), app.get()) != REDISMODULE_OK) {
        throw Error("failed to create LLM application");
    }
    app.release();

    return 1;
}

CreateCommand::Args CreateCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 4) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    auto idx = 2;
    while (idx < argc) {
        auto opt = util::to_sv(argv[idx]);
        if (util::str_case_equal(opt, "--NX")) {
            if (args.opt != Args::Opt::NONE) {
                throw Error("syntax error");
            }

            args.opt = Args::Opt::NX;
        } else if (util::str_case_equal(opt, "--XX")) {
            if (args.opt != Args::Opt::NONE) {
                throw Error("syntax error");
            }

            args.opt = Args::Opt::XX;
        } else if (util::str_case_equal(opt, "--LLM")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.llm_config = _parse_config(argv[idx]);
        } else if (util::str_case_equal(opt, "--EMBEDDING")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.embedding_config = _parse_config(argv[idx]);
        } else if (util::str_case_equal(opt, "--VECTOR_STORE")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.vector_store_config = _parse_config(argv[idx]);
        } else {
            break;
        }

        ++idx;
    }

    if (args.llm_config.is_null()) {
        throw Error("must specify LLM config");
    }

    return args;
}

nlohmann::json CreateCommand::_parse_config(RedisModuleString *str) const {
    auto config = util::to_sv(str);

    nlohmann::json conf;
    try {
        conf = nlohmann::json::parse(config.begin(), config.end());
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse json config: ") + e.what());
    }

    return conf;
}

}
