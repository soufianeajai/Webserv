FROM debian:bullseye-slim

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    php-cgi \
    python3 \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN ln -s /app/www /www

RUN make clean && make && chmod +x webserv

EXPOSE 2020

ENTRYPOINT ["./webserv", "configFiles/default.config"]