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
#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/redis_llm.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

void AddCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    if (args.embedding.empty()) {
        _blocking_add(ctx, args);
    } else {
        // No need to do embedding, so no need to block the client.
        auto id = _add(ctx, args);

        RedisModule_ReplyWithLongLong(ctx, id);

        RedisModule_ReplicateVerbatim(ctx);
    }
}

void AddCommand::_blocking_add(RedisModuleCtx *ctx, const Args &args) const {
    auto &llm = RedisLlm::instance();
    auto *store = api::get_value_by_key<VectorStore>(ctx, args.key_name, llm.vector_store_type());
    if (store == nullptr) {
        // TODO: create one automatically.
        throw Error("vector store does not exist");
    }

    auto *model = api::get_value_by_key<LlmModel>(ctx, store->llm().key, llm.llm_type());
    if (model == nullptr) {
        throw Error("LLM model for vector store does not exist");
    }

    auto vector_store = std::static_pointer_cast<VectorStore>(store->shared_from_this());
    auto llm_model = std::static_pointer_cast<LlmModel>(model->shared_from_this());
    auto *blocked_client = RedisModule_BlockClient(ctx, _reply_func, _timeout_func, _free_func, args.timeout.count());

    try {
        llm.worker_pool().enqueue(&AddCommand::_async_add, this, blocked_client, args, vector_store, llm_model);
    } catch (const Error &err) {
        RedisModule_AbortBlock(blocked_client);

        api::reply_with_error(ctx, err);
    }
}

void AddCommand::_async_add(RedisModuleBlockedClient *blocked_client,
        const Args &args, const VectorStoreSPtr &store, const LlmModelSPtr &model) const {
    assert(args.embedding.empty() && store && model);

    auto result = std::make_unique<AsyncResult>();
    try {
        auto embedding = model->embedding(args.data, store->llm().params);

        if (args.id) {
            store->add(*args.id, args.data, embedding);
            result->id = *args.id;
        } else {
            result->id = store->add(args.data, embedding);
        }
    } catch (const Error &) {
        result->err = std::current_exception();
    }

    RedisModule_UnblockClient(blocked_client, result.release());
}

uint64_t AddCommand::_add(RedisModuleCtx *ctx, const Args &args) const {
    auto *store = api::get_value_by_key<VectorStore>(ctx, args.key_name, RedisLlm::instance().vector_store_type());
    if (store == nullptr) {
        throw Error("vector store does not exist");
    }

    assert(!args.embedding.empty());

    if (args.id) {
        if (store->dim() != args.embedding.size()) {
            throw Error("embedding dimension not match");
        }

        store->add(*(args.id), args.data, args.embedding);

        return *(args.id);
    } else {
        if (store->dim() != args.embedding.size()) {
            throw Error("embedding dimension not match");
        }

        return store->add(args.data, args.embedding);
    }
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

    args.data = util::to_sv(argv[idx]);

    return args;
}

int AddCommand::_reply_func(RedisModuleCtx *ctx, RedisModuleString ** /*argv*/, int /*argc*/) {
    auto *res = static_cast<AsyncResult *>(RedisModule_GetBlockedClientPrivateData(ctx));
    assert(res != nullptr);

    if (res->err) {
        try {
            std::rethrow_exception(res->err);
        } catch (const Error &e) {
            api::reply_with_error(ctx, e);
        }
    } else {
        RedisModule_ReplyWithLongLong(ctx, res->id);

        RedisModule_ReplicateVerbatim(ctx);
    }

    return REDISMODULE_OK;
}

int AddCommand::_timeout_func(RedisModuleCtx *ctx, RedisModuleString ** /*argv*/, int /*argc*/) {
    return RedisModule_ReplyWithNull(ctx);
}

void AddCommand::_free_func(RedisModuleCtx * /*ctx*/, void *privdata) {
    auto *result = static_cast<AsyncResult *>(privdata);
    delete result;
}

}
