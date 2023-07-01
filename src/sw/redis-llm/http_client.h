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

#ifndef SEWENEW_REDIS_LLM_HTTP_CLIENT_H
#define SEWENEW_REDIS_LLM_HTTP_CLIENT_H

#include <chrono>
#include <string>
#include "nlohmann/json.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

namespace sw::redis::llm {

struct HttpClientOptions {
    HttpClientOptions() = default;

    explicit HttpClientOptions(const nlohmann::json &conf);

    std::string uri;

    std::chrono::milliseconds socket_timeout = std::chrono::seconds(5);
    std::chrono::milliseconds connect_timeout = std::chrono::seconds(5);

    std::string bearer_token;

    bool enable_certificate_verification = false;

    std::string proxy_host;

    int proxy_port = 0;
};

struct HttpClientPoolOptions {
    HttpClientPoolOptions() = default;

    explicit HttpClientPoolOptions(const nlohmann::json &conf);

    std::size_t size = 5;

    std::chrono::milliseconds wait_timeout{0};

    std::chrono::milliseconds connection_lifetime{0};
};

class HttpClient {
public:
    explicit HttpClient(const HttpClientOptions &opts);

    std::string post(const std::string &path, const std::string &body, const std::string &content_type = "application/json");

    std::string post(const std::string &path, const httplib::Headers &headers,
            const std::string &body, const std::string &content_type = "application/json");

    void reconnect() {
        _cli = _make_client(_opts);
    }

    bool broken() const {
        return !_cli;
    }

    auto create_time() const
        -> std::chrono::time_point<std::chrono::steady_clock> {
        return _create_time;
    }

private:
    std::string _parse_response(const httplib::Result &res);

    std::unique_ptr<httplib::Client> _make_client(const HttpClientOptions &opts) const;

    HttpClientOptions _opts;

    std::chrono::time_point<std::chrono::steady_clock> _create_time{};

    std::unique_ptr<httplib::Client> _cli;
};

class HttpClientPool {
public:
    HttpClientPool(const HttpClientOptions &opts, const HttpClientPoolOptions &pool_opts);

    HttpClientPool(const HttpClientPool &) = delete;
    HttpClientPool& operator=(const HttpClientPool &) = delete;

    HttpClientPool(HttpClientPool &&that);
    HttpClientPool& operator=(HttpClientPool &&that);

    HttpClient fetch();

    void release(HttpClient cli);

private:
    void _move(HttpClientPool &&that);

    bool _need_reconnect(const HttpClient &cli, const std::chrono::milliseconds &lifetime) const;

    void _wait_for_client(std::unique_lock<std::mutex> &lock);

    HttpClient _fetch();

    HttpClientOptions _opts;

    HttpClientPoolOptions _pool_opts;

    std::deque<HttpClient> _pool;

    std::size_t _used_connections = 0;

    std::mutex _mutex;

    std::condition_variable _cv;
};

class SafeClient {
public:
    explicit SafeClient(HttpClientPool &pool) : _pool(pool), _cli(_pool.fetch()) {
        assert(!_cli.broken());
    }

    SafeClient(const SafeClient &) = delete;
    SafeClient& operator=(const SafeClient &) = delete;

    SafeClient(SafeClient &&) = delete;
    SafeClient& operator=(SafeClient &&) = delete;

    ~SafeClient() {
        _pool.release(std::move(_cli));
    }

    HttpClient& client() {
        return _cli;
    }

private:
    HttpClientPool &_pool;
    HttpClient _cli;
};

}

#endif // end SEWENEW_REDIS_LLM_HTTP_CLIENT_H
