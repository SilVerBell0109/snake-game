FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libncursesw5-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY snake-game/ .

RUN make

CMD ["./snake"]
