FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libncursesw5-dev \
    locales \
    && locale-gen en_US.UTF-8 \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8
ENV TERM=xterm-256color

WORKDIR /app
COPY . .
RUN cd snake-game && make clean && make

WORKDIR /app/snake-game
CMD ["./snake"]
