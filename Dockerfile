# This dockerfile defines the expected runtime environment before the project is installed
FROM ubuntu:bionic as builder
LABEL maintainer="Balázs Dukai <b.dukai@tudelft.nl>"
LABEL description="Build environment for 3dfier, includes PostgreSQL"

# required repositories
RUN set -x; \
    apt-get clean && apt-get update; \
    apt-get install -y software-properties-common; \
    apt-get install -y tzdata; \
    echo "Europe/Amsterdam" > /etc/timezone && dpkg-reconfigure -f noninteractive tzdata

# C++ packages
RUN set -x \
    && apt-get install -y --no-install-recommends --no-install-suggests \
        gcc \
        g++ \
        cmake \
        libboost-dev \
        libboost-filesystem-dev \
        libboost-locale-dev \
        libboost-thread-dev \
        libboost-iostreams-dev \
        libboost-program-options-dev \
        libyaml-cpp-dev \
        libgmp-dev \
        libmpfr-dev \
        gdbserver \
        wget

# PostgreSQL
RUN set -x \
    && apt-get install -y postgresql 

# GIS packages
RUN set -x \
    && apt-get install -y \
        libgdal-dev \
        libproj-dev \
    && apt-get autoremove --purge -y \
    && apt-get autoclean -y \
    && rm -rf /var/cache/apt/* /tmp/*

WORKDIR /opt

# LASTools
RUN set -x; \
    wget -O lastools.tar.gz -L -q https://api.github.com/repos/CGAL/LAStools/tarball/master; \
    mkdir lastools && mkdir lastools/build; \
    tar --strip-components=1 -xf lastools.tar.gz -C lastools; \
    rm lastools.tar.gz; \
    cd lastools/build; \
    cmake .. && make; \
    make install; \
    cd /opt
    

# CGAL
RUN set -x; \
    cd /opt; \
    wget -O cgal.tar.gz -L -q https://github.com/CGAL/cgal/archive/releases/CGAL-4.12.tar.gz; \
    mkdir cgal && mkdir cgal/build; \
    tar --strip-components=1 -xf cgal.tar.gz -C cgal; \
    rm cgal.tar.gz; \
    cd cgal/build; \
    cmake .. -DCMAKE_BUILD_TYPE=Release && make; \
    make install; \
    cd /opt

RUN rm -rf /var/cache/apt/* /tmp/*

RUN useradd -ms /bin/bash 3dfier
USER 3dfier
WORKDIR /home/3dfier
