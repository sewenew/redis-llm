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

#include "sw/redis-llm/create_search_command.h"

namespace sw::redis::llm {

CreateSearchCommand::Args CreateSearchCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 2) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    auto idx = 2;
    while (idx < argc) {
        auto opt = util::to_sv(argv[idx]);
        if (util::str_case_equal(opt, "--NX")) {
            args.opt = api::CreateOption::NX;
        } else if (util::str_case_equal(opt, "--XX")) {
            args.opt = api::CreateOption::XX;
        } else if (util::str_case_equal(opt, "--LLM")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.llm = LlmInfo(util::to_sv(argv[idx]));
        } else if (util::str_case_equal(opt, "--VECTOR-STORE")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.params["vector-store"] = util::to_string(argv[idx]);
        } else if (util::str_case_equal(opt, "--PROMPT")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.params["prompt"] = util::to_string(argv[idx]);
        } else if (util::str_case_equal(opt, "--K")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            try {
                args.params["k"] = std::stoul(util::to_string(argv[idx]));
            } catch (const std::exception &e) {
                throw Error(std::string("invalid k") + e.what());
            }
        } else {
            break;
        }

        ++idx;
    }

    if (idx < argc) {
        throw WrongArityError();
    }

    if (args.params.find("vector-store") == args.params.end()) {
        throw Error("no vector store is specified");
    }

    if (args.params.find("k") == args.params.end()) {
        args.params["k"] = 3;
    }

    return args;
}

}
