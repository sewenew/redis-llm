# redis-llm

[中文文档](https://github.com/sewenew/redis-llm/blob/main/Chinese.md)

- [Overview](#overview)
    - [Features](#features)
- [Installation](#installation)
    - [Run redis-llm With Docker](#run-redis-llm-with-docker)
    - [Install redis-llm With Source Code](#install-redis-llm-with-source-code)
    - [Load redis-llm](#load-redis-llm)
- [Getting Started](#getting-started)
    - [redis-cli](#redis-cli)
    - [C++ Client](#c-client)
    - [Python Client](#python-client)
- [Terminology](#terminology)
    - [LLM](#llm)
    - [Prompt](#prompt)
    - [Vector Store](#vector-store)
    - [Application](#application)
- [Commands](#commands)
    - [Path](#path)
- [Author](#author)

## Overview

redis-llm is a [Redis Module](https://redis.io/topics/modules-intro) that integrates LLM (Large Language Model) with Redis.

LLM is powerful, but it’s still limited.

- It cannot access your private data, and cannot answer questions based on those data.
- It has token limitation, and cannot remember too much chat history.

In order to solve these problems, I write redis-llm to integrate LLM with Redis. This module makes Redis an extended memory of LLM. You can save your private data to Redis with redis-llm, and ask LLM to answer questions based on the data. You can also chat with redis-llm, which can save and index your chat history to help LLM chat with a long history.

### Features

- Run LLM applications with prompt template.
- Vector store.
- Ask questions on private data saved in vector store.
- Chat with a long history.

## Installation

### Run redis-llm With Docker

Run the following command to start redis-llm with Docker.

```
docker run -p 6379:6379 sewenew/redis-llm:latest
```

In this case, Docker runs Redis with a *redis.conf* file located at */usr/lib/redis/conf/redis.conf*.

After running the Docker image, you can go to the [Getting Started section](#getting-started) to see how to run redis-llm commands.

### Install redis-protobuf With Source Code

You can also install redis-llm with source code.

redis-llm depends on curl and openssl, and you need to install these dependencies first.

```
apt-get install libssl-dev libcurl4-openssl-dev
```

redis-llm is built with [CMAKE](https://cmake.org).

```
git clone https://github.com/sewenew/redis-llm.git

cd redis-llm

mkdir compile

cd compile

cmake ..

make
```

When `make` is done, you should find *libredis-llm.so* (or *libredis-llm.dylib* on MacOS) under the *redis-llm/compile* directory.

### Load redis-llm

redis-llm module depends on Redis 5.0's module API, so you must install Redis 5.0 or above.

In order to load redis-llm, you need to modify the *redis.conf* file to add the `loadmodule` directive:

```
loadmodule /path/to/libredis-llm.so
```

Now, you can start your Redis instance:

```
redis-server /path/to/redis.conf
```

If Redis loads the module successfully, you can get the following message from the log:

```
Module 'LLM' loaded from /path/to/libredis-llm.so
```

## Getting Started

After [loading the module](#load-redis-llm), you can use any Redis client to send redis-llm [commands](#Commands).

### redis-cli

The following examples use the offical Redis client, i.e. *redis-cli*, to send redis-llm commands.

List module info:

```
127.0.0.1:6379> MODULE LIST
1) 1) "name"
   2) "LLM"
   3) "ver"
   4) (integer) 1
```

Create a LLM of OpenAI type.

```
127.0.0.1:6379> LLM.CREATE-LLM openai --params '{"api_key" : "$OPENAI_API_KEY"}'
(integer) 1
```

Create a *hello world* application with the created LLM and a prompt, and run it.

```
127.0.0.1:6379> LLM.CREATE-APP hello-world --LLM openai --PROMPT 'Say hello to {{name}}'
(integer) 1
127.0.0.1:6379> LLM.RUN hello-world --vars '{"name":"LLM"}'
"Hello LLM! It's nice to meet you. How can I assist you today?"
```

Create a vector store, and add doc to it.

```
127.0.0.1:6379> LLM.CREATE-VECTOR-STORE store --LLM openai
(integer) 1
127.0.0.1:6379> LLM.ADD store 'redis-llm is a Redis module that integrates LLM (Large Language Model) with Redis'
(integer) 1
```

Create a search application which can answer questions based on data saved in the vector store.

```
127.0.0.1:6379> LLM.CREATE-SEARCH search-private-data --LLM openai --VECTOR-STORE store
(integer) 1
127.0.0.1:6379> LLM.ADD store 'redis-llm is an open source project written by sewenew'
(integer) 1
127.0.0.1:6379> LLM.RUN search-private-data 'who is the author of redis-llm'
"The author of redis-llm is sewenew."
```

### C++ Client

If you are using C++, you can use [redis-plus-plus](https://github.com/sewenew/redis-plus-plus) to send redis-llm commands:

```C++
```

### Python Client

If you are using Python, you can use [redis-py](https://github.com/redis/redis-py) to send redis-llm commands:

```Python
```

## Terminology

You can use redis-llm to create applications with LLM, e.g. chat bot. The following are some terminologies.

### LLM

Large Language Model.

Currently, we support the following LLMs:

- [OpenAI](https://platform.openai.com/docs/api-reference)
- [Azure OpenAI](https://azure.microsoft.com/en-us/products/cognitive-services/openai-service)

If you need other LLMs support, feel free to leave a message [here](https://github.com/sewenew/redis-llm/issues/2).

### Prompt

You input some *prompt*, and LLMs return a text *completion*. A good prompt makes a good completion. Check [this](https://help.openai.com/en/articles/6654000-best-practices-for-prompt-engineering-with-openai-api) for some best practices for prompt engineering.

The following command creates an LLM application with a prompt:

```
LLM.CREATE-APP test-application --PROMPT 'You are an expert on prompt engineering. Please give some best practices for prompt engineering.'
```

Once you run the application, it gives you text completion, i.e. best practices for prompt engineering, based on your prompt.

```
LLM.RUN test-application
```

#### Prompt Template

Besides hard-coded prompt, redis-llm also supports prompt template. You can create a prompt template with some variables. With different values for these variables, you can get a dynamic template.

The following command creates an LLM application with a prompt template (NOTE: in order to make the following example work, you need to create an LLM named *llm* beforehand):

```
LLM.CREATE-APP app-with-prompt-template --LLM llm --PROMPT 'You are an expert on {{domain}}. Please answer the following question: {{question}}'
```

The substring between *{{* and *}}* is a template variable. In the above example, there're two variables, *domain* and *question*. When you run the application, you can set different values for these variables to make a dynamic template.

```
LLM.RUN app-with-prompt-template --LLM llm --VARS '{"domain" : "LLM", "question" : "Please give an introduction on LLM."}'
```

With *--VARS* option, we set values for each variable. The option is in JSON format, and each key-value pair in JSON string corresponds to a variable. redis-llm renders the above template into the following prompt, and ask LLM for completion.

```
You are an expert on LLM. Please answer the following question: Please give an introduction on LLM.
```

### Vector Store

Vector store is a database that store data with their embeddings, i.e. vector. Embedding can capture semantics of the data, and data with similar embeddings are semantically similar. Normally, we use Approximate Nearest Neighbor (ANN) algorithms to search the K approximatly nearest items of a given input.

There are many ANN algorithms, and currently, we support the following ones:

- [HNSW](https://github.com/nmslib/hnswlib)

### Application

With redis-llm, you can create LLM applications.

Currently, we support the following applications:

- Simple Application: Application with a prompt or prompt template. You can run the application with an input string (prompt), and it returns an output string (completion). Check [LLM.CREATE-APP command](#llmcreate-app) for detail.
- Search Application: Application which can answer questions on a given vector store (you can store your private data in the vector store). Check [LLM.CREATE-SEARCH command](#llmcreate-search) for detail.
- Chat Application: Application which helps you chat with LLM. Chat Application indexes the chat history, and helps LLM remember your conversation history. Check [LLM.CREATE-CHAT command](#llmcreate-chat) for detail.

## Commands

Command names and option names are case insensitive.

### LLM.CREATE-LLM

**LLM.CREATE-LLM** creates an LLM model. Since all applications use LLM model to do query or embedding, you must create an LLM model before creating application.

#### Syntax

```
LLM.CREATE-LLM key [--NX] [--XX] [--TYPE model-type] --PARAMS '{model parameters in JSON format}'
```

#### Options

- **--NX**: Create model, if and only if *key* does not exist. Optional.
- **--XX**: Create model, if and only if *key* exists. Optional.
- **--TYPE**: Model type. Optional. If not specified, the default type is *openai*.
- **--PARAMS**: Model parameters. Check [LLM Models section](#llm-models) for detail.

#### LLM Models

##### openai

If you want to use OpenAI, you should specify `--TYPE openai`. The parameters are as follows:

```JSON
{"api_key": "required", "chat": {"chat_path": "/v1/chat/completions", "model": "gpt-3.5-turbo"}, "embedding": {"embedding_path": "/v1/embeddings", "model":"text-embedding-ada-002"}, "http":{"socket_timeout":"5s","connect_timeout":"5s", "enable_certificate_verification":false, "proxy_host": "", "proxy_port": 0, "pool" : {"size": 5, "wait_timeout":"0s", "connection_lifetime":"0s"}}}
```

All parameters are key-value pairs. The required ones are set as *required*. The optional ones are set with default values. If parameter is not specified, the default value is used.

##### azure openai

If you want to use Azure OpenAI, you should specify `--TYPE azure_openai`. The parameters are as follows:

```JSON
{"api_key": "required", "resource_name" : "required", "chat_deployment_id": "required", "embedding_deployment_id": "required", "api_version": "required", "chat": {}, "embedding": {}, "http": {"socket_timeout":"5s", "connect_timeout":"5s", "enable_certificate_verification": false, "proxy_host": "", "proxy_port": 0, "pool" : {"size": 5, "wait_timeout":"0s", "connection_lifetime":"0s"}}}
```

All parameters are key-value pairs. The required ones are set as *required*. The optional ones are set with default values. If parameter is not specified, the default value is used.

#### Return

Integer reply: 1 if create model OK. 0, otherwise, e.g. option *--NX* has been set, while the key already exists.

#### Error

Return an error reply in the following cases:

- Model type does not supported.
- Cannot create a model with the given parameters.
- Incorrect options.

#### Examples

```
// Create a LLM model of openai type. Set its api key, and use default values for other parameters
LLM.CREATE-LLM key --TYPE openai --PARAMS '{"api_key": "Your API KEY"}'

// Create a LLM model of openai type. Set api key, and chat model.
LLM.CREATE-LLM key --TYPE openai --PARAMS '{"api_key": "Your API KEY", "chat" : {"model" : "gpt-3.5-turbo-0301"}}'

// Create a LLM model of azure openai type. Set required parameters.
LLM.CREATE-LLM key --TYPE azure_openai --PARAMS '{"api_key": "Your API KEY", "resource_name": "your resource name", "chat_deployment_id": "your deployment id for chat api", "embedding_deployment_id": "your deployment id for embedding api", "api_version": "api version"}'

// Create a LLM model of azure openai type. Set required parameters, and set http proxy to access azure openai.
LLM.CREATE-LLM key --TYPE azure_openai --PARAMS '{"api_key": "Your API KEY", "resource_name": "your resource name", "chat_deployment_id": "your deployment id for chat api", "embedding_deployment_id": "your deployment id for embedding api", "api_version": "api version", "http": {"proxy_host": "http://xxx.xxx.xx", "proxy_port": 3149}}'
```

### LLM.CREATE-VECTOR-STORE

**LLM.CREATE-VECTOR-STORE** creates a vector store. If you want to LLM application to access your private data, you can save the data into a vector store.

#### Syntax

```
LLM.CREATE-VECTOR-STORE key [--NX] [--XX] [--TYPE vector-store-type] [--LLM llm-info] [--PARAMS '{store parameters in JSON format}']
```

#### Options

- **--NX**: Create store, if and only if *key* does not exist. Optional.
- **--XX**: Create store, if and only if *key* exists. Optional.
- **--TYPE**: Vector store type. Optional. If not specified, the default type is *hnsw*. Optional.
- **--LLM**: Redis key of LLM model that you want to use with this vector store. When you use LLM.ADD command without a given embedding, redis-llm uses this LLM model to build embedding, and add it into the store.
- **--PARAMS**: Vector store parameters. Check [Vector Stores section](#vector-stores) for detail. Optional.

#### Vector Stores

##### hnsw

Currently, we only support vector store of HNSW type, and this is also the default one. Of course, you can specify `--TYPE hnsw` explicitly. The parameters are as follows:

```JSON
{"max_elements": 100000, "m": 16, "ef_construction": 200, "dim": 0}
```

All parameters are key-value pairs. The required ones are set as *required*. The optional ones are set with default values. If parameter is not specified, the default value is used.

- *max_elements*: Max number of items that can be stored in the vector store.
- *dim*: Set the dimension of the vector saved in vector store. If it's 0, the default value, the dimension of the first inserted vector is used as the dimension of the vector store.

#### Return

Integer reply: 1 if create model OK. 0, otherwise, e.g. option *--NX* has been set, while the key already exists.

#### Error

Return an error reply in the following cases:

- Vector store type does not supported.
- Cannot create a vector store with the given parameters.
- Incorrect options.

#### Examples

```
// Create a vector store of default type, i.e. hnsw.
LLM.CREATE-VECTOR-STORE key

// Create a vector store by explicitly set hnsw as type.
LLM.CREATE-VECTOR-STORE key --TYPE hnsw

// Create a vector store, and set some parameters.
LLM.CREATE-VECTOR-STORE key --PARAMS '{"max_elements": 20000}'
```

### LLM.CREATE-APP

**LLM.CREATE-APP** creates a simple application. You can also set a prompt or prompt template for the application.

#### Syntax

```
LLM.CREATE-APP key [--NX] [--XX] --LLM llm-info [--PROMPT prompt]
```

#### Options

- **--NX**: Create store, if and only if *key* does not exist. Optional.
- **--XX**: Create store, if and only if *key* exists. Optional.
- **--TYPE**: Vector store type. Optional. If not specified, the default type is *hnsw*.
- **--LLM**: Redis key of LLM model that this application uses. Required.
- **--PROMPT**: Prompt or prompt template for this application. Check [Prompt section](#prompt) for detail. Optional.

#### Return

Integer reply: 1 if create model OK. 0, otherwise, e.g. option *--NX* has been set, while the key already exists.

#### Error

Return an error reply in the following cases:

- Incorrect options.

#### Examples

```
```

### LLM.ADD

#### Syntax

```
LLM.ADD key [--ID id] [--EMBEDDING xxx] data
```

Add an item into the vector store stored at *key*. Each item in the vector store has a unique ID, and an embedding.

#### Options

- **--ID**: Specify an ID of `uint64_t` type for the data. If ID already exists, overwrite it. Optional.
- **--EMBEDDING**: Specify embedding for the data. Optional.

If ID is not specified, redis-llm automatically generates an ID for the given data. If embedding is not specified, redis-llm calls LLM of the vector store to get an embedding.

#### Return

- *Integer reply*: ID of the inserted data.

#### Error


#### Time Complexity

#### Examples

### LLM.GET

#### Syntax

```
LLM.GET key id
```

#### Options

#### Return Value

#### Error

#### Time Complexity

#### Examples

### LLM.REM

#### Syntax

```
LLM.REM key id
```

#### Options

#### Return Value

#### Error

#### Time Complexity

#### Examples

### LLM.SIZE

#### Syntax

```
LLM.SIZE key
```

#### Options

#### Return Value

#### Error

#### Time Complexity

#### Examples

### LLM.KNN

#### Syntax

```
LLM.KNN key [--K 10] [--embedding xxx] [query]
```

#### Options

#### Return Value

#### Error

#### Time Complexity

#### Examples

### LLM.RUN

#### Syntax

```
LLM.RUN key [--VARS '{"user" : "Jim"}'] [--PARAMS '{}'] [--VERBOSE] [input]
```

#### Options

#### Return Value

#### Error

#### Time Complexity

#### Examples

## Author

redis-llm is written by [sewenew](https://github.com/sewenew), who is also active on [StackOverflow](https://stackoverflow.com/users/5384363/for-stack).
