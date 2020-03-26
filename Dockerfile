FROM tudelft3d/3dfier:base
LABEL maintainer.email="b.dukai@tudelft.nl" maintainer.name="Balázs Dukai"
LABEL description="Image for running 3dfier"
LABEL org.name="3D Geoinformation Research Group, Delft University of Technology, Netherlands" org.website="https://3d.bk.tudelft.nl/"
LABEL website="http://tudelft3d.github.io/3dfier"

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

COPY --chown=1001:0 uid_entrypoint.sh /usr/local/bin/

USER 1001

WORKDIR /data

ENTRYPOINT ["/usr/local/bin/uid_entrypoint.sh"]

CMD ["3dfier --help"]
