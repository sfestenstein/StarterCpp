# =============================================================================
# StarterCpp Development Container
#
# All C++ dependencies are built from source for exact version control.
# Conan is NOT used — the container IS the dependency manager.
#
# Build:
#   docker build -t startercpp-dev .
#
# Run (mount source as volume):
#   docker run -v $(pwd):/workspace -w /workspace -it startercpp-dev
#
# Build inside container:
#   cmake --preset container-debug
#   cmake --build --preset container-debug
#   ctest --preset container-debug
# =============================================================================

FROM ubuntu:24.04

# ---- Dependency versions (override with --build-arg) -----------------------
ARG ABSEIL_VERSION=20240116.2
ARG PROTOBUF_VERSION=v27.0
ARG SPDLOG_VERSION=v1.15.0
ARG ZEROMQ_VERSION=v4.3.5
ARG CPPZMQ_VERSION=v4.10.0
ARG CZMQ_VERSION=v4.2.1
ARG ZYRE_VERSION=v2.0.1
ARG CYCLONEDDS_VERSION=0.10.5
ARG CYCLONEDDS_CXX_VERSION=0.10.5
ARG CROW_VERSION=v1.2.0
ARG GTEST_VERSION=v1.14.0
ARG RE2_VERSION=2023-03-01
ARG CARES_VERSION=v1.34.6
ARG GRPC_VERSION=v1.67.1

# ---- Prevent interactive prompts -------------------------------------------
ENV DEBIAN_FRONTEND=noninteractive

# ---- Container build sentinel — blocks accidental conan install -------------
ENV STARTERCPP_CONTAINER_BUILD=1

# ---- Build tools and system libraries ---------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential \
      g++ \
      cmake \
      ninja-build \
      python3 \
      git \
      ca-certificates \
      pkg-config \
      libsystemd-dev \
      libasio-dev \
      libssl-dev \
      lcov \
      gcovr \
      clang-tidy \
    && rm -rf /var/lib/apt/lists/*

# ---- abseil-cpp (required by protobuf 5.x) ---------------------------------
RUN git clone --depth 1 --branch ${ABSEIL_VERSION} \
      https://github.com/abseil/abseil-cpp.git /tmp/abseil \
    && cmake -S /tmp/abseil -B /tmp/abseil/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_CXX_STANDARD=20 \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
       -DABSL_PROPAGATE_CXX_STD=ON \
       -DABSL_BUILD_TESTING=OFF \
    && cmake --build /tmp/abseil/build \
    && cmake --install /tmp/abseil/build \
    && rm -rf /tmp/abseil

# ---- protobuf 5.27.0 (needs abseil) ----------------------------------------
RUN git clone --depth 1 --branch ${PROTOBUF_VERSION} \
      https://github.com/protocolbuffers/protobuf.git /tmp/protobuf \
    && cmake -S /tmp/protobuf -B /tmp/protobuf/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_CXX_STANDARD=20 \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
       -Dprotobuf_ABSL_PROVIDER=package \
       -Dprotobuf_BUILD_TESTS=OFF \
       -Dprotobuf_BUILD_EXAMPLES=OFF \
    && cmake --build /tmp/protobuf/build \
    && cmake --install /tmp/protobuf/build \
    && ldconfig \
    && rm -rf /tmp/protobuf

# ---- re2 (required by gRPC) -------------------------------------------------
RUN git clone --depth 1 --branch ${RE2_VERSION} \
      https://github.com/google/re2.git /tmp/re2 \
    && cmake -S /tmp/re2 -B /tmp/re2/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_CXX_STANDARD=20 \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
       -DRE2_BUILD_TESTING=OFF \
    && cmake --build /tmp/re2/build \
    && cmake --install /tmp/re2/build \
    && rm -rf /tmp/re2

# ---- c-ares (required by gRPC) ----------------------------------------------
RUN git clone --depth 1 --branch ${CARES_VERSION} \
      https://github.com/c-ares/c-ares.git /tmp/c-ares \
    && cmake -S /tmp/c-ares -B /tmp/c-ares/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
       -DCARES_BUILD_TOOLS=OFF \
       -DCARES_BUILD_TESTS=OFF \
    && cmake --build /tmp/c-ares/build \
    && cmake --install /tmp/c-ares/build \
    && ldconfig \
    && rm -rf /tmp/c-ares

# ---- gRPC 1.67.1 (needs abseil, protobuf, re2, c-ares, OpenSSL) ------------
RUN git clone --depth 1 --branch ${GRPC_VERSION} \
      https://github.com/grpc/grpc.git /tmp/grpc \
    && cmake -S /tmp/grpc -B /tmp/grpc/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_CXX_STANDARD=20 \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
       -DgRPC_INSTALL=ON \
       -DgRPC_BUILD_TESTS=OFF \
       -DgRPC_BUILD_CSHARP_EXT=OFF \
       -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF \
       -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
       -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF \
       -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
       -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF \
       -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF \
       -DgRPC_ABSL_PROVIDER=package \
       -DgRPC_PROTOBUF_PROVIDER=package \
       -DgRPC_RE2_PROVIDER=package \
       -DgRPC_CARES_PROVIDER=package \
       -DgRPC_SSL_PROVIDER=package \
       -DgRPC_ZLIB_PROVIDER=package \
    && cmake --build /tmp/grpc/build \
    && cmake --install /tmp/grpc/build \
    && ldconfig \
    && rm -rf /tmp/grpc

# ---- spdlog 1.15.0 (with C++20 std::format) --------------------------------
RUN git clone --depth 1 --branch ${SPDLOG_VERSION} \
      https://github.com/gabime/spdlog.git /tmp/spdlog \
    && cmake -S /tmp/spdlog -B /tmp/spdlog/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_CXX_STANDARD=20 \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
       -DSPDLOG_USE_STD_FORMAT=ON \
       -DSPDLOG_BUILD_EXAMPLE=OFF \
       -DSPDLOG_BUILD_TESTS=OFF \
    && cmake --build /tmp/spdlog/build \
    && cmake --install /tmp/spdlog/build \
    && rm -rf /tmp/spdlog

# ---- ZeroMQ 4.3.5 ----------------------------------------------------------
RUN git clone --depth 1 --branch ${ZEROMQ_VERSION} \
      https://github.com/zeromq/libzmq.git /tmp/libzmq \
    && cmake -S /tmp/libzmq -B /tmp/libzmq/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DBUILD_STATIC=ON \
       -DBUILD_TESTS=OFF \
       -DWITH_DOCS=OFF \
    && cmake --build /tmp/libzmq/build \
    && cmake --install /tmp/libzmq/build \
    && ldconfig \
    && rm -rf /tmp/libzmq

# ---- cppzmq 4.10.0 (header-only C++ binding) -------------------------------
RUN git clone --depth 1 --branch ${CPPZMQ_VERSION} \
      https://github.com/zeromq/cppzmq.git /tmp/cppzmq \
    && cmake -S /tmp/cppzmq -B /tmp/cppzmq/build -G Ninja \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCPPZMQ_BUILD_TESTS=OFF \
    && cmake --build /tmp/cppzmq/build \
    && cmake --install /tmp/cppzmq/build \
    && rm -rf /tmp/cppzmq

# ---- CZMQ 4.2.1 ------------------------------------------------------------
RUN git clone --depth 1 --branch ${CZMQ_VERSION} \
      https://github.com/zeromq/czmq.git /tmp/czmq \
    && cmake -S /tmp/czmq -B /tmp/czmq/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCZMQ_BUILD_STATIC=ON \
       -DCZMQ_BUILD_TESTS=OFF \
       -DENABLE_DRAFTS=ON \
    && cmake --build /tmp/czmq/build \
    && cmake --install /tmp/czmq/build \
    && ldconfig \
    && rm -rf /tmp/czmq

# ---- Zyre 2.0.1 (not available via apt) ------------------------------------
RUN git clone --depth 1 --branch ${ZYRE_VERSION} \
      https://github.com/zeromq/zyre.git /tmp/zyre \
    && cmake -S /tmp/zyre -B /tmp/zyre/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DZYRE_BUILD_STATIC=ON \
       -DZYRE_BUILD_TESTS=OFF \
       -DENABLE_DRAFTS=ON \
    && cmake --build /tmp/zyre/build \
    && cmake --install /tmp/zyre/build \
    && ldconfig \
    && rm -rf /tmp/zyre

# ---- Eclipse Cyclone DDS 0.10.5 (C binding + IDL compiler) -----------------
RUN git clone --depth 1 --branch ${CYCLONEDDS_VERSION} \
      https://github.com/eclipse-cyclonedds/cyclonedds.git /tmp/cyclonedds \
    && cmake -S /tmp/cyclonedds -B /tmp/cyclonedds/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
       -DBUILD_TESTING=OFF \
       -DBUILD_EXAMPLES=OFF \
       -DBUILD_IDLC=ON \
    && cmake --build /tmp/cyclonedds/build \
    && cmake --install /tmp/cyclonedds/build \
    && ldconfig \
    && rm -rf /tmp/cyclonedds

# ---- Eclipse Cyclone DDS C++ 0.10.5 (not available via apt) ----------------
RUN git clone --depth 1 --branch ${CYCLONEDDS_CXX_VERSION} \
      https://github.com/eclipse-cyclonedds/cyclonedds-cxx.git /tmp/cyclonedds-cxx \
    && cmake -S /tmp/cyclonedds-cxx -B /tmp/cyclonedds-cxx/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
       -DBUILD_TESTING=OFF \
       -DBUILD_EXAMPLES=OFF \
    && cmake --build /tmp/cyclonedds-cxx/build \
    && cmake --install /tmp/cyclonedds-cxx/build \
    && ldconfig \
    && rm -rf /tmp/cyclonedds-cxx

# ---- Fix CycloneDDS C++ 0.10.5 headers for GCC 13+ / C++20 ----------------
# The ~Class<T>() destructor syntax is rejected by GCC 13 in C++20 mode.
# Fixed upstream in CycloneDDS-CXX >= 0.11.0.
RUN sed -i 's/~Reference<DELEGATE>()/~Reference()/' \
      /usr/local/include/ddscxx/dds/core/detail/ReferenceImpl.hpp \
    && sed -i 's/~Topic<T>()/~Topic()/' \
      /usr/local/include/ddscxx/dds/topic/detail/TTopicImpl.hpp \
    && sed -i 's/~DataReader<T>()/~DataReader()/' \
      /usr/local/include/ddscxx/dds/sub/detail/TDataReaderImpl.hpp \
    && sed -i 's/~DataWriter<T>()/~DataWriter()/' \
      /usr/local/include/ddscxx/dds/pub/detail/DataWriterImpl.hpp

# ---- Crow (C++ HTTP/WebSocket framework) ------------------------------------
RUN git clone --depth 1 --branch ${CROW_VERSION} \
      https://github.com/CrowCpp/Crow.git /tmp/crow \
    && cmake -S /tmp/crow -B /tmp/crow/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCROW_BUILD_EXAMPLES=OFF \
       -DCROW_BUILD_TESTS=OFF \
    && cmake --build /tmp/crow/build \
    && cmake --install /tmp/crow/build \
    && rm -rf /tmp/crow

# ---- Google Test 1.14.0 ----------------------------------------------------
RUN git clone --depth 1 --branch ${GTEST_VERSION} \
      https://github.com/google/googletest.git /tmp/gtest \
    && cmake -S /tmp/gtest -B /tmp/gtest/build -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_INSTALL_PREFIX=/usr/local \
       -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
       -DBUILD_GMOCK=ON \
    && cmake --build /tmp/gtest/build \
    && cmake --install /tmp/gtest/build \
    && rm -rf /tmp/gtest

# ---- Ensure dynamic linker sees /usr/local/lib ------------------------------
RUN ldconfig

WORKDIR /workspace
