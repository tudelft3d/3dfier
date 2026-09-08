# syntax=docker/dockerfile:1

FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libboost-program-options-dev \
    libboost-filesystem-dev \
    libboost-locale-dev \
    libboost-chrono-dev \
    libcgal-dev \
    libgdal-dev \
    libpdal-dev \
    libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

COPY . /src

RUN cmake -S /src -B /build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /build --parallel "$(nproc)"

FROM ubuntu:22.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    libboost-chrono1.74.0 \
    libboost-filesystem1.74.0 \
    libboost-locale1.74.0 \
    libboost-program-options1.74.0 \
    libgdal30 \
    libgmp10 \
    libmpfr6 \
    libpdal-base13 \
    libpdal-util13 \
    libyaml-cpp0.7 \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --create-home --user-group 3dfier

COPY --from=builder /build/3dfier /usr/local/bin/3dfier

USER 3dfier
WORKDIR /data

ENTRYPOINT ["/usr/local/bin/3dfier"]
