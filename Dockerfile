# syntax=docker/dockerfile:1

ARG UBUNTU_VERSION=24.04
ARG VCPKG_ROOT=/opt/vcpkg
ARG VCPKG_TRIPLET=x64-linux

# ---------- Builder ----------
FROM ubuntu:${UBUNTU_VERSION} AS builder
ARG VCPKG_ROOT
ARG VCPKG_TRIPLET

ENV DEBIAN_FRONTEND=noninteractive
ENV VCPKG_FEATURE_FLAGS=manifests

RUN apt-get update && apt-get install -y --no-install-recommends \
    software-properties-common \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/* \
 && add-apt-repository -y ppa:ubuntu-toolchain-r/test \
 && apt-get update && apt-get install -y --no-install-recommends \
    gcc-14 \
    g++-14 \
    build-essential \
    cmake \
    git \
    curl \
    zip \
    unzip \
    pkg-config \
 && rm -rf /var/lib/apt/lists/*

ENV CC=gcc-14
ENV CXX=g++-14

# Install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT} \
 && ${VCPKG_ROOT}/bootstrap-vcpkg.sh -disableMetrics

# Copy project files
WORKDIR /app
COPY CMakeLists.txt ./
COPY vcpkg.json vcpkg-configuration.json ./

# Install dependencies declared in vcpkg.json (cache-friendly: only manifests copied)
RUN ${VCPKG_ROOT}/vcpkg install --clean-after-build --triplet ${VCPKG_TRIPLET} --x-manifest-root=/app

# Project sources (later layer to preserve cache on code changes)
COPY . .

# ECS dependency: fallback clone if submodule not present
RUN if [ ! -d ./ECS/.git ]; then \
    rm -rf ./ECS && git clone --depth 1 https://github.com/TrueMoonn/ECS.git ./ECS; \
  fi

# Configure & build
RUN rm -rf build
RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=${VCPKG_TRIPLET}
RUN cmake --build build --config Release -j"$(nproc)"

# ---------- Runtime ----------
FROM ubuntu:${UBUNTU_VERSION} AS runtime
ARG VCPKG_ROOT
ARG VCPKG_TRIPLET

ENV LD_LIBRARY_PATH=${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/lib

RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

# Copy runtime dependencies and binary
COPY --from=builder ${VCPKG_ROOT} ${VCPKG_ROOT}
COPY --from=builder /app/dengine /usr/local/bin/dengine

EXPOSE 6767
CMD ["/usr/local/bin/dengine"]
