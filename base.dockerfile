FROM alpine:3.15
LABEL org.opencontainers.image.authors="Balázs Dukai <balazs.dukai@3dgi.nl>"
LABEL org.opencontainers.image.source="https://github.com/tudelft3d/3dfier"
LABEL org.opencontainers.image.vendor="3DGI"
LABEL org.opencontainers.image.title="3dfier builder"
LABEL org.opencontainers.image.description="Base image for building 3dfier"
LABEL org.opencontainers.image.licenses="GPL-3.0"
LABEL org.opencontainers.image.url="http://tudelft3d.github.io/3dfier"

ARG JOBS

#
# 1 Install FreeXL
#
RUN apk --update add freexl

#
# 2 Install proj
#
ARG PROJ_VERSION=6.1.1
RUN apk --update add sqlite libstdc++ sqlite-libs libgcc && \
    apk --update add --virtual .proj4-deps \
        make \
        gcc \
        g++ \
        file \
        sqlite-dev \
        unzip && \
    cd /tmp && \
    wget http://download.osgeo.org/proj/proj-${PROJ_VERSION}.tar.gz && \
    tar xfvz proj-${PROJ_VERSION}.tar.gz && \
    rm -f proj-${PROJ_VERSION}.tar.gz && \
    wget http://download.osgeo.org/proj/proj-datumgrid-1.8.zip && \
    unzip proj-datumgrid-1.8.zip -d proj-${PROJ_VERSION}/nad/ && \
    rm -f proj-datumgrid-1.8.zip && \
    wget http://download.osgeo.org/proj/proj-datumgrid-europe-1.1.zip && \
    unzip proj-datumgrid-europe-1.1.zip -d proj-${PROJ_VERSION}/nad/ && \
    rm -f proj-datumgrid-europe-1.1.zip && \
    wget http://download.osgeo.org/proj/proj-datumgrid-north-america-1.1.zip && \
    unzip proj-datumgrid-north-america-1.1.zip -d proj-${PROJ_VERSION}/nad/ && \
    rm -f proj-datumgrid-north-america-1.1.zip && \
    wget http://download.osgeo.org/proj/proj-datumgrid-oceania-1.0.zip && \
    unzip proj-datumgrid-oceania-1.0.zip -d proj-${PROJ_VERSION}/nad/ && \
    rm -f proj-datumgrid-oceania-1.0.zip && \
    cd proj-${PROJ_VERSION} && \
    ./configure && \
    make && \
    make install && \
    echo "Entering root folder" && \
    cd / &&\
    echo "Cleaning dependencies tmp and manuals" && \
    apk del .proj4-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib/libproj*; do strip -s $i 2>/dev/null || /bin/true; done && \
    for i in /usr/local/lib/geod*; do strip -s $i 2>/dev/null || /bin/true; done && \
    for i in /usr/local/bin/proj*; do strip -s $i 2>/dev/null || /bin/true; done && \
    proj

# 3 Install geos
ARG GEOS_VERSION=3.7.2
RUN apk --update add --virtual .geos-deps \
        which \
        make \
        gcc \
        g++ \
        file \
        git \
        autoconf \
        automake \
        libtool && \
    cd /tmp && \
    git clone https://git.osgeo.org/gitea/geos/geos.git geos && \
    cd geos && \
    git checkout ${GEOS_VERSION} && \
    ./autogen.sh && \
    ./configure && \
    make && \
    make install && \
    cd ~ && \
    apk del .geos-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib/libgeos*; do strip -s $i 2>/dev/null || /bin/true; done && \
    for i in /usr/local/bin/geos-config*; do strip -s $i 2>/dev/null || /bin/true; done


#
# 5 Install geotiff
#
ARG GEOTIFF_VERSION=1.5.1
RUN apk --update add \
        zlib \
        tiff \
        libjpeg && \
    apk --update add --virtual .geotiff-deps \
        zlib \
        make \
        gcc \
        g++ \
        file \
        zlib-dev \
        tiff-dev \
        sqlite-dev \
        jpeg-dev && \
    cd /tmp && \
    wget http://download.osgeo.org/geotiff/libgeotiff/libgeotiff-${GEOTIFF_VERSION}.tar.gz && \
    tar xfz libgeotiff-${GEOTIFF_VERSION}.tar.gz  && \
    rm -f libgeotiff-${GEOTIFF_VERSION}.tar.gz && \
    cd libgeotiff-${GEOTIFF_VERSION} && \
    ./configure --with-jpeg=yes --with-zlib=yes --with-proj=/usr/local && \
    make && \
    make install && \
    cd / && \
    apk del .geotiff-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib/libgeotiff*; do strip -s $i 2>/dev/null || /bin/true; done

#
# 6 Install Boost
#
ARG BOOST_VERSION=1_77_0
RUN apk del boost boost-dev && \
    apk --update add \
        zlib \
        zstd \
        xz \
        icu \
        bzip2 \
        mpfr4 \
        eigen && \
    apk --update add --virtual .boost-deps \
        zlib-dev \
        zstd-dev \
        xz-dev \
        icu-dev \
        bzip2-dev \
        make \
        gcc \
        g++ \
        git \
        cmake \
        linux-headers && \
    cd /tmp && \
    wget https://boostorg.jfrog.io/artifactory/main/release/1.77.0/source/boost_${BOOST_VERSION}.tar.gz && \
    tar xzf boost_${BOOST_VERSION}.tar.gz && \
    cd boost_${BOOST_VERSION} && \
    ./bootstrap.sh \
        --with-libraries=all \
        --libdir=/usr/local/lib \
        --includedir=/usr/local/include \
        --exec-prefix=/usr/local && \
    ./b2 \
        --libdir=/usr/local/lib \
        --includedir=/usr/local/include \
        --exec-prefix=/usr/local \
        install && \
    cd ~ && \
    apk del .boost-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib/libboost*; do strip -s $i 2>/dev/null || /bin/true; done

#
# 7 Install LASTools
#
ARG LASTOOLS_VERSION=9ecb4e682153436b044adaeb3b4bfdf556109a0f
RUN apk --update add --virtual .lastools-deps \
        which \
        make \
        cmake \
        gcc \
        g++ \
        file \
        git \
        libtool && \
    cd /tmp && \
    git clone https://github.com/LAStools/LAStools.git lastools && \
    cd lastools && \
    git checkout ${LASTOOLS_VERSION} && \
    mkdir "_build" && \
    cd "_build" && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j $JOBS && \
    make install && \
    apk del .lastools-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man

#
# 8 Install CGAL
#
ARG CGAL_VERSION=5.3
RUN apk --update add \
        gmp \
        mpfr4 \
        eigen \
        zlib && \
    apk --update add --virtual .cgal-deps \
        make \
        gcc \
        gmp-dev \
        mpfr-dev \
        eigen-dev \
        zlib-dev \
        g++ \
        git \
        cmake \
        linux-headers && \
    cd /tmp && \
    wget https://github.com/CGAL/cgal/releases/download/v${CGAL_VERSION}/CGAL-${CGAL_VERSION}.tar.xz && \
    tar xf CGAL-${CGAL_VERSION}.tar.xz && \
    cd CGAL-${CGAL_VERSION} && \
    mkdir build && \
    cd build && \
    cmake \
        -DBoost_NO_BOOST_CMAKE=ON \
        -DBoost_NO_SYSTEM_PATHS=ON \
        -DBOOST_ROOT=/usr/local \
        -DCMAKE_BUILD_TYPE=Release \
        -DWITH_examples=OFF \
        -DWITH_demos=OFF \
        -DWITH_CGAL_Core=ON \
        -DWITH_CGAL_Qt5=OFF \
        -DWITH_CGAL_ImageIO=OFF \
        .. && \
    make -j $JOBS && \
    make install && \
    cd ~ && \
    apk del .cgal-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib64/libCGAL*; do strip -s $i 2>/dev/null || /bin/true; done

#
# 9 Install SFCGAL
#
ARG SFCGAL_VERSION=v1.4.1
RUN apk --update add \
        gmp \
        mpfr4 \
        zlib && \
    apk --update add --virtual .sfcgal-deps \
        make \
        gcc \
        gmp-dev \
        mpfr-dev \
        zlib-dev \
        g++ \
        git \
        cmake \
        linux-headers && \
    cd /tmp && \
    git clone https://gitlab.com/Oslandia/SFCGAL.git && \
    cd SFCGAL && \
    git checkout tags/${SFCGAL_VERSION} && \
    mkdir build && \
    cd build && \
    cmake \
        -DBoost_NO_BOOST_CMAKE=TRUE \
        -DBoost_NO_SYSTEM_PATHS=TRUE \
        -DBOOST_ROOT=/usr/local \
        .. && \
    make && \
    make install && \
    cd ~ && \
    apk del .sfcgal-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib64/libSFCGAL*; do strip -s $i 2>/dev/null || /bin/true; done

#
# 10 Install PostGIS
#
ARG POSTGIS_VERSION=3.1.4
# RUN ln -sf /usr/local/lib64/libSFCGAL.so /usr/local/lib && \
RUN apk --update add \
        curl \
        nghttp2 \
        zlib \
        zstd \
        xz \
        icu \
        bzip2 \
        mpfr4 \
        perl \
        json-c \
        libxml2 \
        sqlite \
        postgresql && \
    apk --update add --virtual .postgis-deps \
        curl-dev \
        nghttp2-dev \
        zlib-dev \
        zstd-dev \
        xz-dev \
        icu-dev \
        bzip2-dev \
        mpfr-dev \
        git \
        make \
        wget \
        gcc \
        g++ \
        file \
        perl-dev \
        json-c-dev \
        libxml2-dev \
        sqlite-dev \
        postgresql-dev \
        tiff-dev \
        portablexdr-dev \
        linux-headers && \
    cd /tmp && \
    wget http://download.osgeo.org/postgis/source/postgis-${POSTGIS_VERSION}.tar.gz && \
    tar xzf postgis-${POSTGIS_VERSION}.tar.gz && \
    cd postgis-${POSTGIS_VERSION} && \
    ./configure \
        --without-raster \
        --without-topology  \
        --without-address-standardizer \
        --without-phony-revision \
        --without-protobuf && \
    make -j $JOBS && \
    make install && \
    cd ~ && \
    apk del .postgis-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man

#
# 11 Install GDAL
#
ARG GDAL_VERSION=3.4.0
RUN apk --update add \
        xz-dev \
        zstd-dev \
        sqlite-dev \
        postgresql-dev \
        portablexdr-dev && \
    apk --update add --virtual .gdal-deps \
        make \
        gcc \
        g++ \
        file \
        linux-headers && \
    cd /tmp && \
    wget http://download.osgeo.org/gdal/${GDAL_VERSION}/gdal-${GDAL_VERSION}.tar.gz && \
    tar xzf gdal-${GDAL_VERSION}.tar.gz && \
    rm -f gdal-${GDAL_VERSION}.tar.gz && \
    cd gdal-${GDAL_VERSION} && \
    ./configure \
        PQ_CFLAGS="-I/usr/include/postgresql" \
        PQ_LIBS="-L/usr/lib/postgresql14 -lpq" \
        CFLAGS="-g -O3" \
        CXXFLAGS="-g -O3" \
        --enable-lto && \
    make -j $JOBS && \
    make install && \
    cd ~ && \
    apk del .gdal-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib/libgdal*; do strip -s $i 2>/dev/null || /bin/true; done && \
    for i in /usr/local/bin/gdal*; do strip -s $i 2>/dev/null || /bin/true; done

#
# 12 Additional build dependencies
#
RUN ln -s /usr/local/lib64/libCGAL.so.13 /usr/local/lib && \
    ln -s /usr/local/lib64/libSFCGAL.so.1 /usr/local/lib && \
    apk --update add \
        gmp \
        mpfr4 \
        yaml \
        bash \
        yaml-cpp && \
    apk --update add --virtual .3dfier-deps \
        make \
        gmp-dev \
        mpfr-dev \
        yaml-dev \
        yaml-cpp-dev \
        gcc \
        g++ \
        gdb \
        cmake \
        linux-headers