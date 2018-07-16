# This dockerfile defines the expected runtime environment before the project is installed
FROM ubuntu:xenial as builder
LABEL maintainer="Balázs Dukai <b.dukai@tudelft.nl>"
LABEL description="Requirements for building 3dfier"

# required repositories
RUN set -x; \
    apt-get clean \
    && apt-get update; \
    apt-get install -y software-properties-common; \
    add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && add-apt-repository -y ppa:ubuntugis/ubuntugis-unstable \
    && apt-get update

# C++ packages
RUN set -x \
    && apt-get install -y --no-install-recommends --no-install-suggests \
        gcc-6 \
        g++-6 \
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
        unzip \
        wget \
    && apt-get update \
    && apt-get upgrade -y

# GIS packages
RUN set -x \
    && apt-get install -y \
        libgdal-dev \
        libproj-dev \
        libgeotiff-dev \
    && apt-get update \
    && apt-get upgrade -y \
    && apt-get autoremove --purge -y \
    && apt-get autoclean -y \
    && rm -rf /var/cache/apt/* /tmp/*


RUN export CC=gcc-6 && export CXX=g++-6

#FROM builder as liblas

ENV IN_DIR "/opt"

# get LASzip
RUN set -ex; \
    cd $IN_DIR; \
    wget https://github.com/LASzip/LASzip/releases/download/v2.2.0/laszip-src-2.2.0.tar.gz; \
    wget https://github.com/LASzip/LASzip/releases/download/v2.2.0/laszip-src-2.2.0.tar.gz.md5; \
    md5sum -c laszip-src-2.2.0.tar.gz.md5; \
    rm laszip-src-2.2.0.tar.gz.md5; \
    tar -xf laszip-src-2.2.0.tar.gz; \
    rm laszip-src-2.2.0.tar.gz

# The Makefile need to be modified in order to be compliant to what libLAS
# is looking for. https://github.com/libLAS/libLAS/issues/9
RUN set -x; \
    cd $IN_DIR/laszip-src-2.2.0; \
    sed -i 's/laszipdir = $(includedir)\//laszipdir = $(includedir)\/laszip/' ./include/laszip/Makefile.am; \
    sed -i 's/laszipdir = $(includedir)\//laszipdir = $(includedir)\/laszip/' ./include/laszip/Makefile.in; \
    mkdir build; \
    ./configure --prefix=$IN_DIR/laszip-src-2.2.0/build; \
    make && make install && make clean

#Libraries have been installed in:
#   /home/bdukai/Desktop/laszip-src-2.2.0/build/lib
#
#If you ever happen to want to link against installed libraries
#in a given directory, LIBDIR, you must either use libtool, and
#specify the full pathname of the library, or use the `-LLIBDIR'
#flag during linking and do at least one of the following:
#   - add LIBDIR to the `LD_LIBRARY_PATH' environment variable
#     during execution
#   - add LIBDIR to the `LD_RUN_PATH' environment variable
#     during linking
#   - use the `-Wl,-rpath -Wl,LIBDIR' linker flag
#   - have your system administrator add LIBDIR to `/etc/ld.so.conf'
#
#See any operating system documentation about shared libraries for
#more information, such as the ld(1) and ld.so(8) manual pages.

RUN set -x; \
    cd $IN_DIR; \
    wget http://download.osgeo.org/liblas/libLAS-1.8.1.tar.bz2; \
    # for some reason the md5 file was not accessible on osgeo when I checked it
    # wget http://download2.osgeo.org/liblas/libLAS-1.8.1.tar.bz2.md5
    # md5sum -c libLAS-1.8.1.tar.bz2.md5
    # rm libLAS-1.8.1.tar.bz2.md5
    tar -xf libLAS-1.8.1.tar.bz2; \
    rm libLAS-1.8.1.tar.bz2; \
    cd libLAS-1.8.1; \
    mkdir build; \
    mkdir cmake_build; \
    cd cmake_build; \
    cmake .. \
    -DCMAKE_INSTALL_PREFIX=$IN_DIR/libLAS-1.8.1/build \
    -DWITH_GDAL=ON \
    -DWITH_LASZIP=ON \
    -DLASZIP_INCLUDE_DIR=$IN_DIR/laszip-src-2.2.0/build/include \
    -DLASZIP_LIBRARY=$IN_DIR/laszip-src-2.2.0/build/lib/liblaszip.so \
    ; \
    make && make install && make clean
#    make install; \
#    make clean; \
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$IN_DIR/libLAS-1.8.1/build/lib; \
#    # test installation, should GDAL, LASzip should be listed
#    $IN_DIR/libLAS-1.8.1/build/bin/lasinfo -h | grep -iq "lasinfo (libLAS 1.8.1 with GeoTIFF 1.4.0 GDAL 2.1.0 LASzip 2.2.0)"; \
#    export CC=gcc-6; \
#    export CXX=g++-6

#FROM builder as cgal

WORKDIR /opt/

RUN set -x; \
    wget https://github.com/CGAL/cgal/archive/releases/CGAL-4.10.tar.gz; \
    wget https://github.com/CGAL/cgal/releases/download/releases%2FCGAL-4.10/md5sum.txt; \
    md5sum -c md5sum.txt; \
    rm md5sum.txt; \
    tar -xf CGAL-4.10.tar.gz; \
    rm CGAL-4.10.tar.gz; \
    cd cgal-releases-CGAL-4.10; \
    mkdir build; \
    cd build; \
    cmake ..; \
    make

RUN useradd -ms /bin/bash 3dfier
USER 3dfier
WORKDIR /home/3dfier

