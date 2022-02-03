FROM tudelft3d/3dfier:base AS builder

ARG JOBS

#
# 13 Install 3dfier
#
COPY . /tmp
RUN cd /tmp && \
    mkdir build && \
    cd build && \
    cmake \
        -DCGAL_DIR=/usr/local \
        -DCMAKE_BUILD_TYPE=Release \
        -DBoost_NO_BOOST_CMAKE=TRUE \
        -DBoost_NO_SYSTEM_PATHS=TRUE \
        -DBOOST_ROOT=/usr/local \
        .. && \
    make -j $JOBS && \
    make install && \
    cd ~ && \
    apk del .3dfier-deps && \
    rm -rf /tmp/* && \
    rm -rf /user/local/man

RUN 3dfier --version

# removing unnecessary headers
RUN rm -rf /usr/local/include

RUN mkdir /data && \
    chown 1001 /data && \
    chgrp 0 /data && \
    chmod g=u /data && \
    chgrp 0 /etc/passwd && \
    chmod g=u /etc/passwd

#
# Export the dependencies
#
RUN mkdir /export
COPY strip-docker-image-export /tmp
RUN bash /tmp/strip-docker-image-export \
    -v \
    -d /export \
    -f /bin/bash \
    -f /usr/bin/awk \
    -f /usr/bin/id \
    -f /etc/passwd \
    -f /bin/ls \
    -f /data \
    -f /usr/local/share/proj/proj.db \
    -f /usr/local/bin/3dfier


FROM scratch AS exe
LABEL org.opencontainers.image.authors="Balázs Dukai <balazs.dukai@3dgi.nl>"
LABEL org.opencontainers.image.source="https://github.com/tudelft3d/3dfier"
LABEL org.opencontainers.image.vendor="3DGI"
LABEL org.opencontainers.image.title="3dfier"
LABEL org.opencontainers.image.description="The open-source tool for creating 3D models"
LABEL org.opencontainers.image.licenses="GPL-3.0"
LABEL org.opencontainers.image.url="http://tudelft3d.github.io/3dfier"

COPY --from=builder /export/ /
COPY --chown=1001:0 uid_entrypoint.sh /usr/local/bin/

USER 1001

WORKDIR /data

ENTRYPOINT ["/usr/local/bin/uid_entrypoint.sh"]

CMD ["3dfier"]