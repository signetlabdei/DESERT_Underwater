FROM debian:bookworm-slim AS builder

ENV LANG C.UTF-8
ENV TZ=UTC
ARG DESERT_TAG=master

RUN DEBIAN_FRONTEND=noninteractive apt-get update \
        && apt-get install -y --no-install-recommends \
                autoconf \
                automake \
                bash \
                bison \
                build-essential \
                ca-certificates \
                flex \
                gfortran \
                git \
                libtool \
                libx11-dev \
                libxml2-dev \
                libxmu-dev \
                libxmu-headers \
                libxt-dev \
        && rm -rf /var/lib/apt/lists/*

WORKDIR /work
RUN git clone --depth 1 --single-branch --branch "${DESERT_TAG}" https://github.com/signetlabdei/DESERT_Underwater.git \
        && cd DESERT_Underwater/DESERT_Framework \
        && ./install.sh --target LOCAL --with-woss --inst_mode release --addons ALL --dest_folder /work/DESERT_Underwater/DESERT_buildCopy_LOCAL \
        && cd /work/DESERT_Underwater/DESERT_buildCopy_LOCAL \
        && ./make_environment.sh

FROM debian:bookworm-slim

ENV LANG C.UTF-8
ENV TZ=UTC

RUN DEBIAN_FRONTEND=noninteractive apt-get update \
        && apt-get install -y --no-install-recommends \
                bash \
                libgfortran5 \
                libstdc++6 \
                libx11-6 \
                libxml2 \
                libxmu6 \
                libxt6 \
        && rm -rf /var/lib/apt/lists/*

COPY --from=builder /work/DESERT_Underwater/DESERT_buildCopy_LOCAL/bin /work/DESERT_Underwater/DESERT_buildCopy_LOCAL/bin
COPY --from=builder /work/DESERT_Underwater/DESERT_buildCopy_LOCAL/lib /work/DESERT_Underwater/DESERT_buildCopy_LOCAL/lib
COPY --from=builder /work/DESERT_Underwater/DESERT_buildCopy_LOCAL/environment /work/DESERT_Underwater/DESERT_buildCopy_LOCAL/environment
COPY --from=builder /work/DESERT_Underwater/DESERT_Framework/DESERT/samples/ /work/DESERT_Underwater/DESERT_Framework/DESERT/samples

WORKDIR /work/DESERT_Underwater/DESERT_Framework/DESERT/samples/desert_samples

ENTRYPOINT ["/bin/bash", "-c", "source /work/DESERT_Underwater/DESERT_buildCopy_LOCAL/environment && exec \"$@\"", "--"]
