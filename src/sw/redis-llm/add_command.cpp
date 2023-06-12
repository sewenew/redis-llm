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
#include "sw/redis-llm/llm_model.h"
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
    if (!api::key_exists(key.get(), llm.vector_store_type())) {
        throw Error("key does not exist, call LLM.CREATE first");
    }

    auto *store = api::get_value_by_key<VectorStore>(*key);
    assert(store != nullptr);

    if (args.id) {
        if (!args.embedding.empty()) {
            if (store->dim() != args.embedding.size()) {
                throw Error("embedding dimension not match");
            }

            store->add(*(args.id), args.data, args.embedding);
        } else {
            auto embedding = _get_embedding(ctx, args.data, store->llm().key);
            store->add(*args.id, args.data, embedding);
        }
    } else {
        if (!args.embedding.empty()) {
            if (store->dim() != args.embedding.size()) {
                throw Error("embedding dimension not match");
            }

            store->add(args.data, args.embedding);
        } else {
            auto embedding = _get_embedding(ctx, args.data, store->llm().key);
            store->add(args.data, embedding);
        }
    }
}

Vector AddCommand::_get_embedding(RedisModuleCtx *ctx, const std::string_view &data, const std::string &llm_key) const {
    LlmModel *model = nullptr;
    auto *key_str = RedisModule_CreateString(ctx, llm_key.data(), llm_key.size());
    try {
        auto key = api::open_key(ctx, key_str, api::KeyMode::READONLY);
        assert(key);

        RedisModule_FreeString(ctx, key_str);

        auto &llm = RedisLlm::instance();
        if (!api::key_exists(key.get(), llm.llm_type())) {
            throw Error("LLM model does not exist, call LLM.CREATE first");
        }

        model = api::get_value_by_key<LlmModel>(*key);
        assert(model != nullptr);
    } catch (const Error &) {
        RedisModule_FreeString(ctx, key_str);
        throw;
    }

    return model->embedding(data, {});
}

AddCommand::Args AddCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 3) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    auto idx = 2;
    while (idx < argc) {
        auto opt = util::to_sv(argv[idx]);
        if (util::str_case_equal(opt, "--ID")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            try {
                args.id = std::stoul(util::to_string(argv[idx]));
            } catch (const std::exception &) {
                throw Error("invalid id");
            }
        } else if (util::str_case_equal(opt, "--EMBEDDING")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.embedding = util::parse_embedding(util::to_sv(argv[idx]));
        } else {
            break;
        }

        ++idx;
    }

    args.data = util::to_sv(argv[idx]);

    return args;
}

}
