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

#include "sw/redis-llm/knn_command.h"
#include "sw/redis-llm/llm_model.h"
#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/redis_llm.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

void KnnCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto res = _knn(ctx, argv, argc);
    if (!res || res->empty()) {
        RedisModule_ReplyWithNull(ctx);
    } else {
        RedisModule_ReplyWithArray(ctx, res->size());
        for (const auto &ele : *res) {
            RedisModule_ReplyWithArray(ctx, 2);
            RedisModule_ReplyWithLongLong(ctx, ele.first);
            RedisModule_ReplyWithDouble(ctx, ele.second);
        }
    }
}

std::optional<std::vector<std::pair<uint64_t, float>>> KnnCommand::_knn(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    auto key = api::open_key(ctx, args.key_name, api::KeyMode::READONLY);
    assert(key);

    auto &llm = RedisLlm::instance();
    if (!api::key_exists(key.get(), llm.vector_store_type())) {
        return std::nullopt;
    }

    auto *store = api::get_value_by_key<VectorStore>(*key);
    assert(store != nullptr);

    if (args.embedding.empty()) {
        if (args.query.empty()) {
            throw Error("no query is specified");
        }

        auto *model = api::get_model_by_key(ctx, store->llm().key);
        if (model == nullptr) {
            throw Error("LLM model does not exist: " + store->llm().key);
        }

        auto query = model->embedding(args.query, store->llm().params);

        return store->knn(query, args.k);
    } else {
        return store->knn(args.embedding, args.k);
    }
}

KnnCommand::Args KnnCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 3) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    auto idx = 2;
    while (idx < argc) {
        auto opt = util::to_sv(argv[idx]);
        if (util::str_case_equal(opt, "--K")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            try {
                args.k = std::stoul(util::to_string(argv[idx]));
            } catch (const std::exception &e) {
                throw Error(std::string("invalid k: ") + e.what());
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

    if (idx < argc) {
        args.query = util::to_sv(argv[idx]);
    }

    return args;
}

}
