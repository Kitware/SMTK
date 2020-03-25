FROM rust:latest AS build
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

RUN cargo install \
    --git https://gitlab.kitware.com/utils/rust-ghostflow \
    --branch master \
    --no-default-features \
    -- ghostflow-cli

FROM debian:stretch
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

RUN echo "deb http://deb.debian.org/debian stretch-backports main" > /etc/apt/sources.list.d/stretch-backports.list
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        python3-pip python3-setuptools \
        clang-format-3.8 \
        bind9-host ca-certificates && \
    apt-get install -y --no-install-recommends \
        -t stretch-backports \
        git-core git-lfs && \
    apt-get clean
RUN pip3 install autopep8
RUN git config --global user.name "Ghostflow Container" && \
    git config --global user.email "ghostflow@nowhere"
RUN git clone \
        https://gitlab.kitware.com/utils/source-formatters.git \
        /root/source-formatters
COPY --from=build /usr/local/cargo/bin/ghostflow-cli /usr/bin/ghostflow-cli

VOLUME /repo
WORKDIR /repo
