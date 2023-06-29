# redis-llm

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
- [Commands](#commands)
    - [Path](#path)
- [Author](#author)

## Overview

This is a [Redis Module](https://redis.io/topics/modules-intro) that integrates LLM (Large Language Model) with Redis.

LLM is powerful, but it’s still limited.

- It cannot access your private data, and cannot answer questions based on those data.
- It has token limitation, and cannot remember too much chat history.

In order to make LLM access private data, and remember more history, I wrote redis-llm to integrate LLM with Redis. This module makes Redis an extended memory of LLM. You can save your private data to Redis with redis-llm, and ask LLM to answer questions based on the data. You can also chat with redis-llm, which can save and index your chat history to help LLM chat with a long history.

### Features

- Run LLM applications with prompt template.
- Vector store.
- Ask questions on private data saved in vector store.
- Chat with a long history (this feature will be ready in the near future).

## Installation

### Run redis-llm With Docker

Run the following command to start *redis-llm* with Docker.

```
docker run -p 6379:6379 sewenew/redis-llm:latest
```

In this case, Docker runs Redis with a *redis.conf* file located at */usr/lib/redis/conf/redis.conf*.

After running the Docker image, you can go to the [Getting Started section](#getting-started) to see how to run *redis-llm* commands.

### Install redis-protobuf With Source Code

You can also install redis-llm with source code.

*redis-llm* is built with [CMAKE](https://cmake.org).

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

In order to load *redis-llm*, you need to modify the *redis.conf* file to add the `loadmodule` directive:

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

After [loading the module](#load-redis-llm), you can use any Redis client to send *redis-llm* [commands](#Commands).

### redis-cli

The following examples use the offical Redis client, i.e. *redis-cli*, to send *redis-llm* commands.

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
127.0.0.1:6379> LLM.CREATE LLM openai --params '{"api_key" : "$OPENAI_API_KEY"}'
(integer) 1
```

Create a *hello world* application with the created LLM, and run it.

```
127.0.0.1:6379> LLM.CREATE APP hello-world --LLM openai --PROMPT 'Say hello to {{name}}'
(integer) 1
127.0.0.1:6379> LLM.RUN hello-world --vars '{"name":"LLM"}'
"Hello LLM! It's nice to meet you. How can I assist you today?"
```

Create a vector store, and add doc to it.

```
127.0.0.1:6379> LLM.CREATE VECTOR_STORE store --LLM openai
(integer) 1
127.0.0.1:6379> LLM.ADD store 'redis-llm is a Redis module that integrates LLM (Large Language Model) with Redis'
(integer) 1
```

Create a search application which can answer questions based on data saved in the vector store.

```
127.0.0.1:6379> LLM.CREATE SEARCH search-private-data --LLM openai --VECTOR_STORE store
(integer) 1
127.0.0.1:6379> LLM.ADD store 'redis-llm is an open source project written by sewenew'
(integer) 1
127.0.0.1:6379> LLM.RUN search-private-data 'who is the author of redis-llm'
"The author of redis-llm is sewenew."
```

### C++ Client

If you are using C++, you can use [redis-plus-plus](https://github.com/sewenew/redis-plus-plus) to send *redis-llm* commands:

```C++
```

### Python Client

If you are using Python, you can use [redis-py](https://github.com/redis/redis-py) to send *redis-llm* commands:

```Python
```

## Commands

### LLM.CREATE

#### Syntax

```
LLM.CREATE APP key [--NX] [--XX] [--TYPE app] --LLM llm-info [--PARAMS '{}'] [--PROMPT prompt]

LLM.CREATE SEARCH key [--NX] [--XX] --LLM llm-info [--K 10] --VECTOR_STORE xxx [--PROMPT prompt]

LLM.CREATE VECTOR_STORE key [--NX] [--XX] [--TYPE xxx] [--LLM llm-info] [--PARAMS '{}']

LLM.CREATE LLM key [--NX] [--XX] --TYPE openai --PARAMS '{}'
```

#### Options

#### Return Value

#### Error

#### Time Complexity

#### Examples

### LLM.ADD

#### Syntax

```
LLM.ADD key [--ID id] [--EMBEDDING xxx] data
```

#### Options

#### Return Value

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

*redis-llm* is written by [sewenew](https://github.com/sewenew), who is also active on [StackOverflow](https://stackoverflow.com/users/5384363/for-stack).
