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

#include "sw/redis-llm/application.h"

namespace sw::redis::llm {

std::string Application::ask(const std::string_view &question, bool without_private_data) {
    if (without_private_data) {
        return _model->predict(question);
    }

    _ask(question);
}

std::string Application::_ask(const std::string_view &question) {
    auto context = _search_private_data(question);

    auto input = _build_llm_input(question, context);

    return _model->predict(input);
}

std::vector<std::string> Application::_search_private_data(const std::string_view &question) {
}

std::string Application::_build_llm_input(const std::string_view &question,
        const std::vector<std::string> &context) {
}

}
