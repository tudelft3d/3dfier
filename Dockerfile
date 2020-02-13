FROM alpine:3.10

#
# 1 Install FreeXL
#
ARG FREEXL_VERSION=1.0.5
RUN apk --update add --virtual .freexl-deps \
        make \
        gcc \
        g++ \
        wget \
        autoconf \
        automake \
        libtool && \
    cd /tmp && \
    wget http://www.gaia-gis.it/gaia-sins/freexl-${FREEXL_VERSION}.tar.gz && \
    tar xfz freexl-${FREEXL_VERSION}.tar.gz  && \
    cd freexl-${FREEXL_VERSION} && \
    ./configure && \
    make && \
    make install && \
    cd ~ && \
    apk del .freexl-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib/libfreexl*; do strip -s $i 2>/dev/null || /bin/true; done

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
# 4 Install Spatialite
# use straight current version of repository because there was no release since august 2018
#
ARG SPATIALITE_VERSION=latest
RUN apk --update add \
        gnu-libiconv \
        zlib \
        libxml2 \
        sqlite && \
    apk --update add --virtual .spatialite-deps \
        gnu-libiconv-dev \
        zlib-dev \
        libxml2-dev \
        sqlite-libs \
        fossil \
        sqlite-dev \
        make \
        gcc \
        g++ \
        wget \
        autoconf \
        automake \
        libtool && \
    cd /tmp && \
    fossil clone --admin-user docker-user https://www.gaia-gis.it/fossil/libspatialite libspatialite.fossil && \
    mkdir libspatialite && \
    cd libspatialite && \
    fossil open ../libspatialite.fossil && \
    ./configure && \
    make && \
    make install && \
    cd ~ && \
    apk del .spatialite-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib/libspatialite*; do strip -s $i 2>/dev/null || /bin/true; done && \
    for i in /usr/local/lib/mod_spatialite*; do strip -s $i 2>/dev/null || /bin/true; done

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
ARG BOOST_VERSION=1_71_0
RUN apk del boost boost-dev && \
    apk --update add \
        zlib \
        zstd \
        xz \
        icu \
        bzip2 \
        mpfr3 \
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
    wget https://dl.bintray.com/boostorg/release/1.71.0/source/boost_${BOOST_VERSION}.tar.gz && \
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
# 7 Install LasZip
#
ARG LASZIP_VERSION=3.4.1
RUN apk --update add --virtual .laszip-deps \
        make \
        gcc \
        g++ \
        file \
        git \
        cmake \
        linux-headers && \
    cd /tmp && \
    git clone https://github.com/LASzip/LASzip.git && \
    cd LASzip && \
    git checkout tags/${LASZIP_VERSION} && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install && \
    cd ~ && \
    apk del .laszip-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib/liblaszip*; do strip -s $i 2>/dev/null || /bin/true; done && \
    for i in /usr/local/bin/las*; do strip -s $i 2>/dev/null || /bin/true; done

#
# 8 Install LibLas
#
RUN apk --update add --virtual .liblas-deps \
        make \
        gcc \
        g++ \
        git \
        cmake \
        linux-headers && \
    cd /tmp && \
    git clone https://github.com/LAStools/LAStools.git las_tools && \
    cd las_tools && \
    mkdir build && \
    cd build && \
    cmake -G "Unix Makefiles" ../ && \
    make && \
    make install && \
    cd ~ && \
    apk del .liblas-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib/LASlib/*; do strip -s $i 2>/dev/null || /bin/true; done

#
# 9 Install CGAL
#
ARG CGAL_VERSION=releases/CGAL-4.14.1
RUN apk --update add \
        gmp \
        mpfr3 \
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
    git clone https://github.com/CGAL/cgal.git cgal && \
    cd cgal && \
    git checkout tags/${CGAL_VERSION} && \
    mkdir build && \
    cd build && \
    cmake \
        -DBoost_NO_BOOST_CMAKE=TRUE \
        -DBoost_NO_SYSTEM_PATHS=TRUE \
        -DBOOST_ROOT=/usr/local \
        -DCMAKE_BUILD_TYPE=Release .. && \
    make && \
    make install && \
    cd ~ && \
    apk del .cgal-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib64/libCGAL*; do strip -s $i 2>/dev/null || /bin/true; done

#
# 11 Install PostGIS
#
ARG POSTGIS_VERSION=2.5.3
RUN ln -sf /usr/local/lib64/libSFCGAL.so /usr/local/lib && \
    apk --update add \
        curl \
        nghttp2 \
        zlib \
        zstd \
        xz \
        icu \
        bzip2 \
        mpfr3 \
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
        --without-raster && \
    make && \
    make install && \
    cd ~ && \
    apk del .postgis-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man

#
# 12 Install GDAL
#
ARG GDAL_VERSION=3.0.2
RUN ln -sf /usr/local/lib64/libSFCGAL.so /usr/local/lib && \
    apk --update add \
        xz \
        zstd \
        curl \
        libxml2 \
        sqlite \
        tiff \
        postgresql && \
    apk --update add --virtual .gdal-deps \
        xz-dev \
        zstd-dev \
        curl-dev \
        libxml2-dev \
        sqlite-dev \
        make \
        gcc \
        g++ \
        file \
        tiff-dev \
        postgresql-dev \
        portablexdr-dev \
        linux-headers && \
    cd /tmp && \
    wget http://download.osgeo.org/gdal/${GDAL_VERSION}/gdal-${GDAL_VERSION}.tar.gz && \
    tar xzf gdal-${GDAL_VERSION}.tar.gz && \
    rm -f gdal-${GDAL_VERSION}.tar.gz && \
    cd gdal-${GDAL_VERSION} && \
    ./configure \
        --with-proj=/usr/local \
        --with-liblzma=yes \
        --with-zstd=yes \
        --with-pg=yes \
        --with-geotiff=/usr/local \
        --with-curl=/usr/local \
        --with-xml2=/usr/local \
        --with-sqlite3=yes \
        --with-spatialite=yes \
        --with-sfcgal=yes \
        --with-geos=/usr/local/bin/geos-config && \
    make -j 4 && \
    make install && \
    cd ~ && \
    apk del .gdal-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    for i in /usr/local/lib/libgdal*; do strip -s $i 2>/dev/null || /bin/true; done && \
    for i in /usr/local/bin/gdal*; do strip -s $i 2>/dev/null || /bin/true; done

#
# 13 Install 3dfier
#
COPY . /tmp
RUN ln -s /usr/local/lib64/libCGAL.so.13 /usr/local/lib && \
    ln -s /usr/local/lib64/libSFCGAL.so.1 /usr/local/lib && \
    apk --update add \
        gmp \
        mpfr3 \
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
        cmake \
        linux-headers && \
    cd /tmp && \
    mkdir build && \
    cd build && \
    cmake \
        -DCGAL_DIR=/usr/local \
        -DCMAKE_BUILD_TYPE=Release \
        -DBoost_NO_BOOST_CMAKE=TRUE \
        -DBoost_NO_SYSTEM_PATHS=TRUE \
        -DBOOST_ROOT=/usr/local \
        .. && \
    make && \
    make install && \
    cd ~ && \
    apk del .3dfier-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man && \
    3dfier --help

RUN mkdir /data && \
    chown 1001 /data && \
    chgrp 0 /data && \
    chmod g=u /data && \
    chgrp 0 /etc/passwd && \
    chmod g=u /etc/passwd

# removing unnecessary headers
RUN rm -rf /usr/local/include

USER 1001

COPY --chown=1001:0 uid_entrypoint.sh /usr/local/bin/

WORKDIR /data

ENTRYPOINT ["/usr/local/bin/uid_entrypoint.sh"]

CMD ["3dfier --help"]
