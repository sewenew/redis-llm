From redis:latest

ENV LIBDIR /usr/lib/redis/modules
ENV DEPS "make g++ curl cmake unzip libssl-dev"

# Install dependencies
RUN set -ex;\
	deps="$DEPS";\
        apt-get update;\
        apt-get install -y --no-install-recommends $deps;

# Build redis-llm
RUN set -ex;\
        cd /usr/src;\
        curl -L -k 'https://github.com/sewenew/redis-llm/archive/main.zip' -o redis-llm.zip;\
        unzip redis-llm.zip;\
        cd redis-llm-main;\
        mkdir compile;\
        cd compile;\
        cmake ..;\
        make;

# Load redis-llm
ENV REDISDIR /usr/lib/redis
RUN set -ex;\
        mkdir -p "$REDISDIR/modules" "$REDISDIR/conf";\
        cp /usr/src/redis-llm-main/compile/libredis-llm.so "$REDISDIR/modules";\
        echo 'loadmodule /usr/lib/redis/modules/libredis-llm.so' > "$REDISDIR/conf/redis.conf";

# Cleanup
RUN set -ex;\
	deps="$DEPS";\
	apt-get purge -y --auto-remove $deps;\
	rm -rf /usr/src/*;

CMD ["redis-server", "/usr/lib/redis/conf/redis.conf"]
