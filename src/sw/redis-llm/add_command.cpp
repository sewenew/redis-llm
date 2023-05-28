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

#include "sw/redis-llm/add_command.h"
#include "sw/redis-llm/application.h"
#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/redis_llm.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

void AddCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    _add(ctx, argv, argc);

    RedisModule_ReplyWithSimpleString(ctx, "OK");

    RedisModule_ReplicateVerbatim(ctx);
}

void AddCommand::_add(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    auto key = api::open_key(ctx, args.key_name, api::KeyMode::WRITEONLY);
    assert(key);

    auto &llm = RedisLlm::instance();
    if (!api::key_exists(key.get(), llm.type())) {
        throw Error("key does not exist, call LLM.CREATE first");
    }

    auto *app = api::get_value_by_key<Application>(*key);
    assert(app != nullptr);

    if (args.with_embedding) {
        if (app->dim() != args.embedding.size()) {
            throw Error("embedding dimension not match");
        }

        app->add(args.id, args.data, args.embedding);
    } else {
        app->add(args.id, args.data);
    }
}

AddCommand::Args AddCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 4) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    auto idx = 2;
    while (idx < argc) {
        auto opt = util::to_sv(argv[idx]);
        if (util::str_case_equal(opt, "--WITHEMBEDDING")) {
            args.with_embedding = true;
        } else {
            break;
        }

        ++idx;
    }

    if ((args.with_embedding && argc != 6) ||
            (!args.with_embedding && argc != 4)) {
        throw WrongArityError();
    }

    try {
        args.id = std::stoul(std::string(util::to_sv(argv[idx++])));
    } catch (const std::exception &e) {
        throw Error("invalid id");
    }

    args.data = util::to_sv(argv[idx++]);

    if (args.with_embedding) {
        std::vector<std::string_view> vec;
        util::split(util::to_sv(argv[idx++]), ",", std::back_inserter(vec));
        Vector embedding;
        embedding.reserve(vec.size());
        for (auto &ele : vec) {
            try {
                embedding.push_back(stof(std::string(ele)));
            } catch (const std::exception &e) {
                throw Error("invalid embedding");
            }
        }
        args.embedding = std::move(embedding);
    }

    return args;
}

}
