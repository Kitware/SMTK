FROM fedora:31 AS sccache-builder
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_sccache.sh /root/install_sccache.sh
RUN sh /root/install_sccache.sh

FROM fedora:31
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh

ENV SUPERBUILD_PREFIX /root/misc/root/smtk-deps
COPY install_superbuild.sh /root/install_superbuild.sh
RUN sh /root/install_superbuild.sh

# Install sccache.
COPY --from=sccache-builder /usr/local/bin/sccache /usr/local/bin/sccache
