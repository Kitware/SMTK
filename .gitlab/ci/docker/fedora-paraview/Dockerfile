FROM fedora:41
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh

VOLUME /root/helpers
ARG SCCACHE_REDIS

ARG superbuild_ref=origin/master
ENV SUPERBUILD_PREFIX /root/misc/root/smtk-deps
COPY install_superbuild.sh /root/install_superbuild.sh
RUN sh /root/install_superbuild.sh "$superbuild_ref"
