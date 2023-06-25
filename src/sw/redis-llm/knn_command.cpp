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
    auto args = _parse_args(argv, argc);

    auto &llm = RedisLlm::instance();
    auto *store = api::get_value_by_key<VectorStore>(ctx, args.key_name, llm.vector_store_type());
    if (store == nullptr) {
        // TODO: create one automatically.
        throw Error("vector store does not exist");
    }

    auto vector_store = std::static_pointer_cast<VectorStore>(store->shared_from_this());

    LlmModelSPtr llm_model;
    if (args.embedding.empty()) {
        if (args.query.empty()) {
            throw Error("no query is specified");
        }

        auto *model = api::get_value_by_key<LlmModel>(ctx, store->llm().key, llm.llm_type());
        if (model == nullptr) {
            throw Error("LLM model for vector store does not exist");
        }

        llm_model = std::static_pointer_cast<LlmModel>(model->shared_from_this());
    }

    auto *blocked_client = RedisModule_BlockClient(ctx, _reply_func, _timeout_func, _free_func, args.timeout.count());

    try {
        llm.worker_pool().enqueue(&KnnCommand::_knn, this, blocked_client, args, vector_store, llm_model);
    } catch (const Error &err) {
        RedisModule_AbortBlock(blocked_client);

        api::reply_with_error(ctx, err);
    }
}

void KnnCommand::_knn(RedisModuleBlockedClient *blocked_client,
        const Args &args, const VectorStoreSPtr &store, const LlmModelSPtr &model) const {
    assert(blocked_client != nullptr && store);

    auto result = std::make_unique<AsyncResult>();
    try {
        if (args.embedding.empty()) {
            assert(model && !args.query.empty());

            auto query = model->embedding(args.query, store->llm().params);

            result->neighbors = store->knn(query, args.k);
        } else {
            result->neighbors = store->knn(args.embedding, args.k);
        }
    } catch (const Error &) {
        result->err = std::current_exception();
    }

    RedisModule_UnblockClient(blocked_client, result.release());
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
        } else if (util::str_case_equal(opt, "--TIMEOUT")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            try {
                args.timeout = std::chrono::milliseconds(std::stoul(util::to_string(argv[idx])));
            } catch (const std::exception &e) {
                throw Error(std::string("timeout should be a number: ") + e.what());
            }
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

int KnnCommand::_reply_func(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    auto *res = static_cast<AsyncResult *>(RedisModule_GetBlockedClientPrivateData(ctx));
    assert(res != nullptr);

    if (res->err) {
        try {
            std::rethrow_exception(res->err);
        } catch (const Error &e) {
            api::reply_with_error(ctx, e);
        }
    } else {
        auto &neighbors = res->neighbors;
        RedisModule_ReplyWithArray(ctx, neighbors.size());
        for (const auto &[id, dist] : neighbors) {
            RedisModule_ReplyWithArray(ctx, 2);
            RedisModule_ReplyWithLongLong(ctx, id);
            RedisModule_ReplyWithDouble(ctx, dist);
        }
    }

    return REDISMODULE_OK;
}

int KnnCommand::_timeout_func(RedisModuleCtx *ctx, RedisModuleString ** /*argv*/, int /*argc*/) {
    return RedisModule_ReplyWithNull(ctx);
}

void KnnCommand::_free_func(RedisModuleCtx * /*ctx*/, void *privdata) {
    auto *result = static_cast<AsyncResult *>(privdata);
    delete result;
}

}
