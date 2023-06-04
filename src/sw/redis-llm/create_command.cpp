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
//#include "sw/redis-llm/application.h"
#include "sw/redis-llm/create_llm_command.h"
#include "sw/redis-llm/create_vector_store_command.h"
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
    if (api::key_exists(key.get(), llm.llm_type())) {
        if (args.opt == Args::Opt::NX) {
            return 0;
        }
    } else {
        if (args.opt == Args::Opt::XX) {
            return 0;
        }
    }

    auto *sub_cmd_argv = argv + 3;
    auto sub_cmd_argc = argc - 3;
    if (args.opt != Args::Opt::NONE) {
        ++sub_cmd_argv;
        --sub_cmd_argc;
    }

    switch (args.sub_cmd) {
    case SubCmd::LLM: {
        CreateLlmCommand cmd(*key);
        cmd.run(ctx, sub_cmd_argv, sub_cmd_argc);
        break;
    }

    case SubCmd::VECTOR_STORE: {
        CreateVectorStoreCommand cmd(*key);
        cmd.run(ctx, sub_cmd_argv, sub_cmd_argc);
        break;
    }

    default:
        throw Error("unsupported sub command");
    }

    return 1;
}

bool CreateCommand::_parse_nx_xx_option(const std::string_view &opt, Args &args) const {
    if (util::str_case_equal(opt, "--NX")) {
        if (args.opt != Args::Opt::NONE) {
            throw Error("syntax error");
        }

        args.opt = Args::Opt::NX;

        return true;
    } else if (util::str_case_equal(opt, "--XX")) {
        if (args.opt != Args::Opt::NONE) {
            throw Error("syntax error");
        }

        args.opt = Args::Opt::XX;

        return true;
    } else {
        return false;
    }
}

CreateCommand::SubCmd CreateCommand::_parse_sub_cmd(const std::string_view &opt) const {
    if (util::str_case_equal(opt, "LLM")) {
        return SubCmd::LLM;
    } else if (util::str_case_equal(opt, "EMBEDDING")) {
        return SubCmd::EMBEDDING;
    } else if (util::str_case_equal(opt, "VECTOR_STORE")) {
        return SubCmd::VECTOR_STORE;
    } else if (util::str_case_equal(opt, "PROMPT")) {
        return SubCmd::PROMPT;
    } else if (util::str_case_equal(opt, "APP")) {
        return SubCmd::APP;
    } else {
        throw Error("unknown sub command");
    }
}

CreateCommand::Args CreateCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 3) {
        throw WrongArityError();
    }

    Args args;
    args.sub_cmd = _parse_sub_cmd(util::to_sv(argv[1]));
    args.key_name = argv[2];

    if (argc > 3) {
        auto opt = util::to_sv(argv[3]);
        if (util::str_case_equal(opt, "--NX")) {
            args.opt = Args::Opt::NX;
        } else if (util::str_case_equal(opt, "--XX")) {
            args.opt = Args::Opt::XX;
        }
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
