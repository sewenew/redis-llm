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

#include "sw/redis-llm/errors.h"
#include "sw/redis-llm/http_client.h"

namespace {

size_t write_callback(void *ptr, size_t size, size_t nmemb, std::string *res) {
    auto len = size * nmemb;
    auto *begin = static_cast<char *>(ptr);
    auto *end = begin + len;
    res->insert(res->end(), begin, end);
    return len;
}

std::chrono::milliseconds parse_time(const std::string &str) {
    std::size_t timeout = 0;
    std::string unit;
    try {
        std::size_t pos = 0;
        timeout = std::stoul(str, &pos);
        unit = str.substr(pos);
    } catch (const std::exception &e) {
        throw sw::redis::llm::Error("invalid timeout type: " + str + ", err: " + e.what());
    }

    std::chrono::milliseconds val{0};
    if (unit == "ms") {
        val = std::chrono::milliseconds(timeout);
    } else if (unit == "s") {
        val = std::chrono::seconds(timeout);
    } else if (unit == "m") {
        val = std::chrono::minutes(timeout);
    } else {
        throw sw::redis::llm::Error("unknown timeout unit: " + unit);
    }

    return val;
}

}

namespace sw::redis::llm {

HttpClientOptions::HttpClientOptions(const nlohmann::json &conf) {
    auto iter = conf.find("uri");
    if (iter != conf.end()) {
        uri = iter.value().get<std::string>();
    }

    iter = conf.find("socket_timeout");
    if (iter != conf.end()) {
        socket_timeout = parse_time(iter.value().get<std::string>());
    }

    iter = conf.find("connect_timeout");
    if (iter != conf.end()) {
        connect_timeout = parse_time(iter.value().get<std::string>());
    }

    iter = conf.find("bearer_token");
    if (iter != conf.end()) {
        bearer_token = iter.value().get<std::string>();
    }

    iter = conf.find("enable_certificate_verification");
    if (iter != conf.end()) {
        enable_certificate_verification = iter.value().get<bool>();
    }

    iter = conf.find("proxy_host");
    if (iter != conf.end()) {
        proxy_host = iter.value().get<std::string>();
    }

    iter = conf.find("proxy_port");
    if (iter != conf.end()) {
        proxy_port = iter.value().get<int>();
    }
}

HttpClientPoolOptions::HttpClientPoolOptions(const nlohmann::json &conf) {
    auto iter = conf.find("size");
    if (iter != conf.end()) {
        size = iter.value().get<std::size_t>();
    }

    iter = conf.find("wait_timeout");
    if (iter != conf.end()) {
        wait_timeout = parse_time(iter.value().get<std::string>());
    }

    iter = conf.find("connection_lifetime");
    if (iter != conf.end()) {
        connection_lifetime = parse_time(iter.value().get<std::string>());
    }
}

HttpClient::HttpClient(const HttpClientOptions &opts) :
    _opts(opts),
    _create_time(std::chrono::steady_clock::now()),
    _cli(_make_client()) {
    assert(!broken());
}

std::string HttpClient::post(const std::string &path,
        const std::string &body,
        const std::string &content_type) {
    return post(path, {}, body, content_type);
}

std::string HttpClient::post(const std::string &path,
        const std::unordered_multimap<std::string, std::string> &headers,
        const std::string &body,
        const std::string &content_type) {
    auto *handle = _cli.get();

    auto header = _build_header(content_type, headers);
    _set_option(handle, CURLOPT_HTTPHEADER, header.get());
    //_set_option(handle, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);

    std::string uri = _opts.uri + path;
    _set_option(handle, CURLOPT_URL, uri.data());

    _set_option<long>(handle, CURLOPT_POSTFIELDSIZE, body.size());
    _set_option(handle, CURLOPT_POSTFIELDS, body.data());
    _set_option(handle, CURLOPT_WRITEFUNCTION, write_callback);

    std::string response;
    _set_option(handle, CURLOPT_WRITEDATA, &response);

    auto res = curl_easy_perform(handle);
    if (res != CURLE_OK) {
        _cli.release();
        throw Error(std::string("failed to do post: ") + curl_easy_strerror(res));
    }

    long code = 0;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
    if (code != 200) {
        throw Error("failed to do post: " + response);
    }

    return response;
}

HttpClient::SList HttpClient::_build_header(const std::string &content_type,
        std::unordered_multimap<std::string, std::string> headers) const {
    if (!_opts.bearer_token.empty()) {
        headers.emplace("Authorization", "Bearer " + _opts.bearer_token);
    }

    if (!content_type.empty()) {
        headers.emplace("Content-Type", content_type);
    }

    SList slist;
    for (const auto &[key, value] : headers) {
        std::string header = key + ": " + value;
        auto *tmp = curl_slist_append(slist.get(), header.data());
        if (tmp == nullptr) {
            throw Error("failed to append header: " + header);
        }
        slist.release();
        slist.reset(tmp);
    }

    return slist;
}

HttpClient::Client HttpClient::_make_client() const {
    auto client = Client(curl_easy_init());
    if (!client) {
        throw Error("failed to create curl handle");
    }

    _set_option(client.get(), CURLOPT_FOLLOWLOCATION, 1L);

    long connection_timeout_ms = _opts.connect_timeout.count();
    _set_option(client.get(), CURLOPT_CONNECTTIMEOUT_MS, connection_timeout_ms);

    long socket_timeout_ms = _opts.socket_timeout.count();
    _set_option(client.get(), CURLOPT_TIMEOUT_MS, socket_timeout_ms);

    long verify = _opts.enable_certificate_verification ? 1L : 0L;
    _set_option(client.get(), CURLOPT_SSL_VERIFYPEER, verify);

    if (!_opts.proxy_host.empty()) {
        _set_option(client.get(), CURLOPT_PROXY, _opts.proxy_host.data());

        _set_option(client.get(), CURLOPT_PROXYPORT, static_cast<long>(_opts.proxy_port));
    }

    return client;
}

HttpClientPool::HttpClientPool(const HttpClientOptions &opts,
        const HttpClientPoolOptions &pool_opts) :
    _opts(opts), _pool_opts(pool_opts) {
    if (_pool_opts.size == 0) {
        throw Error("cannot create an empty pool");
    }

    // Lazily create connections.
}

HttpClientPool::HttpClientPool(HttpClientPool &&that) {
    std::lock_guard<std::mutex> lock(that._mutex);

    _move(std::move(that));
}

HttpClientPool& HttpClientPool::operator=(HttpClientPool &&that) {
    if (this != &that) {
        std::lock(_mutex, that._mutex);
        std::lock_guard<std::mutex> lock_this(_mutex, std::adopt_lock);
        std::lock_guard<std::mutex> lock_that(that._mutex, std::adopt_lock);

        _move(std::move(that));
    }

    return *this;
}

HttpClient HttpClientPool::fetch() {
    std::unique_lock<std::mutex> lock(_mutex);

    if (_pool.empty()) {
        if (_used_connections == _pool_opts.size) {
            _wait_for_client(lock);
        } else {
            auto cli = HttpClient(_opts);
            ++_used_connections;
            return cli;
        }
    }

    auto cli = _fetch();

    auto lifetime = _pool_opts.connection_lifetime;

    lock.unlock();

    if (_need_reconnect(cli, lifetime)) {
        try {
            cli.reconnect();
        } catch (...) {
            release(std::move(cli));
            throw;
        }
    }

    return cli;
}

void HttpClientPool::release(HttpClient cli) {
    {
        std::lock_guard<std::mutex> lock(_mutex);

        _pool.push_back(std::move(cli));
    }

    _cv.notify_one();
}

void HttpClientPool::_move(HttpClientPool &&that) {
    _opts = std::move(that._opts);
    _pool_opts = std::move(that._pool_opts);
    _pool = std::move(that._pool);
    _used_connections = std::move(that._used_connections);
}

HttpClient HttpClientPool::_fetch() {
    assert(!_pool.empty());

    auto cli = std::move(_pool.front());
    _pool.pop_front();

    return cli;
}

void HttpClientPool::_wait_for_client(std::unique_lock<std::mutex> &lock) {
    auto timeout = _pool_opts.wait_timeout;
    if (timeout > std::chrono::milliseconds(0)) {
        // Wait until _pool is no longer empty or timeout.
        if (!_cv.wait_for(lock,
                    timeout,
                    [this] { return !(this->_pool).empty(); })) {
            throw Error("failed to fetch a client in "
                    + std::to_string(timeout.count()) + " milliseconds");
        }
    } else {
        // Wait forever.
        _cv.wait(lock, [this] { return !(this->_pool).empty(); });
    }
}

bool HttpClientPool::_need_reconnect(const HttpClient &cli,
        const std::chrono::milliseconds &lifetime) const {
    if (cli.broken()) {
        return true;
    }

    if (lifetime > std::chrono::milliseconds(0)) {
        auto now = std::chrono::steady_clock::now();
        if (now - cli.create_time() > lifetime) {
            return true;
        }
    }

    return false;
}

}
